#include "state_handler.h"

// Machine state ON handler
// Sets machine state to ON, turns on LED, and records the time
void machineOn()
{
    digitalWrite(LED_PIN, HIGH);
    if (DEBUG)
        Serial.println("Machine ON detected");

    machine_state = 1;
    latest_on_time_ctime = time(0);

    connectToWiFi();
    digitalWrite(LED_PIN, LOW);
    latest_on_time = getFormattedDateTime();
    disconnectFromWiFi();

    if (DEBUG)
        Serial.println("Machine ON : " + latest_on_time);
    digitalWrite(LED_PIN, HIGH);
}

// Machine state OFF handler
// Sets machine state to OFF, turns off LED, records the time, and sends time data to web app
void machineOff()
{
    digitalWrite(LED_PIN, LOW);
    if (DEBUG)
        Serial.println("Machine OFF detected");

    machine_state = 0;

    time_t off_time_ctime = time(0);
    connectToWiFi();
    digitalWrite(LED_PIN, HIGH);
    String off_time = getFormattedDateTime();

    if (off_time == "TIMEOUT" || latest_on_time == "TIMEOUT")
    {
        time_t delta_time = off_time_ctime - latest_on_time_ctime;
        if (DEBUG)
            Serial.println("Machine OFF" + off_time + " ; Uptime: " + ctime(&delta_time));

        bool sent = uploadHandler(latest_on_time, off_time, ctime(&delta_time));
    }
    else
    {
        if (DEBUG)
            Serial.println("Machine OFF : " + off_time + " ; Uptime: " + getDeltaTime(latest_on_time, off_time));

        bool sent = uploadHandler(latest_on_time, off_time, getDeltaTime(latest_on_time, off_time));
    }
    disconnectFromWiFi();
    digitalWrite(LED_PIN, LOW);
}

// Returns true if machine is on
bool isMachineOn()
{
    return machine_state == 1;
}

// Returns true if machine is off
bool isMachineOff()
{
    return machine_state == 0;
}