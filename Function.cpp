#include "TKHT_lib.h"


// LOAD WIFI CREDENTIALS FROM EEPROM
bool loadWiFiCredentials()
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
    Serial.print("-> Loaded SSID: "); Serial.println(ssid);
    Serial.print("-> Loaded Password: "); Serial.println(password);

    return true;
}

// SAVE WIFI CREDENTIALS TO EEPROM
void saveWiFiCredentials(String newSSID, String newPassword) 
{
    int ssid_len = newSSID.length();
    EEPROM.write(SSID_ADDR, ssid_len);
    for (int i = 0; i < ssid_len; i++) 
    {
        EEPROM.write(SSID_ADDR + 1 + i, newSSID[i]);
    }

    int password_len = newPassword.length();
    EEPROM.write(PASS_ADDR, password_len);
    for (int i = 0; i < password_len; i++) 
    {
        EEPROM.write(PASS_ADDR + 1 + i, newPassword[i]);
    }

    EEPROM.commit();  // **Ensure data is actually written to EEPROM**
    Serial.println("WiFi credentials saved!");
}

void readSensorData()
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
  } else {
    Serial.println("SHT3x read error");
  }

  checkSensorData();
}

void connectWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
  }

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) 
  {
      Serial.print(".");
      delay(500);
      attempt++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    path = "/device/" + WiFi.macAddress();
    setupFirebase();
  }
}

void disconnectWiFi()
{
  if (disconnect_allowed){
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
  } 
}

void sendData()
{
  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected");

      configTime(25200, 3600, "pool.ntp.org");
      getFormattedTime();

      if(setAlert) Serial.println("Alert update!!!");

      to_database("temp", (void*) &sensorData.temperature);
      to_database("humid", (void*) &sensorData.humidity);
      to_database("time", (void*) &sensorData.timeStamp);

      Serial.print("Temperature: "); Serial.print(sensorData.temperature);
      Serial.print("  |  Humidity: "); Serial.print(sensorData.humidity);
      Serial.print("  |  Timestamp: "); Serial.println(sensorData.timeStamp);

      fbdo.clear();

      disconnectWiFi();
  } else {
      Serial.println("Failed to connect");
  }
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Failed to obtain time";
  }
  char timeStr[20]; // Buffer for formatted time
  strftime(timeStr, sizeof(timeStr), "%d-%m-%Y-%H-%M-%S", &timeinfo);

  sensorData.timeStamp = String(timeStr);
  return sensorData.timeStamp;
}

void checkSensorData()
{
  if(
    sensorData.temperature > sensorData.maxTemperature || 
    sensorData.temperature < sensorData.minTemperature ||
    sensorData.humidity > sensorData.maxHumidity ||
    sensorData.humidity < sensorData.minHumidity
  ){
    setAlert = true;
    if (!just_send_alert)
    {
      connectWiFi();
      sendData();
      just_send_alert = true;
    }
  }
  else
  {
    just_send_alert = false;
    setAlert = false;
  }
}

void setupFirebase() 
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
    while (auth.token.uid == "") {
      Serial.print(".");
      delay(1000);
    }
    
    Serial.println("\nUser UID: " + String(auth.token.uid.c_str()));

    firebase_setup_done = true;
  }


  if (Firebase.isTokenExpired()) {
    Firebase.refreshToken(&config);
    Serial.println("Token refreshed");
  }
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

void fetchConfiguration(){
  Serial.println("fetched configuration");
}




