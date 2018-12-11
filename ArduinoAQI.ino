#include <Button.h>
#include "ArduinoAQIData.h"

Button resetButton(12);
ArduinoAQIData data;

void setup() {
  Serial.begin(9600);

  Serial.println("");
  Serial.println("Arduino AQI v1.0");
  Serial.println("");
  
  data.begin();
  resetButton.begin();
}

void loop() {
  if (resetButton.released()) {
    // data.resetWifi();

    if (data.write(101, 102)) {
      Serial.println("Data written to ThingSpeak");
    } else {
      Serial.println("ThingSpeak write error!");
    }
  }
}
