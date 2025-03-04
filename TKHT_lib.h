#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <myButtonLib.h>
#include "time.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <Wire.h>
#include <ArtronShop_SHT3x.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// EEPROM settings
#define EEPROM_SIZE 100
#define SSID_ADDR 0
#define PASS_ADDR 50


#define API_KEY "AIzaSyB_rcqMGHbd3QGNHnapj2qTX_SjgBEx7cs"
#define DATABASE_URL "https://nhom-iot-1356a-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "myesp32@gmail.com"
#define USER_PASSWORD "15072003"

#define READ_DATA_TASK 1
#define SEND_DATA_TASK 2
#define HANDLE_ALERT_TASK 3

//============================================================== DATA CLASS ==============================================================//
class SensorData{
  public:
    float humidity = 0;
    float temperature = 0;
    float maxHumidity = 1000;
    float minHumidity = -1000;
    float maxTemperature = 1000;
    float minTemperature = -1000;
    String timeStamp = "";
};

//============================================================== VARIABLE DECORATION ==============================================================//
// Task related variables
extern TaskHandle_t setupTaskHandle;
extern TaskHandle_t workingTaskHandle;
extern QueueHandle_t taskQueue;


// WiFi related variables
extern String ssid;
extern String password;
extern bool disconnect_allowed;
extern bool user_want_to_change_wifi;

// Hardware related variables
extern ArtronShop_SHT3x sht3x;
extern Button button;


// Data related variable
extern SensorData sensorData;
extern int send_data_interval;
extern int send_data_interval_normal;
extern int send_data_interval_in_alert;
extern int read_data_interval;
extern int handel_alert_interval;

// Firebase related variables
extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;
extern FirebaseData stream;
extern String path;
extern bool firebase_setup_done;
extern bool stream_is_on;

// Alert related variables
extern bool alert_handeling_is_init;
extern bool buzzer_on;
extern bool alert_is_set;

// Other variables
extern bool enable_logging;
extern bool is_setup_mode;



//============================================================== FUNCTION DECORATION ==============================================================//
template <typename... Args>
void logMessage(Args... args);
bool load_wifi_credentials();
void save_wifi_credentials();
bool connect_to_wifi();
void connect_to_firebase();
void refresh_firebase_token();
void get_config_data_from_firebase();
bool from_database();
void to_database(const String &folder, void *data);
String get_formatted_time();
void send_data_to_firebase();
void disconnect_if_allowed();
void read_sensor_data();
void check_sensor_data_to_send_alert();
void take_wifi_credential_from_user_input();
void begin_data_streamming();
void check_for_buzzer_turn_off();
void stop_data_streaming();


//============================================================== TASK DECORATION ==============================================================//
void Button_Task(void *pvParameters);
void Setup_Task(void *pvParameters);
void Working_Task(void *pvParameters);
