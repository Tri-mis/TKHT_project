#include "TKHT_lib.h"

void setup()
{
    Serial.begin(115200);
    
    Wire.begin();
    while (!sht3x.begin())
    {
        logMessage("SHT3x not found !");
        delay(1000);
    }

    taskQueue = xQueueCreate(100, sizeof(int));
    EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM
    
    // Create tasks
    xTaskCreatePinnedToCore(Button_Task, "ButtonTask", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Setup_Task, "SetupTask", 8000, NULL, 1, &setupTaskHandle, 0);
    xTaskCreatePinnedToCore(Working_Task, "WorkingTask", 1024, NULL, 1, &workingTaskHandle, 0);

    vTaskSuspend(workingTaskHandle); // only keep the Setup task running, if it done successfully, it auto matically change to working task
}

void loop()
{
    int receivedTask;
    if (xQueueReceive(taskQueue, &receivedTask, portMAX_DELAY))
    {
        switch (receivedTask)
        {
            case READ_DATA_TASK:
                read_sensor_data();
                check_sensor_data_to_send_alert();
                break;

            case SEND_DATA_TASK:
                if (connect_to_wifi())
                {
                    refresh_firebase_token();
                    sensorData.timeStamp = get_formatted_time();
                    send_data_to_firebase();
                    disconnect_if_allowed();
                }
                else 
                {
                    logMessage("Unable to connect to WiFi -> abort sending data to firebase");
                }
                break;
            
            case HANDLE_ALERT_TASK:
                if (alert_is_set)
                {
                    if (!alert_handeling_is_init)
                    {
                        buzzer_on = true; // turn on the buzzer
                        send_data_interval = send_data_interval_in_alert;
                        disconnect_allowed = false;

                        if (connect_to_wifi())
                        {
                            refresh_firebase_token();
                            sensorData.timeStamp = get_formatted_time();
                            send_data_to_firebase();
                            
                            begin_data_streamming(); // begin a new data stream
                            stream_is_on = true;
                            alert_handeling_is_init = true;
                        }
                        else 
                        {
                            logMessage("Unable to connect to WiFi -> Alert is not sent to firebase!");
                        }
                    }

                    check_for_buzzer_turn_off(); //listen to the stream, if the buzzer get turn off, dont end the stream!!
                }
                else
                {
                    if (stream_is_on) 
                    {
                        stop_data_streaming(); // stop a data stream, make sure a new one can be created
                        stream_is_on = false;
                    }

                    alert_handeling_is_init = false;
                    buzzer_on = false;
                    disconnect_allowed = true;
                    send_data_interval = send_data_interval_normal;

                    disconnect_if_allowed();
                }
                
                break;

                
        }
    }
}
