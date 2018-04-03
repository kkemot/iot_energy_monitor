#pragma once

#include <ArduinoJson.h>
#include <FS.h>

struct Settings {
    char *ssid_sta;
    char *pass_sta;
    char *ssid_ap;
    char *pass_ap;
    char *ts_api_key;
    int sleep_time;     // [s]
    int wifi_mode;      //0 - WIFI_AP_STA; 1 - WIFI_STA; 2 - WIFI_STA_DEEP_SLEEP; 3 - WIFI_OFF
};

bool load_settings(void);
bool save_settings(void);
void set_default_settings(void);
void print_settings(void);

void init_file_system(void);
