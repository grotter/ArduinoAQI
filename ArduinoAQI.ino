#include <SoftwareSerial.h>
#include <Button.h>
#include <SevenSegmentTM1637.h>
#include <PMS.h>
#include "ArduinoAQIData.h"
#include "CalculateAQI.h"

Button resetButton(PIN_RESET);
SevenSegmentTM1637 display(PIN_LED_CLK, PIN_LED_DIO);

SoftwareSerial pmsSerial(PIN_PMS_TX, PIN_PMS_RX);
PMS pms(pmsSerial);
PMS::DATA pmsData;

ArduinoAQIData data;
unsigned long lastDataSend = 0;
unsigned long spinupTimeStart = 0;
bool isSpinningUp = true;
bool isWifiMode = true;

void setup() {
  Serial.begin(9600);
  pmsSerial.begin(9600);

  Serial.println("");
  Serial.println("ArduinoAQI v1.0");
  Serial.println("");
  
  // LED_BUILTIN on the WeMos D1 R2 is inverted
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // initialize display
  display.begin();
  display.setBacklight(10);
  
  // @todo
  // spinup animation
  display.print("----");
  
  // initialize reset button
  resetButton.begin();

  spinupTimeStart = millis();
}

char* getNumberWithLeadingZeros(long num) {
  return getNumberWithLeadingZeros(num, DISPLAY_LENGTH);
}

char* getNumberWithLeadingZeros(long num, int displayLength) {
  char buffer[displayLength];
  String pattern = "%0" + String(displayLength) + "d";
  sprintf(buffer, pattern.c_str(), num);
  
  return buffer;
}

bool isTimeExceeded(unsigned long &startTime, int numSeconds) {
  unsigned long now = millis();

  if (startTime > now) {
    // clock overflowed
    startTime = 0;
    return false;
  }

  unsigned long diff = now - startTime;

  if (diff > numSeconds * 1000) {
    return true;
  } else {
    return false;
  }
}

void connectWifi() {
  display.clear();
  display.print("----");
  data.begin();
}

void stopSpinup() {
  isSpinningUp = false;

  // @todo
  // stop spinup animation
  display.clear();
  display.print("----");
}

void loop() {
  // reset button switches behavior after spinup
  if (resetButton.released()) {
    if (isSpinningUp) {
      // @todo
      // save preference to EEPROM

      // proceed without wifi
      isWifiMode = false;
      stopSpinup();
    } else {
      if (isWifiMode) {
        // reset wifi
        data.resetWifi();
      } else {
        // switch to wifi mode
        isWifiMode = true;
        connectWifi();
      }
    }
  }
  
  if (isSpinningUp) {
    if (isTimeExceeded(spinupTimeStart, SPINUP_SECONDS)) {
      stopSpinup();
      
      if (isWifiMode) {
        connectWifi();
      }
    }

    return;
  }

  if (data.isConnected()) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  if (pms.read(pmsData)) {
    // calculate AQI
    float aqi = CalculateAQI::getPM25AQI(pmsData.PM_AE_UG_2_5);
    Category category = CalculateAQI::getCategory(aqi);

    // display
    display.clear();
    display.print(getNumberWithLeadingZeros(round(aqi)));

    if (!isWifiMode) return;

    // wait a few secs
    if (!isTimeExceeded(lastDataSend, THINGSPEAK_RATE_LIMIT_SECONDS)) return;

    // send data to ThingSpeak
    if (data.write(pmsData.PM_AE_UG_1_0, pmsData.PM_AE_UG_2_5, pmsData.PM_AE_UG_10_0, aqi)) {
      Serial.println("Data written to ThingSpeak");
    } else {
      Serial.println("ThingSpeak write error!");
    }

    lastDataSend = millis();
  }
}
