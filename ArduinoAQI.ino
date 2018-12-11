#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Button.h>
#include <ThingSpeak.h>
#include <ArduinoJson.h>

#include "src/WiFiManager/WiFiManager.h"
#include "private/config.h"

#define JSON_BUFFER 5000
#define WIFI_VARIABLE_LENGTH 60
#define MAX_CONNECTION_ATTEMPTS 10

Button resetButton(12);
WiFiClient client;
int connectionAttempts = 0;
WiFiManager wifiManager;

bool isRegistered = false;
String thingspeakWriteKey = "";
unsigned long thingspeakChannelId = 0;

void setup() {
  resetButton.begin();
  
  EEPROM.begin(512);
  Serial.begin(9600);

  Serial.println("");
  Serial.println("Arduino AQI v1.0");
  Serial.println("");
  
  wifiManager.setDebugOutput(false);
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setAPCallback(onWifiConfig);
  wifiManager.setSaveConfigCallback(onWifiConfigComplete);
  wifiManager.autoConnect(ACCESS_POINT);
  
  onWifiConnect();
}

void clearEEPROM() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 255);
  }
}

void resetWifi() {  
  Serial.println("Resetting wifi…");
  WiFi.disconnect();
  wifiManager.startConfigPortal(ACCESS_POINT);
  
  onWifiConnect();
}

void reconnectWifi() {
  connectionAttempts++;

  // only try a few times, otherwise reset button is blocked
  if (connectionAttempts > MAX_CONNECTION_ATTEMPTS) {
    connectionAttempts = 0;
    Serial.println("Max connection attempts reached. Back to config…");
    resetWifi();
    return;
  }
  
  // check for saved credentials
  if (isNull(0) || isNull(WIFI_VARIABLE_LENGTH)) {
    connectionAttempts = 0;
    Serial.println("No EEPROM-stored credentials. Back to config…");
    resetWifi();
    return;
  }

  // retrieve saved credentials
  char ssid[WIFI_VARIABLE_LENGTH];
  char password[WIFI_VARIABLE_LENGTH];
  EEPROM.get(0, ssid);
  EEPROM.get(WIFI_VARIABLE_LENGTH, password);

  // attempt connection
  Serial.println("Attempting to reconnect to " + String(ssid) + "… (" + connectionAttempts + ")"); 
  WiFi.disconnect();
  wifiManager.connectWifi(String(ssid), String(password));
  
  // success
  onWifiConnect();
}

void saveWifiCredentials() {
  // only save validated credentials
  if (WiFi.status() != WL_CONNECTED) return;
  
  char mySsid[WIFI_VARIABLE_LENGTH];
  char myPassword[WIFI_VARIABLE_LENGTH];

  WiFi.SSID().toCharArray(mySsid, WIFI_VARIABLE_LENGTH);
  WiFi.psk().toCharArray(myPassword, WIFI_VARIABLE_LENGTH);
  
  EEPROM.put(0, mySsid);
  EEPROM.put(WIFI_VARIABLE_LENGTH, myPassword);
}

void onWifiConfig (WiFiManager *myWiFiManager) {
  Serial.println("onWifiConfig");
  Serial.println("Creating access point: " + myWiFiManager->getConfigPortalSSID());
}

void onWifiConfigComplete () {
  Serial.println("onWifiConfigComplete");
}

void onWifiConnect () {
  Serial.println("onWifiConnect");
  
  // no connection, retry
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWifi();
    return;
  }

  // successfully joined as client
  Serial.println("MAC Address: " + getMacAddress());
  Serial.println("Connected to network: " + WiFi.SSID());

  // save credentials to EEPROM
  saveWifiCredentials();

  // load ThingSpeak config
  loadThingspeakConfig();
}

void loadThingspeakConfig() {
  isRegistered = false;
  ThingSpeak.begin(client);

  String json = ThingSpeak.readRaw(THINGSPEAK_REGISTRY_CHANNEL_NUMBER, String("/feeds/?results=8000&api_key=") + THINGSPEAK_REGISTRY_API_KEY);
  int statusCode = ThingSpeak.getLastReadStatus();
 
  if (statusCode == OK_SUCCESS) {
    if (setThingspeakConfig(json)) {
      isRegistered = true;
      Serial.println("ThingSpeak registration complete.");
    } else {
      Serial.println("ThingSpeak registration failed!");
    }
  } else {
    Serial.println("Failed to load ThingSpeak config!");
  }
}

bool setThingspeakConfig(String json) {
    // try parsing the JSON
    StaticJsonBuffer<JSON_BUFFER> jsonBuffer;

    char jsonChar[JSON_BUFFER];
    json.toCharArray(jsonChar, JSON_BUFFER);
    
    JsonObject& root = jsonBuffer.parseObject(jsonChar);
    if (!root.success()) return false;

    JsonArray& feeds = root["feeds"];
    if (!feeds.success()) return false;

    // iterate our ThingSpeak registration channel and match MAC addresses
    for (JsonObject& feed : feeds) {
      if (feed.containsKey("field1") && feed.containsKey("field2")  && feed.containsKey("field3")) {
        const char* macAddress = feed["field1"];

        if (String(macAddress) == getMacAddress()) {
          // match found, set configuration
          const char* channelId = feed["field2"];
          thingspeakChannelId = atoi(channelId);

          const char* writeKey = feed["field3"];
          thingspeakWriteKey = String(writeKey);
          break;
        }
      }
    }

    return (thingspeakChannelId > 0);
}

/*
if (sendData(101, 102)) {
  Serial.println("Data written to ThingSpeak");
} else {
  Serial.println("ThingSpeak write error!");
}
*/
bool sendData(int number1, int number2) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No wifi connection, cancel data send.");
    return false;
  }
  
  if (!isRegistered) {
    Serial.println("Not registered, cancel data send and load config…");
    loadThingspeakConfig();
    return false;
  }

  Serial.println("Sending data…");  
  ThingSpeak.setField(1, number1);
  ThingSpeak.setField(2, number2);

  int x = ThingSpeak.writeFields(thingspeakChannelId, thingspeakWriteKey.c_str());
  
  if (x == ERR_BADAPIKEY || x == ERR_BADURL) {
    Serial.println("Wrong API key or bad endpoint. Reload config…");
    loadThingspeakConfig();
    return false;
  }

  if (x == ERR_NOT_INSERTED) {
    Serial.println("ThingSpeak write failed! Probably reached rate limit."); 
  }
  
  return x == OK_SUCCESS;
}

bool isNull(int loc) {
    // hack to check if first byte is 255 (assumed to be null)
    int firstChar = EEPROM.read(loc);
    return (firstChar == 255);
}

String getMacAddress() {
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

void loop() {
  if (resetButton.released()) {
    resetWifi();
  }
}
