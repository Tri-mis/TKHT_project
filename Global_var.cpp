#include "TKHT_lib.h"


// Task related variables
TaskHandle_t setupTaskHandle = NULL;
TaskHandle_t workingTaskHandle = NULL;
QueueHandle_t taskQueue;

// WiFi related variables
String ssid = "";
String password = "";
bool disconnect_allowed = true;
bool user_want_to_change_wifi = false;

// Hardware related variables
ArtronShop_SHT3x sht3x(0x44, &Wire);
Button button(17, 50, true, false); // GPIO 17, 50ms debounce, pull-up

// Data related variables
SensorData sensorData;
int send_data_interval = 60;
int read_data_interval = 2;

// Firebase related variables
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String path = "";
bool firebase_setup_done = false;

// Alert related variables
bool instant_alert_is_sent = false;
bool buzzer_on = false;
bool alert_is_set = false;

// Other variables
bool enable_logging = true;
bool isSetupMode = false;

