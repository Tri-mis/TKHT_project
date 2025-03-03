#include "TKHT_lib.h"

template <typename... Args>
void logMessage(Args... args)
{
    if (enable_logging)
    {
        (Serial.print(args), ...); // Expands to multiple Serial.print calls
        Serial.println();          // Add a newline at the end
    }
}

bool load_wifi_credentials()
{
    int ssid_len = EEPROM.read(SSID_ADDR);
    if (ssid_len <= 0 || ssid_len >= 49) return false; // Prevent invalid reads

    char ssid_buf[50] = {0}; // Create buffer with null terminator
    for (int i = 0; i < ssid_len; i++) 
    {
        ssid_buf[i] = EEPROM.read(SSID_ADDR + 1 + i);
    }
    ssid = String(ssid_buf);  // Convert buffer to String

    int password_len = EEPROM.read(PASS_ADDR);
    if (password_len <= 0 || password_len >= 49) return false; // Prevent invalid reads

    char pass_buf[50] = {0}; // Create buffer with null terminator
    for (int i = 0; i < password_len; i++) 
    {
        pass_buf[i] = EEPROM.read(PASS_ADDR + 1 + i);
    }
    password = String(pass_buf);

    // Debug output
    logMessage("-> Loaded SSID: ", ssid);
    logMessage("-> Loaded Password: ", password);
    return true;
}


void save_wifi_credentials()
{
    int ssid_len = ssid.length();
    EEPROM.write(SSID_ADDR, ssid_len);
    for (int i = 0; i < ssid_len; i++) 
    {
        EEPROM.write(SSID_ADDR + 1 + i, ssid[i]);
    }

    int password_len = password.length();
    EEPROM.write(PASS_ADDR, password_len);
    for (int i = 0; i < password_len; i++) 
    {
        EEPROM.write(PASS_ADDR + 1 + i, password[i]);
    }

    EEPROM.commit();  // **Ensure data is actually written to EEPROM**
    logMessage("WiFi credentials saved!");
}

bool connect_to_wifi()
{
    if (WiFi.status() != WL_CONNECTED) 
    {
        logMessage("Connecting to WiFi...");
        WiFi.begin(ssid, password);
    }

    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20) 
    {
        Serial.print(".");
        delay(500);
        attempt++;
    }

    if (WiFi.status() == WL_CONNECTED) 
    {
        path = "/device/" + WiFi.macAddress();
        logMessage("Connected to WiFi successfully!");
        logMessage("Set up path to firebase: " + String(path));
        return true;
    }
    else
    {
        logMessage("Failed to connect to WiFi!");
        return false;
    }
}

void connect_to_firebase()
{
    if (!firebase_setup_done)
    {
        config.api_key = API_KEY;
        config.database_url = DATABASE_URL; 
        auth.user.email = USER_EMAIL;
        auth.user.password = USER_PASSWORD;
        config.token_status_callback = tokenStatusCallback;
        
        Firebase.begin(&config, &auth);
        Firebase.reconnectWiFi(false);

        Serial.print("Getting User UID");
        while (auth.token.uid == "") 
        {
            Serial.print(".");
            delay(1000);
        }
        
        logMessage("\nUser UID: " + String(auth.token.uid.c_str()));
        firebase_setup_done = true;
    }
}

void refresh_firebase_token()
{
    if (Firebase.isTokenExpired()) 
    {
        Firebase.refreshToken(&config);
        Serial.println("Token refreshed");
    }
}

void get_config_data_from_firebase()
{
    logMessage("Configuration data is loaded from firebase");
}

void to_database(const String &folder, void *data)
{
    if (Firebase.ready()) 
    {
        bool success = false;

        if (folder == "temp") 
        {
            float *temperature = static_cast<float *>(data);
            success = Firebase.RTDB.pushFloat(&fbdo, path + "/temp", *temperature);
        } 
        else if (folder == "humid") 
        {
            float *humidity = static_cast<float *>(data);
            success = Firebase.RTDB.pushFloat(&fbdo, path + "/humid", *humidity);
        } 
        else if (folder == "time") 
        {
            String *time = static_cast<String *>(data);
            success = Firebase.RTDB.pushString(&fbdo, path + "/time", *time);
        } 
        else 
        {
            Serial.println("Unsupported folder type.");
            return;
        }

        if (success) 
        {
            Serial.println("\tSent: " + path + "/" + folder);  
        } 
        else 
        {
            Serial.println("Failed to send data: " + fbdo.errorReason());
        }
    } 
    else 
    {
        Serial.println("Firebase is not ready.");
    }
}

String get_formatted_time()
{
    configTime(25200, 3600, "pool.ntp.org");

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) 
    {
        return "Failed to obtain time";
    }

    char timeStr[20]; // Buffer for formatted time
    strftime(timeStr, sizeof(timeStr), "%d-%m-%Y-%H-%M-%S", &timeinfo);

    return String(timeStr);
}

void send_data_to_firebase()
{
    sensorData.timeStamp = get_formatted_time();

    if (alert_is_set) logMessage("Alert update!!!");
    else logMessage("Normal update");

    to_database("temp", (void*) &sensorData.temperature);
    to_database("humid", (void*) &sensorData.humidity);
    to_database("time", (void*) &sensorData.timeStamp);

    logMessage("Temperature: ", sensorData.temperature, "  |  Humidity: ", sensorData.humidity, "  |  Timestamp: ", sensorData.timeStamp);
}

void disconnect_if_allowed()
{
    if (disconnect_allowed)
    {
        WiFi.disconnect();
        fbdo.clear();
        logMessage("Disconnect to wifi and firebase to save energy");
    }
    else
    {
        logMessage("Disconnection to wifi and firebase is not allowed -> keeping connection alive");
    }
}

void read_sensor_data()
{
    if (sht3x.measure()) 
    {
        sensorData.humidity = sht3x.humidity();
        sensorData.temperature = sht3x.temperature();
        Serial.print("Temperature: ");
        Serial.print(sensorData.temperature, 1);
        Serial.print(" *C\tHumidity: ");
        Serial.print(sensorData.humidity, 1);
        Serial.print(" %RH");
        Serial.println();
    }
    else 
    {
        Serial.println("SHT3x read error");
    }
}

void check_sensor_data_to_send_alert()
{
    if 
    (
        sensorData.temperature > sensorData.maxTemperature || 
        sensorData.temperature < sensorData.minTemperature ||
        sensorData.humidity > sensorData.maxHumidity ||
        sensorData.humidity < sensorData.minHumidity
    ) 
    {
        alert_is_set = true;
        logMessage("Data exceed limits -> altert_is_set is set to true");
        
    } 
    else 
    {
        alert_is_set = false;
        logMessage("Data is normal -> altert_is_set is set to false");
    }
}

void take_wifi_credential_from_user_input()
{
    // Take ssid from user input
    Serial.println("SETUP MODE: Enter new WiFi SSID:");
    while (Serial.available() == 0) vTaskDelay(100);
    ssid = Serial.readStringUntil('\n');
    ssid.trim();

    // Take password from user input
    Serial.println("Enter new WiFi Password:");
    while (Serial.available() == 0) vTaskDelay(100);
    password = Serial.readStringUntil('\n');
    password.trim();
}


































