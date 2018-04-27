#include <ESP8266WiFi.h>
#include "configuration.h"
#include "settings.h"
#include "webServer.h"
#include "sensors.h"
#include "filters.h"
#include <PZEM004T.h>
#include <ESPinfluxdb.h>

WiFiClient client;
extern struct Settings settings;

PZEM004T pzem(POWER_MONITOR_1_RX,POWER_MONITOR_1_TX);  // (RX,TX) connect to TX,RX of PZEM
IPAddress ip(192,168,1,1);
bool pzemrdy = false;

//energy meter
int filter_size = 0;
simpleFilter filter_voltage;
simpleFilter filter_current;
simpleFilter filter_power;

void heartBeatModulation(uint32_t time_counter);
void send_data_InfluxDB(float data_1, float data_2, float data_3);
void send_data_ThingSpeak(float data_1, float data_2, float data_3);

void energyMeter_read(void);
void energyMeter_clearBuffers(void);

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
  pzem.setReadTimeout(1000);

//     while (!pzemrdy) {
//      Serial.println("Connecting to PZEM...");
//      pzemrdy = pzem.setAddress(ip);
//      delay(1000);
//   }

  filter_size = (settings.sleep_time * SAMPLING_RATE) / 100;
  Serial.print("Filter size is:");
  Serial.println(filter_size);

  filter_voltage.setFilterSize(filter_size);
  filter_current.setFilterSize(filter_size);
  filter_power.setFilterSize(filter_size);
  energyMeter_clearBuffers();
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

    Serial.println("Send data to databases");
    if (isConnectedSTA()) {

      float v, i, p;

      Serial.print("filter_voltage size = ");
      Serial.println(filter_voltage.getNumSamples(), DEC);
      Serial.print("filter_current size = ");
      Serial.println(filter_current.getNumSamples(), DEC);
      Serial.print("filter_power size = ");
      Serial.println(filter_power.getNumSamples(), DEC);

      v = filter_voltage.get();
      i = filter_current.get();
      p = filter_power.get();

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
      int _start_time = (counter_seconds) % 30;
      int _end_time = (counter_seconds+3) % 30;

      //slot time for blinking code
      if ((_start_time >= 4) && (_end_time >= 4)) {
        Serial.println("out of blink time slot");
        energyMeter_read();
        counter_upload_delay -= 3;
        counter_seconds += 3;
        counter += 30;
      }
      else {
        Serial.println("in blink time slot");
      }

    Serial.print("counter=");
    Serial.print(counter, DEC);
    Serial.print(", count_sec=");
    Serial.print(counter_seconds, DEC);
    Serial.print(", upload_delayc=");
    Serial.println(counter_upload_delay, DEC);
  }
}

void send_data_ThingSpeak(float data_1, float data_2, float data_3) {
  char temp[20];

  // There is no API KEY
  if(strlen(settings.ts_api_key) == 0) {
    Serial.println("NO API KEY - skipped data sending to ThingSpeak");
    return;
  }

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

void send_data_InfluxDB(float data_1, float data_2, float data_3) {
  char temp[20];

  dbMeasurement rowT(settings.influxdb_series_name);
  sprintf(temp,"%.1f", data_1);
  rowT.addField("V", temp); // Add value field
  sprintf(temp,"%.1f", data_2);
  rowT.addField("I", temp); // Add value field
  sprintf(temp,"%.1f", data_3);
  rowT.addField("P", temp); // Add value field

  Serial.print("InfluxDB row=");
  Serial.println(rowT.postString());


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
  row.addField("V", temp); // Add value field
  sprintf(temp,"%.1f", data_2);
  row.addField("I", temp); // Add value field
  sprintf(temp,"%.1f", data_3);
  row.addField("P", temp); // Add value field

  Serial.println(influxdb.write(row) == DB_SUCCESS ? "Object write success"
                 : "Writing failed");
  Serial.print("InfluxDB row=");
  Serial.println(row.postString());
  //Empty field object.
  row.empty();
}

//changes LED state
void heartBeatModulation(uint32_t time_counter) {
  WiFiMode_t currentWiFiMode = getWiFiMode();
  int time_stamp = time_counter % 300;

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




