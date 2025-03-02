#include "TKHT_lib.h"


void setup() {
    Serial.begin(115200);
    
    Wire.begin();
    while (!sht3x.begin()) {
      Serial.println("SHT3x not found !");
      delay(1000);
    }

    taskQueue = xQueueCreate(100, sizeof(int));
    EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM
    isSetupMode = (loadWiFiCredentials()) ? false : true; // Load stored WiFi credentials use that credential if loaded successfully
    
    // Create tasks
    xTaskCreatePinnedToCore(Button_Task, "ButtonTask", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Setup_Task, "SetupTask", 2048, NULL, 1, &setupTaskHandle, 0);
    xTaskCreatePinnedToCore(Working_Task, "WorkingTask", 2048, NULL, 1, &workingTaskHandle, 0);

    // Initialize task
    if (isSetupMode) vTaskSuspend(workingTaskHandle);
    else vTaskSuspend(setupTaskHandle);
}

void loop() {
    int receivedTask;
    if (xQueueReceive(taskQueue, &receivedTask, portMAX_DELAY)) {
        switch (receivedTask) 
        {
            case 1:
                readSensorData();
                break;
            case 2:
                connectWiFi();
                sendData();
                break;
        }
    }
}







