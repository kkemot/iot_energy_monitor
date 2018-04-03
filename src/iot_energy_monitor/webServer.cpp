#include "webServer.h"
#include "configuration.h"
#include "settings.h"

ESP8266WebServer server(80);
extern struct Settings settings;

static WiFiMode_t wifi_mode_last = WIFI_OFF;
static int AP_configurator_timeout = 0;       // switch to stored mode when timer has expired

void handle_root(void)
{
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(200, "text/plain", "No index.html!");
    return;
  }

  String s = file.readString();
  s.replace("{wifi_staip}", WiFi.localIP().toString());
  s.replace("{ssid_sta}", settings.ssid_sta);
  s.replace("{pass_sta}", "<secret>");
  s.replace("{wifi_apip}", WiFi.softAPIP().toString().c_str());
  s.replace("{ssid_ap}", settings.ssid_ap);
  s.replace("{pass_ap}", settings.pass_ap);
  s.replace("{ts_api_key}", settings.ts_api_key);
  s.replace("{sleep_time}", String(settings.sleep_time));

  if (settings.wifi_mode == 0) {
    s.replace("{wifi_mode0}", "checked");
    s.replace("{wifi_mode1}", "");
    s.replace("{wifi_mode2}", "");
    s.replace("{wifi_mode3}", "");
  } else if (settings.wifi_mode == 1) {
    s.replace("{wifi_mode0}", "");
    s.replace("{wifi_mode1}", "checked");
    s.replace("{wifi_mode2}", "");
    s.replace("{wifi_mode3}", "");
  } else if (settings.wifi_mode == 2) {
    s.replace("{wifi_mode0}", "");
    s.replace("{wifi_mode1}", "");
    s.replace("{wifi_mode2}", "checked");
    s.replace("{wifi_mode3}", "");
  } else if (settings.wifi_mode == 3) {
    s.replace("{wifi_mode0}", "");
    s.replace("{wifi_mode1}", "");
    s.replace("{wifi_mode2}", "");
    s.replace("{wifi_mode3}", "checked");
  }

  server.send(200, "text/html", s.c_str());
}

static void handle_restart(void)
{
  if (server.method() != HTTP_POST) {
    server.send(500, "text/plain", "Only POST supported");
    return;
  }

  ESP.restart();
}

static void handle_save(void)
{
  if (server.method() != HTTP_POST) {
    server.send(500, "text/plain", "Only POST supported");
    return;
  }

  if (!server.hasArg("ssid_sta") || !server.hasArg("pass_sta") ||
    !server.hasArg("ssid_ap") || !server.hasArg("pass_ap") ||
    !server.hasArg("ts_api_key") || !server.hasArg("sleep_time") ||
    !server.hasArg("wifi_mode")) {
    server.send(500, "text/plain", "Bad form");
  }

  String val = server.arg("ssid_sta");
  settings.ssid_sta = strdup(val.c_str());
  val = server.arg("pass_sta");
  if (val != "<secret>") {
    settings.pass_sta = strdup(val.c_str());
  }

  val = server.arg("ssid_ap");
  settings.ssid_ap = strdup(val.c_str());
  val = server.arg("pass_ap");
  settings.pass_ap = strdup(val.c_str());

  val = server.arg("ts_api_key");
  settings.ts_api_key = strdup(val.c_str());

  val = server.arg("sleep_time");
  if (atoi(val.c_str()) > 19) {
    settings.sleep_time = atoi(val.c_str());
  }

  val = server.arg("wifi_mode");
  if (atoi(val.c_str()) >= 0 && atoi(val.c_str()) <= 3) {
    settings.wifi_mode = atoi(val.c_str());
  }

  save_settings();
  server.sendHeader("Location", "/");
  server.send(301, "text/html", "Moved Permanently");

  if (wifi_mode_last == WIFI_AP) {
    AP_configurator_timeout = WIFIAP_TIMEOUT * 10;
  }
}

void handle_notFound(){
  server.send(404, "text/plain", "404: Not found");
}

void startHttpServer(void) {
  if (settings.wifi_mode == 0) {
    changeHttpServerMode(WIFI_AP_STA);
  }
  else if (settings.wifi_mode == 1) {
    changeHttpServerMode(WIFI_STA);
  }
  else if (settings.wifi_mode == 2) {
    changeHttpServerMode(WIFI_STA);
  }
  else if (settings.wifi_mode == 3) {
    changeHttpServerMode(WIFI_OFF);
  }
  else {
    changeHttpServerMode(WIFI_OFF);
  }
}

void changeHttpServerMode(WiFiMode_t wifi_mode) {
  WiFi.disconnect();
  Serial.println("WiFi disconnect.");
  delay(1000);
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi Off.");
  delay(1000);

  wifi_mode_last = wifi_mode;

  Serial.println("");
  Serial.println("Change network mode...");
  if (wifi_mode == WIFI_AP_STA) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(settings.ssid_ap, settings.pass_ap);
    delay(1000);
    Serial.println("WIFI mode: WIFI_AP_STA");
    Serial.print("AccessPoint IP:");
    Serial.println(WiFi.softAPIP().toString().c_str());

    Serial.println("begin connect to STA");
    WiFi.begin(settings.ssid_sta, settings.pass_sta);

    for(uint8_t i = 0; i < 30; i++) {
      if (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    }
    Serial.print("Local IP:");
    Serial.println(WiFi.localIP().toString().c_str());
  }
  else if (wifi_mode == WIFI_STA) {
    Serial.println("WIFI mode: WIFI_STA");
    WiFi.mode(WIFI_STA);

    Serial.println("begin connect to STA");
    WiFi.begin(settings.ssid_sta, settings.pass_sta);

    for(uint8_t i = 0; i < 30; i++) {
      if (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    }
    Serial.print("Local IP:");
    Serial.println(WiFi.localIP().toString().c_str());
  }
  else if (wifi_mode == WIFI_AP) {
    AP_configurator_timeout = WIFIAP_TIMEOUT * 10;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(settings.ssid_ap, settings.pass_ap);
    delay(1000);
    Serial.println("WIFI mode: WIFI_AP");
    Serial.print("AccessPoint IP:");
    Serial.println(WiFi.softAPIP().toString().c_str());
  }
  else if (wifi_mode == WIFI_OFF) {
    //Already disconnected
    Serial.println("WiFi OFF");
    return;
  }
  else {
    Serial.println("Error: Incorrent WIFI mode.");
    return;
  }

  server.on("/", handle_root);
  server.on("/save", handle_save);
  server.on("/restart", handle_restart);
  server.onNotFound(handle_notFound);
  server.begin();
}

void handleHttpClients(void) {
  server.handleClient();
}

void handleApConfigurator() {
  if (AP_configurator_timeout != 0) {
    AP_configurator_timeout --;
    if ((wifi_mode_last != WIFI_AP) && (wifi_mode_last != WIFI_AP_STA)) {
      changeHttpServerMode(WIFI_AP);
      Serial.println("zmiana na WIFI AP");
    }
  }
  else if (AP_configurator_timeout == 0 && wifi_mode_last == WIFI_AP) {
    Serial.println("Restart module");
    ESP.restart();
  }
}

bool isConnectedSTA(void) {
   if (WiFi.status() != WL_CONNECTED) {
     return false;
   }
   else {
    return true;
   }
}

WiFiMode_t getWiFiMode(void) {
  return wifi_mode_last;
}

void reconnectLastMode(void) {
  if (WiFi.status() != WL_CONNECTED) { // FIX FOR USING 2.3.0 CORE (only .begin if not connected)

    if (wifi_mode_last == WIFI_STA || wifi_mode_last == WIFI_AP_STA) {
      Serial.println("Re-connect to STA");
      WiFi.begin(settings.ssid_sta, settings.pass_sta);
    }
  }
}

