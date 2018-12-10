#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Button.h>
#include <ThingSpeak.h>
#include <ArduinoJson.h>

#include "src/WiFiManager/WiFiManager.h"
#include "private/config.h"

#define JSON_BUFFER 1200

Button resetButton(12);
WiFiManager wifiManager;
WiFiClient client;

String thingspeakWriteKey = "";
unsigned long thingspeakChannelId = 0;

void setup() {
  resetButton.begin();
  Serial.begin(9600);
  
  Serial.println("");
  Serial.println("Arduino AQI v1.0");
  Serial.println("");

  wifiManager.setDebugOutput(false);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setAPCallback(onWifiConfig);
  wifiManager.setSaveConfigCallback(onWifiConfigComplete);
  wifiManager.autoConnect(ACCESS_POINT);

  onWifiConnect();
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
  Serial.println("MAC Address: " + getMacAddress());
  Serial.println("Connected to network: " + WiFi.SSID());

  ThingSpeak.begin(client);

  String json = ThingSpeak.readRaw(THINGSPEAK_REGISTRY_CHANNEL_NUMBER, String("/feeds/?results=8000&api_key=") + THINGSPEAK_REGISTRY_API_KEY);
  int statusCode = ThingSpeak.getLastReadStatus();
 
  if (statusCode == OK_SUCCESS) {
    if (configThingspeak(json)) {
      onThingspeakRegistration();
    } else {
      Serial.println("ThingSpeak registration failed!");
    } 
  } else {
    Serial.println("ThingSpeak connection error!");
  }
}

void onThingspeakRegistration() {
  Serial.println("onThingspeakRegistration");
  Serial.println("thingspeakWriteKey: " + thingspeakWriteKey);
  Serial.println("thingspeakChannelId: " + String(thingspeakChannelId));
  Serial.println("");

  if (sendData(101, 102)) {
    Serial.println("Data sent to ThingSpeak");
  } else {
    Serial.println("ThingSpeak connection error!");
  }
}

boolean configThingspeak(String json) {
    StaticJsonBuffer<JSON_BUFFER> jsonBuffer;

    char jsonChar[JSON_BUFFER];
    json.toCharArray(jsonChar, JSON_BUFFER);
    
    JsonObject& root = jsonBuffer.parseObject(jsonChar);
    
    if (root.success()) {
      // iterate our thingspeak registration channel and match MAC addresses
      JsonArray& feeds = root["feeds"];
      
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

    return false;
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

boolean sendData(int number1, int number2) {
  ThingSpeak.setField(1, number1);
  ThingSpeak.setField(2, number2);

  int x = ThingSpeak.writeFields(thingspeakChannelId, thingspeakWriteKey.c_str());
  
  return (x == 200);
}

void resetWifi() {
  Serial.println("Resetting wifi…");
  WiFi.disconnect();
  wifiManager.startConfigPortal(ACCESS_POINT);
  onWifiConnect();
}

void loop() {
  if (resetButton.released()) {
    resetWifi();
  }
}
