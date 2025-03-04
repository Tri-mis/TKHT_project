============================================================== 07:44 - 03/03/2025 ==============================================================

- I changed all the functions names so that they are intuitive and a bit more readable

- The way alert is handel now is: when check_data_to_send_alert see data exceeded limit, the alert_is_set variable is set to true, then in the Working task, it check if the
    alert_is_set is true, then it set the buzzer_on to true, the disconnect_allowed to false, and the send_data_interval to be much shorter.
    After that, it check if instant_alert_is_sent is not set, working task will trigger an instant update to firebase by putting 2 in the task queue and then set the 
    instant_alert_is_sent to false, ensuring that the instant message is sent just one. After this step, an alert is sent instantly, the connection is kept alive,
    the data update interval is shorter, and the buzzer is on. In the front end, it suppose to be that everytime 
    an update that exceeds limit will instantly trigger a mail to be sent to user, and during alert time, the update is more frequently than normal because update interval is
    shorter. when check_data_to_send_alert see data is back to normal, it will set the alert_is_set to false then in the working task set the instant_alert_is_sent to false, 
    the send_data_interval to normal, the disconnect_allowed to true, and buzzer_on to false. Also, the buzzer_on can be set to false by the user at the front end, this is 
    why the connection is kept alive. Also when the button is set to false, I should set the disconnect_allowed to true because we don't need to control the button any more.

- What should I work on next time?
    - need to enable the fload configuration from firebase freature
    - need to be able to turn off the buzzer from firebase 

============================================================== 12:23 - 05/03/2025 ==============================================================

- I add a HANDEL alert task. This task is called every 2 second, it check for alert_is_set variable and do all the alert handling including 
    turnning of the alert buzzer and send data imediately oncec, then checking if the user turn buzzer off, and keep the stream as long as the alert_is_set still true

- currently, the device goes into SetupTask first, then it tries loading the wifi credential and then try to connect to wifi. if both of that success, it will connect to firebase
    and then get the configuration, then disconnect and go to working mode. if one of the condition failed, it will ask for wifi password and credential from user. That mean
    although it load the ssid and password succesfully, but it still cant connect to wifi within 20 tries, it still ask for new input of ssid and password. Also, with this logic
    we can force the device to update configuration from firebase by putting it in SetUp task, if the wifi credential is not true, or not exist, it ask for new one, if the wifi credentail
    is loaded succesfully try to connect, either way, if will update the configuration from firebase once connected to wifi.

- holding the button for 3 second put the device in setUp task, for 10 second it wipes its EEPROM memory to clear wifi ssid and password. After that, we can put it in setUp mode
    and it will ask for new ssid and password -> we can change wifi credentials.

- I need to work on:
    - begin_data_streamming()
    - check_for_buzzer_turn_off()
    - stop_data_streaming()

