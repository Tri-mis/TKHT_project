#include "esp32-hal-gpio.h"
#include "HardwareSerial.h"
#include "TKHT_lib.h"

// BUTTON TASK
void Hardware_Control(void *pvParameters)
{
    while (true)
    {
        button.changeLeverState(false);

        if (button.hold_state_time > 3000 && button.hold_state_time < 5000)
        {  // Held > 3s
            is_setup_mode = !is_setup_mode; // Toggle mode

            if (is_setup_mode)
            {
                Serial.println("Switching to SETUP mode...");
                user_want_to_change_wifi = true;
                change_task(true);
            }
            else
            {
                Serial.println("Switching to WORKING mode...");
                change_task(false);
            }
            button.hold_state_time = 0;
        }
        else if (button.hold_state_time > 5000 && button.hold_state_time < 10000)
        {
            if (buzzer_on)
            {
                buzzer_on = false;
                digitalWrite(BUZZER_PIN, 0);
                int taskType = BUZZER_UPDATE;
                xQueueSend(taskQueue, &taskType, 0);
                
            }

            button.hold_state_time = 0;
        }
        else if(button.hold_state_time > 10000 && button.hold_state_time < 15000)
        {
            for (int i = 0; i < 512; i++) 
            {
                EEPROM.write(i, 0xFF);
            }
            
            EEPROM.commit();
            button.hold_state_time = 0;
            logMessage("Clear EEPORM");
            led_flicker(100, 3, RED_LED);

            button.hold_state_time = 0;
        }
        else if(button.hold_state_time > 15000 && button.hold_state_time < 20000)
        {
            int taskType = SEND_DATA_TASK;
            xQueueSend(taskQueue, &taskType, 0);
            button.hold_state_time = 0;
        }
        else if(button.hold_state_time > 20000)
        {
            do_an_do_luong = !do_an_do_luong;
            led_flicker(100, 10, YELLOW_LED);
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
        if (load_wifi_credentials() && connect_to_wifi())
        {
            connect_to_firebase();
            get_config_data_from_firebase();
            disconnect_if_allowed();
            stop_bluetooth();
            led_flicker(100, 3, GREEN_LED);

            change_task(false);
        }
        else
        {
            led_flicker(100, 3, RED_LED);
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
