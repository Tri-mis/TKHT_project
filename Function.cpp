#include "esp32-hal-adc.h"
#include "esp32-hal-gpio.h"
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

    if (from_database()) 
    {
        send_data_interval = send_data_interval_normal;
        logMessage("Configuration data is loaded from firebase");
    }

    else logMessage("Failed to load configuration data from firebase");
}

void to_database(const String &folder, void *data)
{
    if (Firebase.ready()) 
    {
        bool success = false;

        if (folder == "/data/temp") 
        {
            float *temperature = static_cast<float *>(data);
            success = Firebase.RTDB.pushFloat(&fbdo, path + folder, *temperature);
        } 
        else if (folder == "/data/humid") 
        {
            float *humidity = static_cast<float *>(data);
            success = Firebase.RTDB.pushFloat(&fbdo, path + folder, *humidity);
        } 
        else if (folder == "/data/time") 
        {
            String *time = static_cast<String *>(data);
            success = Firebase.RTDB.pushString(&fbdo, path + folder, *time);
        } 
        else if (folder == "/actuator/buzzer")
        {
            bool *buzzer_on = static_cast<bool *>(data);
            success = Firebase.RTDB.setBool(&fbdo, path + folder, *buzzer_on);
        }
        else if (folder == "/data/battery")
        {
            int *battery = static_cast<int *>(data);
            success = Firebase.RTDB.setInt(&fbdo, path + folder, *battery);
        }
        else if(folder == "/device_data")
        {
            String *device_data = static_cast<String *>(data);
            success = Firebase.RTDB.setString(&fbdo, path + folder + + "/" + String(sensorData.timeStamp), *device_data);
        }
        else 
        {
            Serial.println("Unsupported folder type.");
            return;
        }

        if (success) 
        {
            Serial.println("\tSent: " + path + folder);  
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

bool from_database()
{
    if 
    (
        Firebase.RTDB.getFloat(&fbdo, path + "/config/maxHumid", &sensorData.maxHumidity) &&
        Firebase.RTDB.getFloat(&fbdo, path + "/config/minHumid", &sensorData.minHumidity) &&
        Firebase.RTDB.getFloat(&fbdo, path + "/config/maxTemp", &sensorData.maxTemperature) &&
        Firebase.RTDB.getFloat(&fbdo, path + "/config/minTemp", &sensorData.minTemperature) &&
        Firebase.RTDB.getInt(&fbdo, path + "/config/update_interval_normal", &send_data_interval_normal) &&
        Firebase.RTDB.getInt(&fbdo, path + "/config/update_interval_alert", &send_data_interval_in_alert)
    )
    {
        logMessage("maxHumid: ", sensorData.maxHumidity, " | minHumid: ", sensorData.minHumidity, " | maxTemp: ", sensorData.maxTemperature, " | minTemp: ", sensorData.minTemperature);
        logMessage("send_data_interval_normal: ", send_data_interval_normal, " seconds | update_interval_alert: ", send_data_interval_in_alert, " seconds");
        return true;
    }
    else 
    {
        logMessage(fbdo.errorReason());
        return false;
    }
}

void get_formatted_time()
{
    configTime(25200, 3600, "pool.ntp.org");

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) 
    {
        logMessage("Failed to obtain time");
    }

    sensorData.timeStamp = mktime(&timeinfo);

    // char timeStr[20]; // Buffer for formatted time
    // strftime(timeStr, sizeof(timeStr), "%d-%m-%Y-%H-%M-%S", &timeinfo);

    // return String(timeStr);
}

String formatData(float temperature, float humidity, int battery) {

    String result;
    result += String(temperature, 2) + "|";
    result += String(humidity, 2) + "|";
    result += String(battery);

    return result;
}

void send_data_to_firebase()
{
    get_formatted_time();

    if (alert_is_set) logMessage("Alert update!!!");
    else logMessage("Normal update");

    String data_to_sent = formatData(sensorData.temperature, sensorData.humidity, sensorData.battery);

    // to_database("/data/temp", (void*) &sensorData.temperature);
    // to_database("/data/humid", (void*) &sensorData.humidity);
    // to_database("/data/time", (void*) &sensorData.timeStamp);
    // to_database("/data/battery", (void*) &sensorData.battery);

    to_database("/device_data", (void*) &data_to_sent);

    logMessage("Temperature: ", sensorData.temperature, "  |  Humidity: ", sensorData.humidity, "  |  Timestamp: ", sensorData.timeStamp);
}

void disconnect_if_allowed()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        if (disconnect_allowed && !do_an_do_luong)
        {
            WiFi.disconnect();
            fbdo.clear();
            logMessage("Disconnection to wifi and firebase is allowed -> disconnect to wifi and firebase to save energy");
        }
        else
        {
            logMessage("Disconnection to wifi and firebase is NOT allowed -> keeping connection alive");
            if (do_an_do_luong)
            logMessage("Dang lam do an do luong");
        }
    }
}

void read_sensor_data()
{
    if (sht3x.measure()) 
    {
        sensorData.humidity = round(sht3x.humidity() * 100) / 100;
        sensorData.temperature = round(sht3x.temperature() * 100) / 100;
        logMessage("Temperature: ", sensorData.temperature, " *C  |  Humidity: ", sensorData.humidity, " %RH");
    }
    else 
    {
        logMessage("SHT3x read error");
        Wire.end();
        Wire.begin();
        while (!sht3x.begin())
        {
            logMessage("SHT3x not found !");
            delay(1000);
        }
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
        logMessage("Data exceed limits -> alert_is_set is set to true");
        
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

void begin_data_streamming()
{
    Firebase.RTDB.beginStream(&stream, path + "/actuator");
    Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
    logMessage("Begin data streamming");
}

void stop_data_streaming()
{
    Firebase.RTDB.endStream(&stream);
    stream.clear();
    logMessage("Stop data streaming");
}

void streamCallback(FirebaseStream data)
{
    if (data.dataTypeEnum() == fb_esp_rtdb_data_type_boolean)
    {
        buzzer_on = data.boolData();
        if (!buzzer_on)
        {
            digitalWrite(BUZZER_PIN, buzzer_on);
            logMessage("User turn buzzer off");
        }
        else
        {
            logMessage("Error turning off buzzer");
        }
    }
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    logMessage("stream timeout, resuming...\n");
  if (!stream.httpConnected())
    logMessage("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}


void start_taking_wifi_credentials_using_bluetooth()
{
    // Reset stored credentials
    ssid = "";
    password = "";

    logMessage("Starting Bluetooth and waiting for WiFi credentials...");

    if (!bluetooth_is_init)
    {

        BLEDevice::init("ESP32");

        // Create the BLE Server
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());

        // Create BLE Service
        BLEService* pService = pServer->createService(SERVICE_UUID);

        // Create WiFi Credential Characteristic
        pWiFiCharacteristic = pService->createCharacteristic(
            WIFI_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_WRITE
        );
        pWiFiCharacteristic->addDescriptor(new BLE2902());


        // Create WiFi MAC Characteristic
        pMacCharacteristic = pService->createCharacteristic(
            MAC_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ
        );
        pMacCharacteristic->addDescriptor(new BLE2902());
        // Get WiFi MAC Address instead of Bluetooth MAC
        String macStr = WiFi.macAddress();
        pMacCharacteristic->setValue(macStr.c_str());
        logMessage("Mac address is: ", macStr.c_str());

        // Start the service
        pService->start();

        // Start Advertising
        BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);
        BLEDevice::startAdvertising();

        logMessage("BLE Advertising started. Waiting for Web BLE connection...");

        bluetooth_is_init = true;
    }

    // Blocking loop to wait for credentials
    while (ssid.length() == 0 || password.length() == 0) 
    {
        String value = pWiFiCharacteristic->getValue();
        if (value.length() > 0) 
        {
            int separator = value.indexOf("|");
            if (separator != -1) 
            {
                ssid = value.substring(0, separator);
                password = value.substring(separator + 1);
                logMessage("Received WiFi Credentials - SSID: ", ssid, " | PASSWORD: ", password);
                break;
            }
        }
        delay(100);  // Prevents excessive CPU usage
    }

    logMessage("WiFi credentials received. Proceeding...");
}


void stop_bluetooth() 
{
    logMessage("Stopping Bluetooth...");

    if (pServer) {
        pServer->getAdvertising()->stop();  // Stop BLE advertising
        BLEDevice::deinit();  // Fully shut down BLE
        pServer = nullptr;  // Reset pointer
        bluetooth_is_init = false;
    }

    logMessage("Bluetooth Stopped.");
}


void change_task(bool change_to_setup)
{
    if (change_to_setup)
    {
        is_setup_mode = true;
        digitalWrite(YELLOW_LED, 1);
        digitalWrite(GREEN_LED, 0);
        vTaskResume(setupTaskHandle);
        vTaskSuspend(workingTaskHandle);
    }
    else
    {
        is_setup_mode = false;
        digitalWrite(GREEN_LED, 1);
        digitalWrite(YELLOW_LED, 0);
        vTaskResume(workingTaskHandle);
        vTaskSuspend(setupTaskHandle);
    }
}

void led_flicker(int interval, int times, int led)
{
    int status = digitalRead(led);

    if (!status)
    {
        for (int i = 0; i < times; i ++)
        {
            digitalWrite(led, 1);
            delay(interval);
            digitalWrite(led, 0);
            delay(interval);
        }
    }
    else
    {
        for (int i = 0; i < times; i ++)
        {
            digitalWrite(led, 0);
            delay(interval);
            digitalWrite(led, 1);
            delay(interval);
        }
    }
}

void MyServerCallbacks::onConnect(BLEServer* pServer) {
    logMessage("BLE Device Connected");
    led_flicker(100, 3, YELLOW_LED);
}

void MyServerCallbacks::onDisconnect(BLEServer* pServer) {
    logMessage("BLE Device Disconnected. Restarting advertising...");
    led_flicker(100, 3, YELLOW_LED);
    pServer->startAdvertising();  // Restart advertising when disconnected
}

void read_battery()
{
    sensorData.battery = map(analogReadMilliVolts(BATTERY_PIN), MIN_BAT_VOLT, MAX_BAT_VOLT, 0, 100);
    logMessage("Battery: ", sensorData.battery);
}

SensorData::SensorData()
{
    read_battery();
}





















