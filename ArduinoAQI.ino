#include <SoftwareSerial.h>
#include <Button.h>
#include <PMS.h>
#include "SevenSegmentSnake.h"
#include "ArduinoAQIData.h"
#include "CalculateAQI.h"

static const uint16_t PMS_TIMEOUT_MS = 2000;

Button resetButton(PIN_RESET);
SevenSegmentSnake display(PIN_LED_CLK, PIN_LED_DIO);

SoftwareSerial pmsSerial(PIN_PMS_TX, PIN_PMS_RX);
PMS pms(pmsSerial);
PMS::DATA pmsData;

ArduinoAQIData data;
SensorData sensorData;
unsigned long lastDataSend = 0;
unsigned long lastSpinupStep = 0;
unsigned long spinupTimeStart = 0;
unsigned long programStartTime = 0;
bool isSpinningUp = true;
bool isWifiMode = true;

void setup() {
  Serial.begin(9600);
  pmsSerial.begin(9600);

  Serial.println("");
  Serial.println("ArduinoAQI");
  Serial.println("");

  // LED_BUILTIN on the WeMos D1 R2 is inverted
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // initialize display
  Serial.println("Starting in spinup mode…");
  display.begin();
  display.setBacklight(10);

  // check for saved wifi mode
  if (wasWifiModeDisabled()) {
    Serial.println("Wifi mode previously disabled. Skipping spinup…");
    stopSpinup();
    isWifiMode = false;
  }
  
  // initialize reset button
  resetButton.begin();

  // initialize sensor data
  resetSensorAverages();
  
  programStartTime = millis();
  spinupTimeStart = millis();
}

void resetSensorAverages() {
  sensorData = {
    PM_AE_UG_1_0: 0,
    PM_AE_UG_2_5: 0,
    PM_AE_UG_10_0: 0,
    AQI: 0,
    numReads: 0  
  };
}

void saveWifiMode() {
  byte disableWifi = isWifiMode ? 0 : 1;
  EEPROM.put(WIFI_VARIABLE_LENGTH * 2, disableWifi);
  EEPROM.commit();
}

bool wasWifiModeDisabled() {
  byte disableWifi = EEPROM.read(WIFI_VARIABLE_LENGTH * 2);
  return disableWifi == 1;
}

char *getNumberWithLeadingZeros(long num, char *buffer, int displayLength) {
  snprintf(buffer, ((size_t) displayLength) + 1, "%0*ld", displayLength, num);
  return buffer;
}

bool isTimeExceeded(unsigned long &startTime, unsigned long numMilliseconds) {
  unsigned long now = millis();

  if (startTime > now) {
    // clock overflowed
    startTime = 0;
    return false;
  }

  unsigned long diff = now - startTime;

  if (diff > numMilliseconds) {
    return true;
  } else {
    return false;
  }
}

void clearDisplay() {
  display.clear();
  display.print("----");
}

void setDisplay(const char *text) {
  display.clear();
  display.print(text);
}

void onAPMode(WiFiManager *myWiFiManager) {
  Serial.println("Creating access point: " + myWiFiManager->getConfigPortalSSID());
  setDisplay("conn");
}

void connectWifi() {
  clearDisplay();
  data.setAPCallback(onAPMode);
  data.begin();
}

void stopSpinup() {
  isSpinningUp = false;
  clearDisplay();
}

void onResetRelease() {
  // reset button switches behavior after spinup
  if (isSpinningUp) {
    Serial.println("Start without wifi.");
    isWifiMode = false;
    saveWifiMode();
    stopSpinup();
  } else {
    if (isWifiMode) {
      Serial.println("Reset wifi config…");
      data.resetWifi();
    } else {
      Serial.println("Switch to wifi mode. Attempt to connect…");
      isWifiMode = true;
      saveWifiMode();
      connectWifi();
    }
  }
}

unsigned long getRateLimitSeconds() {
  float messagesPerDay = MESSAGES_PER_YEAR / 365;
  float secondsPerDay = 60 * 60 * 24;
  float rateLimitSeconds = (secondsPerDay / messagesPerDay) * data.numRegisteredDevices;

  if (rateLimitSeconds < THINGSPEAK_RATE_LIMIT_SECONDS) {
    rateLimitSeconds = THINGSPEAK_RATE_LIMIT_SECONDS;
  }

  return ceil(rateLimitSeconds);
}

void processSensorData(bool trace) {
  if (pms.readUntil(pmsData, PMS_TIMEOUT_MS)) {
    if (trace) {
      Serial.print("PM 1.0 (ug/m3): ");
      Serial.println(pmsData.PM_AE_UG_1_0);
    
      Serial.print("PM 2.5 (ug/m3): ");
      Serial.println(pmsData.PM_AE_UG_2_5);
    
      Serial.print("PM 10.0 (ug/m3): ");
      Serial.println(pmsData.PM_AE_UG_10_0);
    
      Serial.println();
    }
    
    // calculate AQI
    float aqi = CalculateAQI::getPM25AQI(pmsData.PM_AE_UG_2_5);
    
    // display realtime unaveraged AQI
    char displayBuffer[DISPLAY_LENGTH + 1];
    setDisplay(getNumberWithLeadingZeros(round(aqi), displayBuffer, DISPLAY_LENGTH));
    
    if (!isWifiMode) return;

    // update for averaging
    CalculateAQI::updateSensorData(sensorData, pmsData, aqi);
    
    // wait a few secs
    unsigned long rateLimitSeconds = getRateLimitSeconds();
    if (!isTimeExceeded(lastDataSend, rateLimitSeconds * 1000)) return;
    
    // send averaged data to ThingSpeak
    SensorData averagedData = CalculateAQI::getAveragedData(sensorData);
    
    if (data.write(averagedData.PM_AE_UG_1_0, averagedData.PM_AE_UG_2_5, averagedData.PM_AE_UG_10_0, averagedData.AQI)) {
      Serial.println("Data written to ThingSpeak");
    } else {
      Serial.println("ThingSpeak write error!");
    }

    resetSensorAverages();
  
    lastDataSend = millis();
  } else {
    setDisplay("Err");
  }
}

void loop() {
  // hard restart every few hours
  if (isTimeExceeded(programStartTime, HOURS_TO_RESTART * 60 * 60 * 1000)) {
    data.disconnectAndRestart(false);
    return;
  }
  
  if (resetButton.released()) {
    onResetRelease();
  }
  
  if (isSpinningUp) {
    // process spinup animation
    if (isTimeExceeded(lastSpinupStep, 10)) {
      display.process();
      lastSpinupStep = millis();
    }
    
    if (isTimeExceeded(spinupTimeStart, SPINUP_SECONDS * 1000)) {
      stopSpinup();
      
      if (isWifiMode) {
        Serial.println("Spinup complete! Connect to wifi…");
        connectWifi();
      } else {
        Serial.println("Spinup complete! Skip wifi…");
      }
    }

    return;
  }

  if (data.isConnected()) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  processSensorData(true);
}
