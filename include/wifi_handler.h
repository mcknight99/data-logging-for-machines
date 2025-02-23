#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <WiFi.h>
#include "VARS.h"
#include "AUTH.h"

bool connectToWiFi();      // Connect to WiFi using hidden SSID and password
void disconnectFromWiFi(); // Disconnect from WiFi

#endif
