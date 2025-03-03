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
        vTaskDelay(10);
    }
}

// SETUP TASK: Change WiFi credentials via Serial Monitor
void Setup_Task(void *pvParameters)
{
    while (true)
    {
        if (load_wifi_credentials() && user_want_to_change_wifi == false)
        {
            if (connect_to_wifi())
            {
                connect_to_firebase();
                get_config_data_from_firebase();
                disconnect_if_allowed();

                vTaskResume(workingTaskHandle);
                vTaskSuspend(setupTaskHandle);
            }
        }
        else
        {
            take_wifi_credential_from_user_input();
            save_wifi_credentials();
            user_want_to_change_wifi = false;
        }

        vTaskDelay(100);
    }
}

void Working_Task(void *pvParameters)
{
    int64_t curTime;
    int64_t read_data_pre_time = 0;
    int64_t send_data_pre_time = 0;

    while (true)
    {
        curTime = esp_timer_get_time() / 1000000;

        if (curTime - read_data_pre_time >= read_data_interval)
        {
            int taskType = 1;
            xQueueSend(taskQueue, &taskType, 0);
            read_data_pre_time = curTime;
        }

        if (curTime - send_data_pre_time >= send_data_interval)
        {
            int taskType = 2;
            xQueueSend(taskQueue, &taskType, 0);
            send_data_pre_time = curTime;
        }

        if (alert_is_set)
        {
            buzzer_on = true;
            send_data_interval = 10;
            disconnect_allowed = false;

            if (!instant_alert_is_sent)
            {
                int taskType = 1;
                xQueueSend(taskQueue, &taskType, 0);

                instant_alert_is_sent = true;
                logMessage("instant alert is sent!!!");
            }
        }
        else
        {
            instant_alert_is_sent = false;
            send_data_interval = 60;
            disconnect_allowed = true;
            buzzer_on = false;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
