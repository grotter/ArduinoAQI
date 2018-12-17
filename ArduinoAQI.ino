#include <Button.h>
#include <SevenSegmentTM1637.h>
#include "ArduinoAQIData.h"

#define DISPLAY_LENGTH 4

Button resetButton(PIN_RESET);
SevenSegmentTM1637 display(PIN_LED_CLK, PIN_LED_DIO);
ArduinoAQIData data;

void setup() {
  Serial.begin(9600);

  Serial.println("");
  Serial.println("Arduino AQI v1.0");
  Serial.println("");

  // LED_BUILTIN on the WeMos D1 R2 is inverted
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // initialize display
  display.begin();
  display.setBacklight(25);

  // connect to WiFi
  // note: access point mode is blocking
  data.begin();

  // initialize WiFi reset button
  resetButton.begin();
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

void loop() {    
  if (data.isConnected()) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  
  if (resetButton.released()) {
    // data.resetWifi();

    if (data.write(101, 102)) {
      Serial.println("Data written to ThingSpeak");
    } else {
      Serial.println("ThingSpeak write error!");
    }
  }

  long num = random(0, 9999);
  display.clear();
  display.print(getNumberWithLeadingZeros(num));
  delay(300);
}
