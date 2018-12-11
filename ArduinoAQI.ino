#include <Button.h>
#include "ArduinoAQIData.h"

Button resetButton(12);
ArduinoAQIData data;

void setup() {
  Serial.begin(9600);

  Serial.println("");
  Serial.println("Arduino AQI v1.0");
  Serial.println("");

  // LED_BUILTIN on the WeMos D1 R2 is inverted
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  data.begin();
  resetButton.begin();
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
}
