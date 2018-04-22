#include "settings.h"
#include "configuration.h"

struct Settings settings;

void init_file_system(void) {
   SPIFFS.begin();
}

void set_default_settings(void)
{
  settings.ssid_sta =  strdup(SSID_STA);
  settings.pass_sta = strdup(PASS_STA);
  settings.ssid_ap = strdup(SSID_AP);
  settings.pass_ap = strdup(PASS_AP);
  settings.wifi_mode = WIFIMODE_DEFAULT;

  settings.sleep_time = SLEEP_TIME_DEFAULT;

  //ThingSpeak:
  settings.ts_api_key = strdup(TS_WRITE_KEY);

  //InfuxDB:
  settings.influxdb_server_address = strdup(INFLUXDB_SERVER_ADDRESS);
  settings.influxdb_server_port = INFLUXDB_SERVER_PORT;
  settings.influxdb_user = strdup(INFLUXDB_USER);
  settings.influxdb_pass = strdup(INFLUXDB_PASS);
  settings.influxdb_db_name = strdup(INFLUXDB_DB_NAME);
  settings.influxdb_series_name = strdup(INFLUXDB_SERIES_NAME);
  settings.influxdb_location_tag = strdup(INFLUXDB_LOCATION_TAG);
  settings.influxdb_type_tag = strdup(INFLUXDB_TYPE_TAG);
  settings.influxdb_nodeid_tag = strdup(INFLUXDB_NODEID_TAG);
}

void print_settings(void)
{
  Serial.println();
  Serial.println("Settings:");

  Serial.print("ssid_sta=");
  Serial.println(settings.ssid_sta);

  Serial.println("pass_sta=<secret>");

  Serial.print("ssid_ap=");
  Serial.println(settings.ssid_ap);

  Serial.print("pass_ap=");
  Serial.println(settings.pass_ap);

  Serial.print("wifi_mode=");
  Serial.println(settings.wifi_mode,DEC);

  Serial.print("sleep_time=");
  Serial.println(settings.sleep_time,DEC);

  Serial.print("ts_api_key=");
  Serial.println(settings.ts_api_key);

  Serial.print("influxdb_server_address=");
  Serial.println(settings.influxdb_server_address);

  Serial.print("influxdb_server_port=");
  Serial.println(settings.influxdb_server_port, DEC);

  Serial.print("influxdb_user=");
  Serial.println(settings.influxdb_user);

  Serial.print("influxdb_pass=");
  Serial.println(settings.influxdb_pass);

  Serial.print("influxdb_db_name=");
  Serial.println(settings.influxdb_db_name); 

  Serial.print("influxdb_series_name=");
  Serial.println(settings.influxdb_series_name);

  Serial.print("influxdb_location_tag=");
  Serial.println(settings.influxdb_location_tag);

  Serial.print("influxdb_type_tag=");
  Serial.println(settings.influxdb_type_tag);

  Serial.print("influxdb_nodeid_tag=");
  Serial.println(settings.influxdb_nodeid_tag);
  Serial.println();
}

bool save_settings(void)
{
    bool ret = false;

    StaticJsonBuffer<400> json_buf;
    JsonObject &json = json_buf.createObject();
    json["ssid_sta"] = settings.ssid_sta;
    json["pass_sta"] = settings.pass_sta;
    json["ssid_ap"] = settings.ssid_ap;
    json["pass_ap"] = settings.pass_ap;
    json["ts_api_key"] = settings.ts_api_key;
    json["sleep_time"] = settings.sleep_time;
    json["wifi_mode"] = settings.wifi_mode;
    json["influxdb_server_address"] = settings.influxdb_server_address;
    json["influxdb_server_port"] = settings.influxdb_server_port;
    json["influxdb_user"] = settings.influxdb_user;
    json["influxdb_pass"] = settings.influxdb_pass;
    json["influxdb_db_name"] = settings.influxdb_db_name;
    json["influxdb_series_name"] = settings.influxdb_series_name;
    json["influxdb_location_tag"] = settings.influxdb_location_tag;
    json["influxdb_type_tag"] = settings.influxdb_type_tag;
    json["influxdb_nodeid_tag"] = settings.influxdb_nodeid_tag;

    File file = SPIFFS.open("/settings.json", "w");
    if (!file) {
        goto out;
    }

    json.printTo(file);
out:
    return ret;
}

bool load_settings(void)
{
  char buf[2048];
  const char *tmp;
  StaticJsonBuffer<400> json_buf;
  bool ret = false;
  size_t size;

  memset(&settings, 0, sizeof(settings));

  File file = SPIFFS.open("/settings.json", "r");
  if (!file) {
    return ret;
  }

  size = file.size();
  if (size > sizeof(buf)) {
    goto out;
  }

  file.readBytes(buf, size);

  {
    JsonObject &json = json_buf.parseObject(buf);

    if (!json.success()) {
      goto out;
    }

    tmp = json["ssid_sta"];
    if (tmp) {
      settings.ssid_sta = strdup(tmp);
    }

    tmp = json["pass_sta"];
    if (tmp) {
      settings.pass_sta = strdup(tmp);
    }

    tmp = json["ssid_ap"];
    if (tmp) {
      settings.ssid_ap = strdup(tmp);
    }

    tmp = json["pass_ap"];
    if (tmp) {
      settings.pass_ap = strdup(tmp);
    }

    tmp = json["ts_api_key"];
    if (tmp) {
      settings.ts_api_key = strdup(tmp);
    }

    tmp = json["sleep_time"];
    if (tmp) {
      settings.sleep_time = atoi(tmp);
    }
    if (settings.sleep_time < 15) {
      settings.sleep_time = SLEEP_TIME_DEFAULT;
    }

    tmp = json["wifi_mode"];
    if (tmp) {
      settings.wifi_mode = atoi(tmp);
    }
    if (settings.wifi_mode < 0 || settings.wifi_mode > 3) {
      settings.wifi_mode = WIFIMODE_DEFAULT;
    }

    tmp = json["influxdb_server_address"];
    if (tmp) {
      settings.influxdb_server_address = strdup(tmp);
    }

    tmp = json["influxdb_server_port"];
    if (tmp) {
      settings.influxdb_server_port = atoi(tmp);
    }

    tmp = json["influxdb_user"];
    if (tmp) {
      settings.influxdb_user = strdup(tmp);
    }

    tmp = json["influxdb_pass"];
    if (tmp) {
      settings.influxdb_pass = strdup(tmp);
    }

    tmp = json["influxdb_db_name"];
    if (tmp) {
      settings.influxdb_db_name = strdup(tmp);
    }

    tmp = json["influxdb_series_name"];
    if (tmp) {
      settings.influxdb_series_name = strdup(tmp);
    }

    tmp = json["influxdb_location_tag"];
    if (tmp) {
      settings.influxdb_location_tag = strdup(tmp);
    }

    tmp = json["influxdb_type_tag"];
    if (tmp) {
      settings.influxdb_type_tag = strdup(tmp);
    }

    tmp = json["influxdb_nodeid_tag"];
    if (tmp) {
      settings.influxdb_nodeid_tag = strdup(tmp);
    }
  }
  ret = true;
out:
  file.close();
  return ret;
}
