# ESP32 Weather-Based Frost Warning

A personal embedded-systems project developed approximately one year ago as my first project using the Arduino Nano ESP32-S3R8 IoT development board and C++, made for my parents.

The goal was to retrieve weather forecast data and check whether the temperature was expected to fall to 2°C or below during the night. The ESP32 uses Wi-Fi to retrieve forecast data from OpenWeatherMap and processes it using ArduinoJson.

The original idea was to turn on a lamp if frost was expected. The ESP32 then enters deep sleep until its next scheduled check.
