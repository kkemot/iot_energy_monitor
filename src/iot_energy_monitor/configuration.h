#pragma once

//Default settings
#define SSID_AP "**********"
#define PASS_AP "**********"
#define SSID_STA "**********"
#define PASS_STA "**********"
#define TS_WRITE_KEY "NO_API_KEY"
#define SLEEP_TIME_DEFAULT 60
#define WIFIMODE_DEFAULT 0 //0 - WIFI_AP_STA; 1 - WIFI_STA; 2 - WIFI_STA_DEEP_SLEEP; 3 - WIFI_OFF
#define TS_SERVER_NAME "api.thingspeak.com"
#define WIFIAP_TIMEOUT (60*3)

#define BUTTON_PRESSED  0
#define BUTTON_RELEASED 1

//Reserved pins:
//0 - Arduino pin: D3 (Flash button/BootMode)
//2 - Arduino pin: D4 (Buildin LED/BootMode)
//15 - Arduino pin: D8(BootMode)
//Normal mode: HHL
//Boot from uart: HLL
//
//16 - Arduino pin: D0 (wakeup <-> reset)

//Ports
#define USER_BUTTON 12  //Arduino pin: D6
#define LED_PIN     2   //Arduino pin: D4
#define LED_ON      LOW
#define LED_OFF     HIGH

//Power monitor
#define POWER_MONITOR_1_TX 13
#define POWER_MONITOR_1_RX 14
