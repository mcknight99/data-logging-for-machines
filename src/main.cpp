// To Do (improve the agreed upon product requirements to guarantee robustness and a good quality product):
// add deep sleep and wakeup functionality to reduce power consumption
    // https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#deep-sleep-mode-and-wake-up
    // Currently the XIAO ESP32C3 only supports GPIO wake-up, and the only pins that support wake-up are D0~D3.
// fix real time zone? not sure if it is my hotspot that is causing the time to be in the wrong timezone or how I'm sending the request

// Requirement Drift Ideas (will not implement due to not agreed upon product, but are ideas that I liked that I came up with):
// add a backlog of requests to send to the web app if the connection is lost or wifi is lost.
// flow: add request to backlog, send request, if response is good, remove request from backlog, if response is bad, keep request in backlog and try again later
// add a sheet to the google sheet that stores default vars to boot up with or overwrite hardcode vars without having to change all existing boards 
    //-> and/or perhaps on the dashboard, a row with easy shop wide variables like wifi info as well as machine specific variables on their own pages
    //-> a local var here that determines if it will use the google sheet vars or local vars on boot
    // or perhaps on each sheet, a locked three rows: 1=var labels, 2=var values, 3=column headers for data and on boot, it reads the second row of the respective sheet that the machine is on
    // this would definitely be useful for variables such as wifi ssid and password, machine name, etc. that are more prone to change and are easier to alter by not having to reupload code
// make ctime the primary time source; use API during setup and use ctime relative to that time to reduce wifi uptime and api call reliance. adds complexity with ctime plus API time, but would be 50% less reliant on API and WiFi

#include <Arduino.h>
#include "VARS.h"
#include "AUTH.h"
#include "fsr_handler.h"
#include "wifi_handler.h"
#include "datetime_handler.h"
#include "state_handler.h"

int machine_state = 0;
String latest_on_time = "";
time_t latest_on_time_ctime = time(0);

void setup()
{
    Serial.begin(115200);
    pinMode(FSR1_PIN, INPUT);
    pinMode(FSR2_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(OPENSTATEPIN1, OUTPUT);
    pinMode(OPENSTATEPIN2, OUTPUT);
    digitalWrite(OPENSTATEPIN1, !ANALOG_STATE);
    digitalWrite(OPENSTATEPIN2, !ANALOG_STATE);

    machine_state = 0;              // Set machine state to off
    latest_on_time = "";            // Set latest on time to empty string
    latest_on_time_ctime = time(0); // Set latest on time ctime to current time

    // 3 quick blinks to indicate machine is ready
    for(int i = 0; i < 3; i++)
    {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }

    if (DEBUG)
    {
        Serial.println("Machine name: " + String(MACHINE_NAME));
        Serial.println("FSR1 pin: " + String(FSR1_PIN));
        Serial.println("FSR2 pin: " + String(FSR2_PIN));
        Serial.println("LED pin: " + String(LED_PIN));
        Serial.println("High1 pin: " + String(OPENSTATEPIN1));
        Serial.println("High2 pin: " + String(OPENSTATEPIN2));
        Serial.println("Machine " + String(MACHINE_NAME) + " is ready");
    }
}

void loop()
{
    int fsrValue1 = readFSR(FSR1_PIN);
    int fsrValue2 = readFSR(FSR2_PIN);
    analyzeFSR(fsrValue1, fsrValue2);
}
