# IOT Energy monitor
Simple module based on open hardware devices: PZEM004T and ESP8266.

# Requirements
- Arduino tool
- libraries:
1) ArduinoJson (https://github.com/bblanchon/ArduinoJson)
2) PZEM004T(https://github.com/olehs/PZEM004T)


# Hardware
- ESP8266 module, eg: ESP8266MOD
- Energy monitor: PZEM004T

# Description
1) Istall latest 'Arduino' tool(https://www.arduino.cc/en/Main/Software)
2) Install ESP8266 toolchain(https://github.com/esp8266/Arduino)
3) Install Arduino ESP8266 filesystem uploader(https://github.com/esp8266/arduino-esp8266fs-plugin)
4) Intall Arduino library for PZEM004T module

# Functionality
1) User button usage:
- no press after restart -> starting 'normal mode'
- short press(<5s) after restart -> starting  'setup mode'
- long press (>5s) after restart -> restoring default data and starting 'setup mode'

# LED code
- 10 short blinks -> device is starting up after restart or power on
- 1 long blink -> device mode is WIFI_STA. It means that wifi module tries to connect to defined wifi network
- 2 long blinks -> device mode is WIFI_AP_STA. It means that wifi module create own wifi network(SSID and password are defined) and tries to connect to defined wifi network
- 3 long blinks -> device mode is WIFI_AP. Module tries to create own wifi network
- 2 short blinks (after 1 or 2 long blinks) -> device is connected to external network(STA).

# IoT node configuration
Each wifi mode(WIFI_STA/WIFI_AP_STA/WIFI_AP) supports http server. It allows for easly reconfiguration via web page. Configuration page is available on:
- 192.168.4.1 for wifi network created by node(mode: WIFI_AP_STA, WIFI_AP)
- assigned IP address(mode: WIFI_STA, WIFI_AP_STA).

