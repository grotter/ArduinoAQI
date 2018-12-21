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
  display.print("----");
  
  // initialize WiFi reset button
  resetButton.begin();
  
  // connect to WiFi
  // note: access point mode is blocking
  data.begin();
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

bool isRateLimited() {
  unsigned long now = millis();

  if (lastDataSend > now) {
    // clock overflowed
    lastDataSend = 0;
    return true;
  }

  unsigned long diff = now - lastDataSend;

  if (diff > THINGSPEAK_RATE_LIMIT_SECONDS * 1000) {
    return false;
  } else {
    return true;
  }
}

void loop() {
  if (data.isConnected()) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  
  if (resetButton.released()) {
    data.resetWifi();
  }
  
  if (pms.read(pmsData)) {
    // calculate AQI
    float aqi = CalculateAQI::getPM25AQI(pmsData.PM_AE_UG_2_5);
    Category category = CalculateAQI::getCategory(aqi);

    // display
    display.clear();
    display.print(getNumberWithLeadingZeros(round(aqi)));

    // wait a few secs
    if (isRateLimited()) return;

    // send data to ThingSpeak
    if (data.write(pmsData.PM_AE_UG_1_0, pmsData.PM_AE_UG_2_5, pmsData.PM_AE_UG_10_0, aqi)) {
      Serial.println("Data written to ThingSpeak");
    } else {
      Serial.println("ThingSpeak write error!");
    }

    lastDataSend = millis();
  }
}
