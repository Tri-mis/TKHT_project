#include "TKHT_lib.h"

void setup()
{
    Serial.begin(115200);
    
    Wire.begin();
    while (!sht3x.begin())
    {
        Serial.println("SHT3x not found !");
        delay(1000);
    }

    taskQueue = xQueueCreate(100, sizeof(int));
    EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM
    
    // Create tasks
    xTaskCreatePinnedToCore(Button_Task, "ButtonTask", 1024, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Setup_Task, "SetupTask", 8000, NULL, 1, &setupTaskHandle, 0);
    xTaskCreatePinnedToCore(Working_Task, "WorkingTask", 1024, NULL, 1, &workingTaskHandle, 0);

    vTaskSuspend(workingTaskHandle);
}

void loop()
{
    int receivedTask;
    if (xQueueReceive(taskQueue, &receivedTask, portMAX_DELAY))
    {
        switch (receivedTask)
        {
            case 1:
                read_sensor_data();
                check_sensor_data_to_send_alert();
                break;
            case 2:
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
        }
    }
}
