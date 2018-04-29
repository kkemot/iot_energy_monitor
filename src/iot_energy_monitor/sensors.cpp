#include <ESP8266WiFi.h>
#include "sensors.h"
#include "configuration.h"
#include "settings.h"
#include "filters.h"
#include <PZEM004T.h>

extern struct Settings settings;

PZEM004T pzem(POWER_MONITOR_1_RX,POWER_MONITOR_1_TX);  // (RX,TX) connect to TX,RX of PZEM
IPAddress ip(192,168,1,1);
bool pzemrdy = false;

//energy meter
int filter_size = 0;
simpleFilter filter_voltage;
simpleFilter filter_current;
simpleFilter filter_power;


void init_sensors(void) {
 int led_state = 0;
 pzem.setReadTimeout(1000);

  //pzem.setAddress(ip);
  //or:
  while (!pzemrdy) {
    Serial.println("Connecting to PZEM...");
    pzemrdy = pzem.setAddress(ip);
    delay(1000);

    //blinks LED
    if (led_state == 0) {
      digitalWrite(LED_PIN, LED_ON);
      led_state = 1;
    }
    else {
      digitalWrite(LED_PIN, LED_OFF);
      led_state = 0;
    }
  }
  digitalWrite(LED_PIN, LED_OFF);

  filter_size = (settings.sleep_time * SAMPLING_RATE) / 100;
  Serial.print("Filter size is:");
  Serial.println(filter_size);

  filter_voltage.setFilterSize(filter_size);
  filter_current.setFilterSize(filter_size);
  filter_power.setFilterSize(filter_size);
  energyMeter_clearBuffers();
}

void energyMeter_clearBuffers(void) {
  Serial.println("Clear energy buffers");
  filter_voltage.clear();
  filter_current.clear();
  filter_power.clear();
}

void energyMeter_read(void) {
    Serial.println("Read V,I,P");
    float v, i, p;
    v = pzem.voltage(ip);
    if (v >= 0.0) {
      filter_voltage.add(v);
    }

    i = pzem.current(ip);
    if (i >= 0.0) {
      filter_current.add(i);
    }

    p = pzem.power(ip);
    if (p >= 0.0) {
      filter_power.add(p);
    }

    Serial.print("V= ");
    Serial.print(v);
    Serial.print(", I= ");
    Serial.print(i);
    Serial.print(", P= ");
    Serial.println(p);
}

