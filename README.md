# data-logging-for-machines
 
This project is a commission to create a system that combines with an ESP32 microcontroller to independently log a collections of machines when they were on, off, and their uptimes.

The system includes 2 pressure sensors that act as infinite resistance until pressed.
Once pressed, it pulls an analog pin from a known unpressed state to a known pressed state (ex. from GND unpressed to 5V pressed).

The system registers one pressure sensor as the "on" button and one as the "off" button. The program only toggles once between states, so it cannot be toggled to "on" when it is already "on", and vice versa. The program launches in the "off" state.
When the "on" button is pressed, it sends an API request for the current date and time and stores it as the latest on time.
When the "off" button is pressed, it sends an API request for the current date and time and posts the on time, the off time, and the uptime of the machine.
When either button is pressed, it connects to the WiFi temporarily to make the API requests, and disconnects as soon as API requests have finished.

The off button posts to a custom Apps Script in Google Sheets that logs the posted times to a sheet that is named after the Machine Name, so all machine data is centralized.

An optional LED may be connected to an LED pin that will output status (solid on = machine on, solid off = machine off, blinking = connecting to wifi or retrying API request, turning machine off: off-on-off = uploading data)

VARS.h contains all variables that a tech savvy user would use if they needed to modify their system slightly to get the whole project working.

AUTH.h is not included in this project publicly, and will need to be included at /include/AUTH.h with the following:

```
#ifndef AUTH_H
#define AUTH_H

#define WIFI_SSID "SSID"
#define WIFI_PASS "PASSWORD"

#define WEB_APP_URL "https://WEB.APP.URL/"

#endif
```