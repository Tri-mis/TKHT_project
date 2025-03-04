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