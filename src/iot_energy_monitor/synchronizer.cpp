#include <ESP8266WiFi.h>
#include <ESPinfluxdb.h>
#include "synchronizer.h"
#include "settings.h"
#include "configuration.h"


WiFiClient client;
extern struct Settings settings;

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
    sprintf(temp,"%.2f", data_1);
    postStr += String(temp);
    postStr +="&field2=";
    sprintf(temp,"%.2f", data_2);
    postStr += String(temp);
    postStr +="&field3=";
    sprintf(temp,"%.2f", data_3);
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

  // There is no server address
  if(strlen(settings.influxdb_server_address) == 0) {
    Serial.println("UNKNOW InfluxDB server - skipped data sending to database");
    return;
  }

  Influxdb influxdb(settings.influxdb_server_address, settings.influxdb_server_port);

  if ((strlen(settings.influxdb_user) == 0) &&  (strlen(settings.influxdb_pass) == 0)) {
      if (influxdb.configure(settings.influxdb_db_name) != DB_SUCCESS) {
        Serial.println("Opend database failed(wrong: db)");
        return;
      }
  }
  else {
      if (influxdb.configure(settings.influxdb_db_name, settings.influxdb_user, settings.influxdb_pass) != DB_SUCCESS ) {
        Serial.println("Opend database failed(wrong: db, user or password)");
        return;
      }
  }

  // Create data object: series,tag=ta1,tag=tag2,tag=tag3 value=1.0, value=2.0
  dbMeasurement row(settings.influxdb_series_name);
  row.addTag("module", settings.influxdb_type_tag);       // Add type: electrometer, envirement sensor, watermeter
  row.addTag("location", settings.influxdb_location_tag); // Add location: wroclaw
  row.addTag("id", settings.influxdb_nodeid_tag); // Add id: module name -> "light" / "kitchen"

  if (data_1 >= 0.0) {
    sprintf(temp,"%.1f", data_1);
    row.addField("V", temp); // Add value field
  }

  if (data_2 >= 0.0) {
    sprintf(temp,"%.1f", data_2);
    row.addField("I", temp); // Add value field
  }

  if (data_3 >= 0.0) {
    sprintf(temp,"%.1f", data_3);
    row.addField("P", temp); // Add value field
  }

  Serial.println(influxdb.write(row) == DB_SUCCESS ? "Object write success"
                 : "Writing failed");
  Serial.print("InfluxDB row=");
  Serial.println(row.postString());
  //Empty field object.
  row.empty();
}

