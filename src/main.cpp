#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ctime>
#include "VARS.h"
#include "AUTH.h"

// Global Variables
int machine_state; // 0 for off, 1 for on
String latest_on_time; // "YYYY-MM-DD HH:MM:SS"
time_t latest_on_time_ctime; // ctime of latest_on_time

// To Do (improve the agreed upon product requirements to guarantee robustness and a good quality product):
// make ctime the primary time source; use API during setup and use ctime relative to that time to reduce wifi uptime and api call reliance
// move all the code into included files instead of one large main
// fix negative delta time bug in getDeltaTime
// figure out why the mcu crashes after sending data to the web app (more specifically, after the call goes through and receives the response); also DNS failure??
// add a timeout and response verification to the sendToWebApp function
// add deep sleep and wakeup functionality to reduce power consumption

// Requirement Drift Ideas (will not implement due to not agreed upon product, but are ideas that I liked that I came up with):
// add a sheet to the google sheet that stores default vars to boot up with
    // a local var here that determines if it will use the google sheet vars or local vars on boot
    // or perhaps on each sheet, a locked three rows: 1=var labels, 2=var values, 3=column headers for data and on boot, it reads the second row of the respective sheet that the machine is on

// Function prototypes
int readFSR(int pin);                           // Running average function to reduce noise
void analyzeFSR(int fsrValue1, int fsrValue2);  // Analyze FSR value function
bool connectToWifi(char *ssid, char *password); // Connect to WiFi function
void machineOn();
void machineOff();
String sendDateTimeRequest();
String extractTime(String payload);
String extractDate(String payload);
String getFormattedDateTime();
bool isMachineOn();
bool isMachineOff();
String getDeltaTime(String on, String off);
bool sendToWebApp(String on_time, String off_time, String uptime);
bool connectToWiFi();
void disconnectFromWiFi();

void setup()
{
    delay(3000); // Delay for 3 seconds to allow for serial monitor to connect
    Serial.begin(115200);     // Start serial communication
    pinMode(FSR1_PIN, INPUT); // Set FSR1 as input
    pinMode(FSR2_PIN, INPUT); // Set FSR2 as input
    pinMode(HIGH1, OUTPUT);
    pinMode(HIGH2, OUTPUT);
    digitalWrite(HIGH1, HIGH);
    digitalWrite(HIGH2, HIGH);
    pinMode(LED_PIN, OUTPUT); // Set LED as output

    if (DEBUG)
    {
        Serial.println("Machine name: " + String(MACHINE_NAME));
        Serial.println("FSR1 pin: " + String(FSR1_PIN));
        Serial.println("FSR2 pin: " + String(FSR2_PIN));
        Serial.println("LED pin: " + String(LED_PIN));
        Serial.println("High1 pin: " + String(HIGH1));
        Serial.println("High2 pin: " + String(HIGH2));
        Serial.println("Machine is ready");
    }

    machine_state = 0; // Set machine state to off
    latest_on_time = ""; // Set latest on time to empty string
    latest_on_time_ctime = time(0); // Set latest on time ctime to current time
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
    if (DEBUG && false)
    {
        Serial.print("FSR1: " + String(fsrValue1));
        Serial.println("\t|\tFSR2: " + String(fsrValue2));
    }
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

// Prerequirement: connected to WiFi
String sendDateTimeRequest()
{
    auto ipv4 = WiFi.localIP();
    if (DEBUG)
    {
        Serial.print("Sending API request for timezone of the following IP address: ");
        Serial.println(String(ipv4[0]) + "." + String(ipv4[1]) + "." + String(ipv4[2]) + "." + String(ipv4[3]));
    }
    HTTPClient http;                              // Declare HTTPClient object
    String httpAddress = "https://timeapi.io/api/time/current/ip?ipAddress=" + String(ipv4[0]) + "." + String(ipv4[1]) + "." + String(ipv4[2]) + "." + String(ipv4[3]);
    http.begin(httpAddress); // Connect to API
    int httpResponseCode = http.GET();            // Get response from API
    String payload = http.getString();            // Get payload
    http.end();                                   // Close connection
    WiFi.disconnect();                            // Disconnect from WiFi
    if(DEBUG)
        Serial.println("API response: " + payload);
    return payload;                               // Return payload
}

String extractTime(String payload)
{
    int timeIndex = payload.indexOf("dateTime") + 22;   // Find index of "datetime" in the payload and add 23 to get to the time
    return payload.substring(timeIndex, timeIndex + 8); // Return time
}

String extractDate(String payload)
{
    int dateIndex = payload.indexOf("dateTime") + 11;    // Find index of "datetime" in the payload and add 12 to get to the date
    return payload.substring(dateIndex, dateIndex + 10); // Return date
}

String getFormattedDateTime()
{
    String payload = sendDateTimeRequest(); // Get payload from API
    String time = extractTime(payload);     // Extract time
    String date = extractDate(payload);     // Extract date

    // if time or date are empty, retry API request up to API_TIMEOUT times
    int attempts = 0;
    while (time == "" || date == "")
    {
        if (attempts >= API_TIMEOUT)
        {
            return "TIMEOUT"; // Return empty string if API_TIMEOUT reached
        }
        delay(1000);                     // Delay for 1 second
        payload = sendDateTimeRequest(); // Get payload from API
        time = extractTime(payload);     // Extract time
        date = extractDate(payload);     // Extract date
        attempts++;                      // Increment attempts
    }
    return date + " " + time; // Return date and time
}

void machineOn()
{
    if(DEBUG)
        Serial.println("Detected Machine ON");
    machine_state = 1;
    digitalWrite(LED_PIN, HIGH);    // Turn on LED
    latest_on_time_ctime = time(0); // record ctime in case API fails here or later

    connectToWiFi();
    String on_time = getFormattedDateTime();
    disconnectFromWiFi();
    latest_on_time = on_time;
    if (DEBUG)
        Serial.println("Machine ON : " + on_time);
}

void machineOff()
{
    if(DEBUG)
        Serial.println("Detected Machine OFF");
    machine_state = 0;
    digitalWrite(LED_PIN, LOW);      // Turn off LED
    time_t off_time_ctime = time(0); // record ctime in case API fails here

    connectToWiFi();
    String off_time = getFormattedDateTime();
    disconnectFromWiFi();
    if (off_time == "TIMEOUT" || latest_on_time == "TIMEOUT")
    {
        time_t delta_time = off_time_ctime - latest_on_time_ctime;
        if(DEBUG)
            Serial.println("Machine OFF" + off_time + " ; Uptime: " + ctime(&delta_time));
        connectToWiFi();
        bool sent = sendToWebApp(latest_on_time, off_time, String(delta_time));
        if (sent && DEBUG)
        {
            Serial.println("Data sent to web app");
        }
        else
        {
            Serial.println("Failed to send data to web app");
        }
        disconnectFromWiFi();
    }
    else
    {
        connectToWiFi();
        bool sent = sendToWebApp(latest_on_time, off_time, getDeltaTime(latest_on_time, off_time));
        if (sent && DEBUG)
        {
            Serial.println("Data sent to web app");
        }
        else
        {
            Serial.println("Failed to send data to web app");
        }
        disconnectFromWiFi();
        if (DEBUG)
            Serial.println("Machine OFF : " + off_time + " ; Uptime: " + getDeltaTime(latest_on_time, off_time));
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

// get the time difference between the latest on time and the off time, String formatted as "YYYY-MM-DD HH:MM:SS"
// the difference should be in "HH:MM:SS" format in length
String getDeltaTime(String on, String off)
{
    int on_year = on.substring(0, 4).toInt();
    int on_month = on.substring(5, 7).toInt();
    int on_day = on.substring(8, 10).toInt();
    int on_hour = on.substring(11, 13).toInt();
    int on_minute = on.substring(14, 16).toInt();
    int on_second = on.substring(17, 19).toInt();

    int off_year = off.substring(0, 4).toInt();
    int off_month = off.substring(5, 7).toInt();
    int off_day = off.substring(8, 10).toInt();
    int off_hour = off.substring(11, 13).toInt();
    int off_minute = off.substring(14, 16).toInt();
    int off_second = off.substring(17, 19).toInt();

    int delta_year = off_year - on_year;
    int delta_month = off_month - on_month;
    int delta_day = off_day - on_day;
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
    if (delta_hour < 0)
    {
        delta_hour += 24;
        delta_day--;
    }
    if (delta_day < 0)
    {
        delta_day += 30;
        delta_month--;
    }
    if (delta_month < 0)
    {
        delta_month += 12;
        delta_year--;
    }

    return String(delta_year) + "-" + String(delta_month) + "-" + String(delta_day) + " " + String(delta_hour) + ":" + String(delta_minute) + ":" + String(delta_second);
}

// Prerequirement: connected to WiFi
bool sendToWebApp(String on_time, String off_time, String uptime)
{

    HTTPClient http;
    http.begin(WEB_APP_URL);
    http.addHeader("Content-Type", "application/json"); // Set JSON header

    // Create JSON payload
    String jsonPayload = "{\"machine\":\"" + String(MACHINE_NAME) + "\",\"on_time\":\"" + on_time + "\",\"off_time\":\"" + off_time + "\",\"uptime\":\"" + uptime + "\"}";

    Serial.println("Sending JSON: " + jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0)
    {
        Serial.println("Response Code: " + String(httpResponseCode));
        // Serial.println("Response: " + http.getString());
    }
    else
    {
        Serial.println("Error sending request: " + String(httpResponseCode));
    }

    http.end();
}

bool connectToWiFi()
{
    bool connected = false;
    connected = connectToWifi(WIFI_SSID, WIFI_PASS);
    // Connect to WiFi
    if (connected && DEBUG)
    {
        Serial.println("Connected to WiFi");
    }
    else
    {
        Serial.println("Failed to connect to WiFi");
    }
    return connected;
}

void disconnectFromWiFi()
{
    WiFi.disconnect();
    if (DEBUG)
    {
        Serial.println("Disconnected from WiFi");
    }
}