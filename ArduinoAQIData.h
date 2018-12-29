#ifndef ArduinoAQIData_h
#define ArduinoAQIData_h

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ThingSpeak.h>
#include <ArduinoJson.h>
#include "src/WiFiManager/WiFiManager.h"
#include "private/config.h"

#define JSON_BUFFER 5000

struct Credentials {
  String ssid;
  String password;
};

class ArduinoAQIData {
  public:
    ArduinoAQIData();

    void begin();
    void restart();
    void disconnectAndRestart(bool clearCredentials);
    void resetWifi();
    String getMacAddress();
    bool isConnected();
    bool write(float number1, float number2, float number3, float number4);
    void setAPCallback( void (*func)(WiFiManager*) );
    Credentials getSavedCredentials();
  private:
    WiFiClient _client;
    WiFiManager _wifiManager;
    int _connectionAttempts = 0;
    bool _isRegistered = false;
    String _thingspeakWriteKey = "";
    unsigned long _thingspeakChannelId = 0;
    
    void _onWifiConnect();    
    void _clearEEPROMCredentials();
    void _reconnectWifi();
    void _saveWifiCredentials();
    void _loadThingspeakConfig();
    bool _setThingspeakConfig(String json);
    bool _isNull(int loc);
};

#endif
