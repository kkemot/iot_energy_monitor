#pragma once
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

void startHttpServer(void);
void handleHttpClients(void);
void changeHttpServerMode(WiFiMode_t wifi_mode);
void handleApConfigurator(void);
bool isConnectedSTA(void);
WiFiMode_t getWiFiMode(void);
void reconnectLastMode(void);
