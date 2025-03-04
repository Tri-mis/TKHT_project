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
Button button(17, 50, true, false);

// Data related variables
SensorData sensorData;
int send_data_interval = 0;
int send_data_interval_normal = 3600;
int send_data_interval_in_alert = 300;
int read_data_interval = 2;
int handel_alert_interval = 2;

// Firebase related variables
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData stream;
String path = "";
bool firebase_setup_done = false;
bool stream_is_on = false;

// Alert related variables
bool alert_handeling_is_init = false;
bool buzzer_on = false;
bool alert_is_set = false;

// Other variables
bool enable_logging = true;
bool is_setup_mode = false;

