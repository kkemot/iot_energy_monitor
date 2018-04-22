#include <ESP8266WiFi.h>
#include "configuration.h"
#include "settings.h"
#include "webServer.h"
#include "sensors.h"
#include <PZEM004T.h>
#include <ESPinfluxdb.h>

WiFiClient client;
extern struct Settings settings;

PZEM004T pzem(POWER_MONITOR_1_RX,POWER_MONITOR_1_TX);  // (RX,TX) connect to TX,RX of PZEM
IPAddress ip(192,168,1,1);
bool pzemrdy = false;

void heartBeatModulation(uint32_t time_counter);
void send_data_InfluxDB(float data_1, float data_2);
void send_data_ThingSpeak(float data_1, float data_2);

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
  pzem.setAddress(ip);

     while (!pzemrdy) {
      Serial.println("Connecting to PZEM...");
      pzemrdy = pzem.setAddress(ip);
      delay(1000);
   }
}

uint32_t counter = 0;
void loop() {

  delay(100);

  handleHttpClients();
  handleApConfigurator();
  heartBeatModulation(counter);

  if (counter%10  == 0) {
    Serial.print("counter = ");
    Serial.println(counter,DEC);

    float v = pzem.voltage(ip);
    //if (v < 0.0) v = 0.0;
    Serial.print(v);Serial.print("V; ");
delay(500);
    float i = pzem.current(ip);
    //if(i >= 0.0)
    { Serial.print(i);Serial.print("A; "); }
delay(500);
    float p = pzem.power(ip);
    //if(p >= 0.0)
    { Serial.print(p);Serial.print("W; "); }
    float e = pzem.energy(ip);
delay(500);
    //if(e >= 0.0)
    { Serial.print(e);Serial.print("Wh; "); }
    Serial.println();
  }

  if (counter == 0) {

  }
  else if(counter < (SENSOR_WARMING_TIME * 10)) {

  } else if (counter  == (SENSOR_WARMING_TIME * 10)) {
    //sent to ThinkSpeak
    //if (isConnectedSTA()) {
      //send_data_ThingSpeak(f1, f2, f3, f4);
    //}
  } else {
    if (settings.wifi_mode == 2) { //Sleep mode -> wifi mode: 2 - WIFI_STA_DEEP_SLEEP;
      uint32_t sleep_time = settings.sleep_time - SENSOR_WARMING_TIME;
      Serial.print("sleep = ");
      Serial.println(sleep_time,DEC);
      ESP.deepSleep(sleep_time * 1000000);
    } else {
      //Serial.println("wait...");
    }
  }

  counter++;
  if (counter > (settings.sleep_time * 10)) {
    counter = 0;
  }

}

void send_data_ThingSpeak(float data_1, float data_2, float data_3, float data_4) {
  char temp[20];

  if (client.connect(TS_SERVER_NAME,80)) {
    String API_KEY = settings.ts_api_key;
    String postStr = API_KEY;
    postStr +="&field1=";
    sprintf(temp,"%.1f", data_1);
    postStr += String(temp);
    postStr +="&field2=";
    sprintf(temp,"%.0f", data_2);
    postStr += String(temp);
    postStr +="&field3=";
    sprintf(temp,"%.1f", data_3);
    postStr += String(temp);
    postStr +="&field4=";
    sprintf(temp,"%.1f", data_4);
    postStr += String(temp);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+API_KEY+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println(postStr);
    Serial.println("Sent to Thingspeak.");
  }
  client.stop();
}

void send_data_InfluxDB(float data_1, float data_2) {
  char temp[20];
  // There is no server address
  if(strlen(settings.influxdb_server_address) == 0) {
    Serial.println("UNKNOW InfluxDB server - skipped data sending to database");
    return;
  }

  Influxdb influxdb(settings.influxdb_server_address, settings.influxdb_server_port);

  if (settings.influxdb_user != "" &&  settings.influxdb_pass != "") {
      if (influxdb.configure(settings.influxdb_db_name, settings.influxdb_user, settings.influxdb_pass)) {
        Serial.println("Opend database failed");
      }
  }
  else {
    if (influxdb.configure(settings.influxdb_db_name)!=DB_SUCCESS) {
      Serial.println("Opend database failed");
    }
  }

  // Create data object: series,tag=ta1,tag=tag2,tag=tag3 value=1.0, value=2.0
  dbMeasurement row(settings.influxdb_series_name);
  row.addTag("module", settings.influxdb_type_tag);       // Add type: electrometer, envirement sensor, watermeter
  row.addTag("location", settings.influxdb_location_tag); // Add location: wroclaw
  row.addTag("id", settings.influxdb_nodeid_tag); // Add id: module name -> "light" / "kitchen"

  sprintf(temp,"%.1f", data_1);
  row.addField("temperature", temp); // Add value field
  sprintf(temp,"%.0f", data_2);
  row.addField("humidity", temp); // Add random value

  Serial.println(influxdb.write(row) == DB_SUCCESS ? "Object write success"
                 : "Writing failed");

  //Empty field object.
  row.empty();
}

//changes LED state
void heartBeatModulation(uint32_t time_counter) {
  WiFiMode_t currentWiFiMode = getWiFiMode();
  int time_stamp = time_counter % 100;

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

