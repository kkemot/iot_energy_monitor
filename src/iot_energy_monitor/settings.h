#pragma once

#include <ArduinoJson.h>
#include <FS.h>

struct Settings {
    // Device notwork configuration
    char *ssid_sta;
    char *pass_sta;
    char *ssid_ap;
    char *pass_ap;
    int wifi_mode;      //0 - WIFI_AP_STA; 1 - WIFI_STA; 2 - WIFI_STA_DEEP_SLEEP; 3 - WIFI_OFF

    //Data sinks:
    int sleep_time;     // [s]
    //ThingSpeak:
    char *ts_api_key;
    //InfuxDB:
    char *influxdb_server_address;
    uint16_t influxdb_server_port;
    char *influxdb_user;
    char *influxdb_pass;
    char *influxdb_db_name;
    char *influxdb_series_name;
    char *influxdb_location_tag;
    char *influxdb_type_tag;
    char *influxdb_nodeid_tag;

};

bool load_settings(void);
bool save_settings(void);
void set_default_settings(void);
void print_settings(void);

void init_file_system(void);
