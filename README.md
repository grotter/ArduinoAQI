# ArduinoAQI v1.0

This device continuously monitors atmospheric particulate matter. It sends sensor data to a registered ThingSpeak channel and displays realtime Air Quality Index [calculations](https://en.wikipedia.org/wiki/Air_quality_index#United_States). It was designed to be cheap and efficient.

## Hardware
* ESP8266 Arduino-compatible microcontroller. I‘m using a [WeMos D1 R2](https://wiki.wemos.cc/products:d1:d1), pins will need to be transposed for other controllers
* [Plantower PMS5003](https://cdn-shop.adafruit.com/product-files/3686/plantower-pms5003-manual_v2-3.pdf) laser particle concentration sensor
* [TM1637](http://dx.com/p/0-36-led-4-digit-display-module-for-arduino-black-blue-works-with-official-arduino-boards-254978) seven segment LED display module
* A momentary switch for resetting the wifi configuration

## Dependencies
* A minimally hacked version (included in this repo) of tzapu’s [WiFiManager](https://github.com/tzapu/WiFiManager)
* [Arduino_JSON](https://github.com/arduino-libraries/Arduino_JSON)
* fu-hsi‘s [library](https://github.com/fu-hsi/PMS) for Plantower PMS x003 family sensors
* bremme‘s [library](https://github.com/bremme/arduino-tm1637) for controlling TM1637 display modules
* mathwork‘s [ThingSpeak library](https://github.com/mathworks/thingspeak-arduino) – note: latest version is buggy, use [release v1.4.3](https://github.com/mathworks/thingspeak-arduino/releases/tag/1.4.3) 
* madleech‘s [Button library](https://github.com/madleech/Button)

## Usage

To setup wifi connectivity, the device will create an *ArduinoAQI Setup* access point when first run. Connect to this network with a phone or computer to set your wifi credentials.

The device powers up in spinup mode for an initial ten seconds. Set wifi mode to **off** by clicking the reset button during spinup. In wifi mode, the reset button will clear your network credentials and relaunch the configuration access point.

## Wiring Diagram
![ArduinoAQI wiring diagram](ArduinoAQI_bb.png)

## JavaScript Chart

I've included a small graph built on [Chart.js](https://github.com/chartjs/Chart.js) to view your sensor data and any optional PurpleAir sensors. The graph uses the following GET variables:

* `key` - Your ThingSpeak read API key
* `id` - Your ThingSpeak sensor ID
* `purpleAirSensorIds` (optional) - A comma-delimited collection of PurpleAir sensor IDs
* `results` (optional) - Number of results, defaults to 100

### ♥
