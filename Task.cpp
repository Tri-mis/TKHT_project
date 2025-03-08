#include "HardwareSerial.h"
#include "TKHT_lib.h"

// BUTTON TASK
void Button_Task(void *pvParameters)
{
    while (true)
    {
        button.changeLeverState(false);

        if (button.hold_state_time > 3000 && button.hold_state_time < 10000)
        {  // Held > 3s
            is_setup_mode = !is_setup_mode; // Toggle mode

            if (is_setup_mode)
            {
                Serial.println("Switching to SETUP mode...");
                user_want_to_change_wifi = true;
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
        else if(button.hold_state_time > 10000)
        {
            for (int i = 0; i < 512; i++) 
            {
                EEPROM.write(i, 0xFF);
            }
            
            EEPROM.commit();
            button.hold_state_time = 0;
            Serial.println("Clear EEPORM");
        }

        vTaskDelay(10);
    }
}

// SETUP TASK: Change WiFi credentials via Serial Monitor
void Setup_Task(void *pvParameters)
{
    while (true)
    {
        if (load_wifi_credentials() && connect_to_wifi())
        {
            connect_to_firebase();
            get_config_data_from_firebase();
            disconnect_if_allowed();
            stop_bluetooth();

            is_setup_mode = false;
            vTaskResume(workingTaskHandle);
            vTaskSuspend(setupTaskHandle);
        }
        else
        {
            start_taking_wifi_credentials_using_bluetooth();
            save_wifi_credentials();
        }

        vTaskDelay(100);
    }
}

void Working_Task(void *pvParameters)
{
    int64_t curTime;
    int64_t read_data_pre_time = 0;
    int64_t send_data_pre_time = 0;
    int64_t handel_alert_pre_time = 0;

    while (true)
    {
        curTime = esp_timer_get_time() / 1000000;

        if (curTime - read_data_pre_time >= read_data_interval)
        {
            int taskType = READ_DATA_TASK;
            xQueueSend(taskQueue, &taskType, 0);

            read_data_pre_time = curTime;
        }


        if (curTime - send_data_pre_time >= send_data_interval)
        {
            int taskType = SEND_DATA_TASK;
            xQueueSend(taskQueue, &taskType, 0);

            send_data_pre_time = curTime;
        }

        if (curTime - handel_alert_pre_time >= handel_alert_interval)
        {
            int taskType = HANDLE_ALERT_TASK;
            xQueueSend(taskQueue, &taskType, 0);

            handel_alert_pre_time = curTime;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
