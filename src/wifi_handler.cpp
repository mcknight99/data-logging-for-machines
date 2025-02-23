#include "wifi_handler.h"

// Function to connect to WiFi
// Returns true if connected, false if not connected
// Will wait for connection for WIFI_TIMEOUT milliseconds
// Will return false if connection attempt times out
bool connectToWiFi()
{
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED)
    { // Wait for connection
        if (millis() - startAttemptTime > WIFI_TIMEOUT)
        {
            if (DEBUG)
                Serial.println("\nWiFi connection attempt timed out");
            return false;
        }
        delay(1000); // Small delay to avoid spam
        if(DEBUG) Serial.print(".");
    }
    if (DEBUG)
        Serial.println("\nConnected to WiFi");

    return true;
}

void disconnectFromWiFi()
{
    WiFi.disconnect();
    if (DEBUG)
    {
        Serial.println("Disconnected from WiFi");
    }
}
