#include "TKHT_lib.h"

void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    
    Wire.begin();
    while (!sht3x.begin())
    {
        logMessage("SHT3x not found !");
        delay(1000);
    }

    taskQueue = xQueueCreate(100, sizeof(int));
    EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM
    
    // Create tasks
    xTaskCreatePinnedToCore(Hardware_Control, "Hardware_Control", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Setup_Task, "SetupTask", 8000, NULL, 1, &setupTaskHandle, 0);
    xTaskCreatePinnedToCore(Working_Task, "WorkingTask", 1024, NULL, 1, &workingTaskHandle, 0);

    change_task(true);
}


void loop()
{
    int receivedTask;
    if (xQueueReceive(taskQueue, &receivedTask, portMAX_DELAY))
    {
        switch (receivedTask)
        {
            case READ_DATA_TASK:
                logMessage("\n==================== READ DATA TASK ====================");
                read_sensor_data();
                check_sensor_data_to_send_alert();
                break;

            case SEND_DATA_TASK:
                logMessage("\n==================== SEND DATA TASK ====================");
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
                logMessage("\n=================== HANDLE ALERT TASK ==================");
                if (alert_is_set)
                {
                    logMessage("Alert!!!");

                    if (!alert_handeling_is_init)
                    {
                        buzzer_on = true; // turn on the buzzer
                        digitalWrite(BUZZER_PIN, buzzer_on);
                        send_data_interval = send_data_interval_in_alert;
                        disconnect_allowed = false;

                        if (connect_to_wifi())
                        {
                            refresh_firebase_token();
                            sensorData.timeStamp = get_formatted_time();
                            send_data_to_firebase();
                            to_database("/actuator/buzzer", (void*) &buzzer_on);
                            begin_data_streamming();// begin a new data stream
                            stream_is_on = true;
                            alert_handeling_is_init = true;
                        }
                        else 
                        {
                            logMessage("Unable to connect to WiFi -> Alert is not sent to firebase!");
                        }
                    }

                }
                else
                {
                    logMessage("No alert");

                    if (stream_is_on)
                    {
                        // stop a data stream, make sure a new one can be created
                        stop_data_streaming();
                        buzzer_on = false;
                        digitalWrite(BUZZER_PIN, buzzer_on);
                        to_database("/actuator/buzzer", (void*) &buzzer_on);
                        stream_is_on = false;
                    }

                    alert_handeling_is_init = false;
                    disconnect_allowed = true;
                    send_data_interval = send_data_interval_normal;

                    disconnect_if_allowed();
                }
                
                break;  
        }
    }
}
