#include "TKHT_lib.h"


// Task Handles
TaskHandle_t setupTaskHandle = NULL;
TaskHandle_t workingTaskHandle = NULL;

// Mode flag
bool isSetupMode = false;

// WiFi Credentials
String ssid = "";
String password = "";
bool disconnect_allowed = true;

QueueHandle_t taskQueue;

ArtronShop_SHT3x sht3x(0x44, &Wire);
SensorData sensorData;
bool setAlert = false;
bool just_send_alert = false;

// Button object
Button button(17, 50, true, false); // GPIO 17, 50ms debounce, pull-up


// Firebase stuff
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;
String path = "";
bool firebase_setup_done = false;

