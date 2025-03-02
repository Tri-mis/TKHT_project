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


//============================================================== CLASS ==============================================================//
class SensorData{
  public:
    float humidity = 0;
    float temperature = 0;
    float maxHumidity = 100;
    float minHumidity = 0;
    float maxTemperature = 33;
    float minTemperature = 0;
    String timeStamp = "";
};

//============================================================== VARIABLE DECORATION ==============================================================//
// Task Handles
extern TaskHandle_t setupTaskHandle;
extern TaskHandle_t workingTaskHandle;

// Mode flag
extern bool isSetupMode;

// WiFi Credentials
extern String ssid;
extern String password;
extern bool disconnect_allowed;

extern QueueHandle_t taskQueue;

extern SensorData sensorData;
extern ArtronShop_SHT3x sht3x;
extern bool setAlert;
extern bool just_send_alert;

// Button object
extern Button button; // GPIO 17, 50ms debounce, pull-up

extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;
extern String path;
extern bool firebase_setup_done;

extern bool enable_logging;
extern bool alert_is_set;
extern bool user_want_to_change_wifi;


//============================================================== FUNCTION DECORATION ==============================================================//
bool loadWiFiCredentials();
void saveWiFiCredentials(String newSSID, String newPassword);
void readSensorData();
void connectWiFi();
void sendData();
void sendAlert();
String getFormattedTime();
void checkSensorData();
void setupFirebase();
void to_database(const String &folder, void *data);

//============================================================== FUNCTION DECORATION ==============================================================//
template <typename... Args>
void logMessage(Args... args);
bool load_wifi_credentials();
void save_wifi_credentials();
bool connect_to_wifi();
void connect_to_firebase();
void refresh_firebase_token();
void get_config_data_from_firebase();
String get_formatted_time();
void send_data_to_firebase();
void disconnect_if_allowed();
void read_sensor_data();
void check_sensor_data_to_send_alert();
void take_wifi_credential_from_user_input();




// Define tasks
void Button_Task(void *pvParameters);
void Setup_Task(void *pvParameters);
void Working_Task(void *pvParameters);
