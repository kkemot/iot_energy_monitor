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
  settings.ts_api_key = strdup(TS_WRITE_KEY);
  settings.sleep_time = SLEEP_TIME_DEFAULT;
  settings.wifi_mode = WIFIMODE_DEFAULT;
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

  Serial.print("ts_api_key=");
  Serial.println(settings.ts_api_key);

  Serial.print("sleep_time=");
  Serial.println(settings.sleep_time,DEC);

  Serial.print("wifi_mode=");
  Serial.println(settings.wifi_mode,DEC);
}

bool save_settings(void)
{
    bool ret = false;

    StaticJsonBuffer<200> json_buf;
    JsonObject &json = json_buf.createObject();
    json["ssid_sta"] = settings.ssid_sta;
    json["pass_sta"] = settings.pass_sta;
    json["ssid_ap"] = settings.ssid_ap;
    json["pass_ap"] = settings.pass_ap;
    json["ts_api_key"] = settings.ts_api_key;
    json["sleep_time"] = settings.sleep_time;
    json["wifi_mode"] = settings.wifi_mode;

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
  char buf[1024];
  const char *tmp;
  StaticJsonBuffer<200> json_buf;
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
    if (settings.sleep_time < 60) {
      settings.sleep_time = SLEEP_TIME_DEFAULT;
    }

    tmp = json["wifi_mode"];
    if (tmp) {
      settings.wifi_mode = atoi(tmp);
    }
    if (settings.wifi_mode < 0 || settings.wifi_mode > 3) {
      settings.wifi_mode = WIFIMODE_DEFAULT;
    }
  }
  ret = true;
out:
  file.close();
  return ret;
}
