#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "VARS.h"
#include "AUTH.h"
#include <ctime>

int machine_state = 0; // 0 = off, 1 = on
String latest_on_time = "";
// will fall back to ctime difference if API fails
time_t latest_on_time_ctime = time(0);

// Function prototypes
int readFSR(int pin);                           // Running average function to reduce noise
void analyzeFSR(int fsrValue1, int fsrValue2);  // Analyze FSR value function
bool connectToWifi(char *ssid, char *password); // Connect to WiFi function
void machineOn();
void machineOff();
String getAPIRequest();
String extractTime(String payload);
String extractDate(String payload);
String getFormattedDateTime();
bool isMachineOn();
bool isMachineOff();
String getDeltaTime(String on, String off);

void setup()
{
    Serial.begin(115200);     // Start serial communication
    pinMode(FSR1_PIN, INPUT); // Set FSR1 as input
    pinMode(FSR2_PIN, INPUT); // Set FSR2 as input
    pinMode(HIGH1, OUTPUT);
    pinMode(HIGH2, OUTPUT);
    digitalWrite(HIGH1, HIGH);
    digitalWrite(HIGH2, HIGH);
    pinMode(LED_PIN, OUTPUT); // Set LED as output

    // Connect to WiFi
    if (connectToWifi(WIFI_SSID, WIFI_PASS) && DEBUG)
    {
        Serial.println("Connected to WiFi");
    }
    else
    {
        Serial.println("Failed to connect to WiFi");
    }
}

void loop()
{
    int fsrValue1 = readFSR(FSR1_PIN); // Read FSR1 value
    int fsrValue2 = readFSR(FSR2_PIN); // Read FSR2 value

    analyzeFSR(fsrValue1, fsrValue2); // Analyze FSR values
}

int readFSR(int pin)
{
    const int numSamples = 10;
    int sum = 0;
    for (int i = 0; i < numSamples; i++)
    { // running average
        sum += analogRead(pin);
        delay(READ_DELAY); // Small delay to reduce noise
    }
    return sum / numSamples;
}

void analyzeFSR(int fsrValue1, int fsrValue2)
{
    if (ANALOG_STATE == 1)
    { // if we are expecting 3.3V/4095 for pressed state
        if (fsrValue1 > THRESHOLD && isMachineOff())
        { // then we are looking for a value greater than the threshold for the machine to be on
            machineOn();
        }
        if (fsrValue2 > THRESHOLD && isMachineOn())
        {
            machineOff();
        }
    }
    else
    { // otherwise we are expecting GND/0 for pressed state
        if (fsrValue1 < THRESHOLD && isMachineOff())
        { // and we are looking for a value less than the threshold for the machine to be on
            machineOn();
        }
        if (fsrValue2 < THRESHOLD && isMachineOn())
        {
            machineOff();
        }
    }
}

// Connect to WiFi function
bool connectToWifi(char *ssid, char *password)
{
    Serial.println("Connecting to WiFi");
    WiFi.begin(ssid, password);                // Connect to WiFi
    unsigned long startAttemptTime = millis(); // Get current time
    while (WiFi.status() != WL_CONNECTED)
    { // Wait for connection
        if (millis() - startAttemptTime > WIFI_TIMEOUT)
        {                 // Check if timeout
            return false; // Return false if timeout
        }
        delay(100); // Small delay to avoid spam
    }
    return true; // Return true if connected
}

String getAPIRequest()
{
    if(DEBUG)
        Serial.println("Sending API request");
    HTTPClient http;                              // Declare HTTPClient object
    http.begin("http://worldtimeapi.org/api/ip"); // Connect to API
    int httpResponseCode = http.GET();            // Get response from API
    String payload = http.getString();            // Get payload
    http.end();                                   // Close connection
    return payload;                               // Return payload
}

String extractTime(String payload)
{
    int timeIndex = payload.indexOf("datetime") + 22;   // Find index of "datetime" in the payload and add 23 to get to the time
    return payload.substring(timeIndex, timeIndex + 8); // Return time
}

String extractDate(String payload)
{
    int dateIndex = payload.indexOf("datetime") + 11;    // Find index of "datetime" in the payload and add 12 to get to the date
    return payload.substring(dateIndex, dateIndex + 10); // Return date
}

String getFormattedDateTime()
{
    String payload = getAPIRequest();   // Get payload from API
    String time = extractTime(payload); // Extract time
    String date = extractDate(payload); // Extract date

    // if time or date are empty, retry API request up to API_TIMEOUT times
    int attempts = 0;
    while (time == "" || date == "")
    {
        if (attempts >= API_TIMEOUT)
        {
            return "TIMEOUT"; // Return empty string if API_TIMEOUT reached
        }
        delay(1000);                 // Delay for 1 second
        payload = getAPIRequest();   // Get payload from API
        time = extractTime(payload); // Extract time
        date = extractDate(payload); // Extract date
        attempts++;                  // Increment attempts
    }

    return " " + date + " " + time; // Return date and time
}

void machineOn()
{
    machine_state = 1;
    latest_on_time_ctime = time(0); // record ctime in case API fails here or later
    String on_time = getFormattedDateTime();
    latest_on_time = on_time;
    digitalWrite(LED_PIN, HIGH); // Turn on LED
    if(DEBUG)
        Serial.println("Machine ON" + on_time);
}

void machineOff()
{
    machine_state = 0;
    time_t off_time_ctime = time(0); // record ctime in case API fails here
    String off_time = getFormattedDateTime();
    digitalWrite(LED_PIN, LOW); // Turn off LED
    if (off_time == "TIMEOUT" || latest_on_time == "TIMEOUT")
    {
        // time_t delta_time = off_time_ctime - latest_on_time_ctime;
        // if(DEBUG)
            //Serial.println("Machine OFF" + ctime(&off_time_ctime) + " ; Uptime: " + ctime(&delta_time));
    }
    else
    {
        if(DEBUG)
            Serial.println("Machine OFF" + getFormattedDateTime() + " ; Uptime: " + getDeltaTime(latest_on_time, getFormattedDateTime()));
    }
}

bool isMachineOn()
{
    return machine_state == 1;
}

bool isMachineOff()
{
    return machine_state == 0;
}

// get the time difference between the latest on time and the off time, formatted as HH:MM:SS
String getDeltaTime(String on, String off)
{
    int on_hour = on.substring(12, 14).toInt();
    int on_minute = on.substring(15, 17).toInt();
    int on_second = on.substring(18, 20).toInt();

    int off_hour = off.substring(12, 14).toInt();
    int off_minute = off.substring(15, 17).toInt();
    int off_second = off.substring(18, 20).toInt();

    int delta_hour = off_hour - on_hour;
    int delta_minute = off_minute - on_minute;
    int delta_second = off_second - on_second;

    if (delta_second < 0)
    {
        delta_second += 60;
        delta_minute--;
    }
    if (delta_minute < 0)
    {
        delta_minute += 60;
        delta_hour--;
    }

    return String(delta_hour) + ":" + String(delta_minute) + ":" + String(delta_second);
}