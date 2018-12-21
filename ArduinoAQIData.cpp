#include "ArduinoAQIData.h"

ArduinoAQIData::ArduinoAQIData() {
  EEPROM.begin(512);
}

void ArduinoAQIData::begin() {
  Credentials myCredentials = getSavedCredentials();
  _wifiManager.setSavedSsid(myCredentials.ssid);
  _wifiManager.setSavedPassword(myCredentials.password);
  
  _wifiManager.setDebugOutput(false);
  _wifiManager.setConfigPortalTimeout(300);

  // @see
  // https://stackoverflow.com/questions/39803135/c-unresolved-overloaded-function-type
  _wifiManager.setAPCallback([](WiFiManager *myWiFiManager) -> void {
    Serial.println("Creating access point: " + myWiFiManager->getConfigPortalSSID());
  });

  _wifiManager.setSaveConfigCallback([]() -> void {
    Serial.println("WiFi config saved");
  });
  
  _wifiManager.autoConnect(ACCESS_POINT);
  
  _onWifiConnect();
}

bool ArduinoAQIData::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool ArduinoAQIData::write(float number1, float number2, float number3, float number4) {
  if (!isConnected()) {
    Serial.println("No wifi connection, cancel data send.");
    return false;
  }
  
  if (!_isRegistered) {
    Serial.println("Not registered, cancel data send and load config…");
    _loadThingspeakConfig();
    return false;
  }

  Serial.println("Sending data…");  
  ThingSpeak.setField(1, number1);
  ThingSpeak.setField(2, number2);
  ThingSpeak.setField(3, number3);
  ThingSpeak.setField(4, number4);

  int x = ThingSpeak.writeFields(_thingspeakChannelId, _thingspeakWriteKey.c_str());

  if (x == ERR_CONNECT_FAILED) {
    Serial.println("ThingSpeak connection error!");
    _connectionAttempts = 0;
    _reconnectWifi();
    return false;
  }
  
  if (x == ERR_BADAPIKEY || x == ERR_BADURL) {
    Serial.println("Wrong API key or bad endpoint. Reload config…");
    _loadThingspeakConfig();
    return false;
  }

  if (x == ERR_NOT_INSERTED) {
    Serial.println("ThingSpeak write failed! Probably reached rate limit."); 
  }

  if (x != OK_SUCCESS) {
    Serial.println("Error code: " + String(x)); 
  }
  
  return x == OK_SUCCESS;
}

void ArduinoAQIData::resetWifi() {  
  Serial.println("Resetting wifi in AP mode…");
  
  _connectionAttempts = 0;
  WiFi.disconnect();
  _wifiManager.startConfigPortal(ACCESS_POINT);
  
  _onWifiConnect();
}

String ArduinoAQIData::getMacAddress() {
  String myMac;
  
  byte mac[6];
  WiFi.macAddress(mac);

  myMac = String(mac[5], HEX) + ":";
  myMac += String(mac[4], HEX) + ":";
  myMac += String(mac[3], HEX) + ":";
  myMac += String(mac[2], HEX) + ":";
  myMac += String(mac[1], HEX) + ":";
  myMac += String(mac[0], HEX);
   
  return myMac; 
}

Credentials ArduinoAQIData::getSavedCredentials() {
  char ssid[WIFI_VARIABLE_LENGTH];
  char password[WIFI_VARIABLE_LENGTH];
  EEPROM.get(0, ssid);
  EEPROM.get(WIFI_VARIABLE_LENGTH, password);

  String mySsid = String(ssid);
  String myPassword = String(password);
  
  if (_isNull(0)) mySsid = "";
  if (_isNull(WIFI_VARIABLE_LENGTH)) myPassword = "";

  Credentials myCredentials;
  myCredentials.ssid = mySsid;
  myCredentials.password = myPassword;

  return myCredentials;
}

void ArduinoAQIData::_clearEEPROM() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 255);
  }
}

void ArduinoAQIData::_reconnectWifi() {
  _connectionAttempts++;

  // only try a few times, otherwise reset button is blocked
  if (_connectionAttempts > MAX_CONNECTION_ATTEMPTS) {
    Serial.println("Max connection attempts reached. Back to config…");
    resetWifi();
    return;
  }
  
  // retrieve saved credentials
  Credentials myCredentials = getSavedCredentials();

  if (myCredentials.ssid == "") {
    Serial.println("No EEPROM-stored credentials. Back to config…");
    resetWifi();
    return;
  }

  // attempt connection
  Serial.println("Attempting to reconnect to " + myCredentials.ssid + "… (" + _connectionAttempts + ")"); 
  WiFi.disconnect();
  _wifiManager.connectWifi(myCredentials.ssid, myCredentials.password);
  
  // success
  _onWifiConnect();
}

void ArduinoAQIData::_saveWifiCredentials() {
  // only save validated credentials
  if (!isConnected()) return;
  
  char mySsid[WIFI_VARIABLE_LENGTH];
  char myPassword[WIFI_VARIABLE_LENGTH];

  WiFi.SSID().toCharArray(mySsid, WIFI_VARIABLE_LENGTH);
  WiFi.psk().toCharArray(myPassword, WIFI_VARIABLE_LENGTH);
  
  EEPROM.put(0, mySsid);
  EEPROM.put(WIFI_VARIABLE_LENGTH, myPassword);
}

void ArduinoAQIData::_onWifiConnect () {
  Serial.println("_onWifiConnect");
  
  // no connection, retry
  if (!isConnected()) {
    _reconnectWifi();
    return;
  }

  // successfully joined as client
  _connectionAttempts = 0;
  
  Serial.println("MAC Address: " + getMacAddress());
  Serial.println("Connected to network: " + WiFi.SSID());

  // save credentials to EEPROM
  _saveWifiCredentials();

  // load ThingSpeak config
  _loadThingspeakConfig();
}

void ArduinoAQIData::_loadThingspeakConfig() {
  _isRegistered = false;
  ThingSpeak.begin(_client);

  String json = ThingSpeak.readRaw(THINGSPEAK_REGISTRY_CHANNEL_NUMBER, String("/feeds/?results=8000"), THINGSPEAK_REGISTRY_API_KEY);
  int statusCode = ThingSpeak.getLastReadStatus();
  
  if (statusCode == OK_SUCCESS) {
    if (_setThingspeakConfig(json)) {
      _isRegistered = true;
      Serial.println("ThingSpeak registration complete.");
    } else {
      Serial.println("ThingSpeak registration failed!");
    }
  } else {
    Serial.println("Failed to load ThingSpeak config!");
  }
}

bool ArduinoAQIData::_setThingspeakConfig(String json) {
    // try parsing the JSON
    StaticJsonBuffer<JSON_BUFFER> jsonBuffer;

    char jsonChar[JSON_BUFFER];
    json.toCharArray(jsonChar, JSON_BUFFER);
    
    JsonObject& root = jsonBuffer.parseObject(jsonChar);
    if (!root.success()) return false;
    if (!root.containsKey("feeds")) return false;

    JsonArray& feeds = root["feeds"];
    if (!feeds.success()) return false;

    // iterate our ThingSpeak registration channel and match MAC addresses
    for (JsonObject& feed : feeds) {
      if (feed.containsKey("field1") && feed.containsKey("field2")  && feed.containsKey("field3")) {
        const char* macAddress = feed["field1"];

        if (String(macAddress) == getMacAddress()) {
          // match found, set configuration
          const char* channelId = feed["field2"];
          _thingspeakChannelId = atoi(channelId);

          const char* writeKey = feed["field3"];
          _thingspeakWriteKey = String(writeKey);
          break;
        }
      }
    }

    return _thingspeakChannelId > 0;
}

bool ArduinoAQIData::_isNull(int loc) {
    // hack to check if first byte is 255 (assumed to be null)
    int firstChar = EEPROM.read(loc);
    return firstChar == 255;
}
