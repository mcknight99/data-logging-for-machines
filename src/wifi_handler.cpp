#include "wifi_handler.h"

// Function to connect to WiFi
// Returns true if connected, false if not connected
// Will wait for connection for WIFI_TIMEOUT milliseconds
// Will return false if connection attempt times out
bool connectToWiFi()
{
    if (DEBUG)
        Serial.print("Connecting to WiFi");
    bool ledState = digitalRead(LED_PIN);
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long startAttemptTime = millis();
    // state of LED before blinking
    while (WiFi.status() != WL_CONNECTED)
    { // Wait for connection
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        if (millis() - startAttemptTime > WIFI_TIMEOUT)
        {
            if (DEBUG)
                Serial.println("\nWiFi connection attempt timed out");
            // rewrite the LED state to what it was before
            digitalWrite(LED_PIN, ledState);
            return false;
        }
        delay(1000); // Small delay to avoid spam
        if (DEBUG)
            Serial.print(".");
    }
    if (DEBUG)
        Serial.println("\nConnected to WiFi");

    // rewrite the LED state to what it was before
    digitalWrite(LED_PIN, ledState);
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
