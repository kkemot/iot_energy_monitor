# IOT Energy monitor
Simple module based on open hardware devices: PZEM004T and ESP8266.
The device allows to monitor power lines up to 100A and support external databases: ThingSpeak and Influx.
Historical Voltage, current and active power samples provided by IOT module can be reused for creating graph, pie charts and statisstics. I recommend to use Grafana service which supports InfluxDB and allows for simple access to old measured datas.

# Requirements
1) Hardware:
- ESP8266 module, eg: ESP8266MOD
- Energy monitor: PZEM004T
- AC/DC Converter(AC: 110V-220V to DC 5V)
- Prototype PCB(4cm x 6cm)
- wires

Software:
- Arduino tool + ESP8266 toolchain
- libraries:
	- ArduinoJson (https://github.com/bblanchon/ArduinoJson)
	- PZEM004T(https://github.com/olehs/PZEM004T)
	- Arduino ESP8266 filesystem uploader(https://github.com/esp8266/arduino-esp8266fs-plugin)
	- ESP Influxdb (library supports latest InfluxDB is available in adds/ESP_influxdb-master.zip)

# Description
1) Istall latest 'Arduino' tool(https://www.arduino.cc/en/Main/Software)
2) Install ESP8266 toolchain(https://github.com/esp8266/Arduino)
3) Install required libraries.

# Functionality
1) User button usage:
- no press after restart -> starting 'normal mode'
- short press(<5s) after restart -> starting 'setup mode'(AccessPoint mode is activated)
- long press (>5s) after restart -> restoring default data and starting 'setup mode'

# LED code
- 10 short blinks -> device is starting up after restart or power on
- 1 long blink every 1s - ESP8266 modules tries to connect to PZEM004T module(timeout: 60s)
- 1 long blink every 30s-> device mode is WIFI_STA. It means that wifi module tries to connect to defined wifi network
- 2 long blinks every 30s-> device mode is WIFI_AP_STA. It means that wifi module create own wifi network(SSID and password are defined) and tries to connect to defined wifi network
- 3 long blinks every 30s-> device mode is WIFI_AP. Module tries to create own wifi network
- 2 short blinks (after 1 or 2 long blinks) -> device is connected to external network(STA)

# IoT node configuration
Each wifi mode(WIFI_STA/WIFI_AP_STA/WIFI_AP) supports http server. It allows for easly reconfiguration via web page. Configuration page is available on:
- 192.168.4.1 for wifi network created by node(mode: WIFI_AP_STA, WIFI_AP)
- assigned IP address(mode: WIFI_STA, WIFI_AP_STA).

