#ifndef VARS_H
#define VARS_H

#include <Arduino.h>
#include <ctime>

// User Variables; change as needed
#define MACHINE_NAME "Dust Collection" // Machine name, will be used to identify sheet in Google Sheets
#define FSR1_PIN 2                     // FSR1 pin
#define FSR2_PIN 3                     // FSR2 pin
#define LED_PIN 8                      // LED pin
#define HIGH1 10                       // HIGH1 pin for FSR1 power
#define HIGH2 9                        // HIGH2 pin for FSR2 power
#define READINGS 10                    // Number of readings to average
#define READ_DELAY 5                   // Delay between readings in ms
#define THRESHOLD 200                  // Threshold value for FSR (range of 0-4095 digital = 0.0V-3.3V analog)
#define ANALOG_STATE 0                 // 0 for detecting GND as pressed, 1 for detecting HIGH as pressed
#define UPLOAD_TIMEOUT 30000           // Timeout for uploading data to web app in ms
#define WIFI_TIMEOUT 10000             // Timeout for connecting to WiFi in ms
#define API_TIMEOUT 5                  // Number of API request attempts before timeout
#define DEBUG 1                        // 1 for debugging prints to serial monitor, 0 for no prints

// Global Variable Declarations; do not touch
extern int machine_state;
extern String latest_on_time;
extern time_t latest_on_time_ctime;

#endif // VARS_H
