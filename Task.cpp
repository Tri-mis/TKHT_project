#include "TKHT_lib.h"


// BUTTON TASK
void Button_Task(void *pvParameters) 
{
    while (true) 
    {
        button.changeLeverState(false);

        if (button.hold_state_time > 3000) 
        {  // Held > 3s
            isSetupMode = !isSetupMode; // Toggle mode

            if (isSetupMode) 
            {
                Serial.println("Switching to SETUP mode...");
                vTaskSuspend(workingTaskHandle);
                vTaskResume(setupTaskHandle);
            }
            else
            {
                Serial.println("Switching to WORKING mode...");
                vTaskSuspend(setupTaskHandle);
                vTaskResume(workingTaskHandle);
            }
            button.hold_state_time = 0;
        }


        vTaskDelay(10);
    }
}

// SETUP TASK: Change WiFi credentials via Serial Monitor
void Setup_Task(void *pvParameters) 
{
    while (true) 
    {
      // Take ssid from user input
      Serial.println("SETUP MODE: Enter new WiFi SSID:");
      while (Serial.available() == 0) vTaskDelay(100);
      String newSSID = Serial.readStringUntil('\n');
      newSSID.trim();

      // Take password from user input
      Serial.println("Enter new WiFi Password:");
      while (Serial.available() == 0)  vTaskDelay(100);
      String newPassword = Serial.readStringUntil('\n');
      newPassword.trim();

      // Save ssid and password into EEPROM
      saveWiFiCredentials(newSSID, newPassword);

      // Load ssid and password from EEPROM
      loadWiFiCredentials();

      
      Serial.println("WiFi credentials updated!");

      vTaskDelay(5000);
    }
}

void Working_Task(void *pvParameters) 
{
    int64_t curTime;
    int64_t prevTime2Sec = 0;
    int64_t prevTime30Sec = 0;
    
    while (true) {
        curTime = esp_timer_get_time() / 1000000;
        
        if (curTime - prevTime2Sec >= 2) {
            int taskType = 1;
            xQueueSend(taskQueue, &taskType, 0);
            prevTime2Sec = curTime;
        }
        
        if (curTime - prevTime30Sec >= 600) {
            int taskType = 2;
            xQueueSend(taskQueue, &taskType, 0);
            prevTime30Sec = curTime;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
