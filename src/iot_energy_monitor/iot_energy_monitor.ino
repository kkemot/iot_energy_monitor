#include <ESP8266WiFi.h>
#include "configuration.h"
#include "settings.h"
#include "webServer.h"
#include "sensors.h"
#include "filters.h"
#include "synchronizer.h"

extern struct Settings settings;

extern simpleFilter filter_voltage;
extern simpleFilter filter_current;
extern simpleFilter filter_power;

void heartBeatModulation(uint32_t time_counter);

void setup() {
  int button_cnt = 0;

  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();

  //hardware_configuration
  pinMode(LED_PIN, OUTPUT);
  pinMode(USER_BUTTON, INPUT_PULLUP);

  init_file_system();

  if (load_settings()) {
    Serial.println("Configuration loaded.");
  }
  else {
    Serial.println("Resore default configuration.");
    set_default_settings();
    save_settings();
  }
  print_settings();

  while (digitalRead(USER_BUTTON) == BUTTON_PRESSED) {
    digitalWrite(LED_PIN, LED_ON);
    button_cnt ++;
    delay(1000);
    if (button_cnt > 5)
      break;
  }
  //Blinking LED
  for (int i = 0; i < 10; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(50);
      digitalWrite(LED_PIN, LED_OFF);
      delay(50);
    }

  digitalWrite(LED_PIN, LED_OFF);

  //start normal mode
  if (button_cnt == 0) {
    Serial.println("Start normal mode");
    startHttpServer();
  }
  //restore default settings and start setup mode
  else if ( button_cnt > 5) {
    Serial.println("Set default settings and start setup mode");
    set_default_settings();
    save_settings();
    startHttpServer();
  }
  //start setup mode only
  else {
    Serial.println("Start setup mode only");
    changeHttpServerMode(WIFI_AP);
  }

  init_sensors();
}

void loop() {
  static uint32_t counter = 0;
  static uint32_t counter_seconds = 0;
  static int counter_upload_delay = settings.sleep_time;

  delay(100);

  handleHttpClients();
  handleApConfigurator();
  heartBeatModulation(counter);

  if (counter_upload_delay <= 0) {
    counter_upload_delay = settings.sleep_time;

    if (isConnectedSTA()) {
      float v, i, p;
      v = filter_voltage.get();
      i = filter_current.get();
      p = filter_power.get();

      Serial.println("Send data to databases");
      send_data_ThingSpeak(v, i, p);
      send_data_InfluxDB(v, i, p);
    }
    energyMeter_clearBuffers();
  }

  counter++;
  if (counter%10  == 0) {
      counter_seconds += 1;
      counter_upload_delay -= 1;

      //calculate measure time slot
      int _start_time = (counter_seconds) % 60;
      int _end_time = (counter_seconds+3) % 60;

      //slot time for blinking code
      if ((_start_time >= 4) && (_end_time >= 4)) {
        // reading one value takes one second
        int value_counter = energyMeter_read();
        counter_upload_delay -= value_counter;
        counter_seconds += value_counter;
        counter += (value_counter * 10);
      }
      else {
        // no action here
      }
  }
}

//changes LED state
void heartBeatModulation(uint32_t time_counter) {
  WiFiMode_t currentWiFiMode = getWiFiMode();
  int time_stamp = time_counter % 600;

  if (currentWiFiMode == WIFI_AP_STA) {
    if (time_stamp == 0)
      digitalWrite(LED_PIN, LED_ON);
    else if (time_stamp == 10)
      digitalWrite(LED_PIN, LED_OFF);
    else if (time_stamp == 15)
      digitalWrite(LED_PIN, LED_ON);
    else if (time_stamp == 25)
      digitalWrite(LED_PIN, LED_OFF);

    if (isConnectedSTA()) {
      if (time_stamp == 30)
        digitalWrite(LED_PIN, LED_ON);
      else if (time_stamp == 32)
        digitalWrite(LED_PIN, LED_OFF);
      else if (time_stamp == 34)
        digitalWrite(LED_PIN, LED_ON);
      else if (time_stamp == 36)
        digitalWrite(LED_PIN, LED_OFF);
    }
    else {
      if (time_stamp == 0)
        reconnectLastMode();
    }
  } else if (currentWiFiMode == WIFI_STA) {
     if (time_stamp == 0)
      digitalWrite(LED_PIN, LED_ON);
    else if (time_stamp == 10)
      digitalWrite(LED_PIN, LED_OFF);

    if (isConnectedSTA()) {
      if (time_stamp == 15)
        digitalWrite(LED_PIN, LED_ON);
      else if (time_stamp == 17)
        digitalWrite(LED_PIN, LED_OFF);
      else if (time_stamp == 19)
        digitalWrite(LED_PIN, LED_ON);
      else if (time_stamp == 21)
        digitalWrite(LED_PIN, LED_OFF);
    }
    else {
      if (time_stamp == 0)
        reconnectLastMode();
    }
  }  else if (currentWiFiMode == WIFI_AP) {
    if (time_stamp == 0)
      digitalWrite(LED_PIN, LED_ON);
    else if (time_stamp == 10)
      digitalWrite(LED_PIN, LED_OFF);
    else if (time_stamp == 15)
      digitalWrite(LED_PIN, LED_ON);
    else if (time_stamp == 25)
      digitalWrite(LED_PIN, LED_OFF);
   else if (time_stamp == 30)
      digitalWrite(LED_PIN, LED_ON);
    else if (time_stamp == 40)
      digitalWrite(LED_PIN, LED_OFF);
  }
}

