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
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

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
#define BUZZER_UPDATE 4

#define BUZZER_PIN 33
#define GREEN_LED 13
#define YELLOW_LED 12
#define RED_LED 27

#define SERVICE_UUID "19b10000-e8f2-537e-4f6c-d104768a1214"
#define WIFI_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define MAC_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"

#define BATTERY_PIN 34
#define MIN_BAT_VOLT 1680
#define MAX_BAT_VOLT 2350

#define BUTTON_PIN 17

//============================================================== DATA CLASS ==============================================================//
class SensorData{
  public:
    float humidity = 0;
    float temperature = 0;
    float maxHumidity = 1000;
    float minHumidity = -1000;
    float maxTemperature = 1000;
    float minTemperature = -1000;
    int battery = 0;
    long timeStamp = 0;

    SensorData();
};

//============================================================== BLUETOOTH CLASS ==============================================================//
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer);

    void onDisconnect(BLEServer* pServer);
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
extern bool do_an_do_luong;

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

// Bluetooth variables
extern BLEServer* pServer;
extern BLECharacteristic* pWiFiCharacteristic;
extern BLECharacteristic* pMacCharacteristic;
extern bool bluetooth_is_init;


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
void get_formatted_time();
void send_data_to_firebase();
void disconnect_if_allowed();
void read_sensor_data();
void check_sensor_data_to_send_alert();
void take_wifi_credential_from_user_input();
void begin_data_streamming();
void stop_data_streaming();
void streamCallback(FirebaseStream data);
void streamTimeoutCallback(bool timeout);
void start_taking_wifi_credentials_using_bluetooth();
void stop_bluetooth();
void change_task(bool change_to_setup);
void led_flicker(int interval, int times, int led);
void read_battery();
String formatData(String timeStampe, float temperature, float humidity, int battery);


//============================================================== TASK DECORATION ==============================================================//
void Hardware_Control(void *pvParameters);
void Setup_Task(void *pvParameters);
void Working_Task(void *pvParameters);
