#ifndef VARS_H
#define VARS_H

#include <Arduino.h>
#include <ctime>

// User Variables; change as needed
#define MACHINE_NAME "Dust Collection" // Machine name, will be used to identify sheet in Google Sheets

#define FSR1_PIN 2                     // FSR1 pin
#define FSR2_PIN 3                     // FSR2 pin
#define LED_PIN 8                      // LED pin
#define OPENSTATEPIN1 10               // open state (unpressed) pin output for FSR1 default state
#define OPENSTATEPIN2 9                // open state (unpressed) pin output for FSR2 default state

#define READINGS 10                    // Number of readings to average
#define READ_DELAY 5                   // Delay between readings in ms
#define THRESHOLD 30                   // Threshold value for FSR (range of 0-4095 digital = 0.0V-3.3V analog)
#define ANALOG_STATE 1                 // 0 for detecting below THRESHOLD/LOW as closed state (pressed), 
                                            // 1 for detecting above threshold/HIGH as closed state. 
                                            // OPENSTATE pins will be opposite of this value
                                            // For a default LOW, draw to HIGH state, I recommend using 5V to get drawn to
                                            // For a default HIGH state, draw to LOW state, I recommend using 3.3V default high

#define UPLOAD_TIMEOUT 30000           // Timeout for uploading data to web app in ms
#define WIFI_TIMEOUT 10000             // Timeout for connecting to WiFi in ms
#define API_TIMEOUT 5                  // Number of API request attempts before timeout

#define DEBUG 1                        // 1 for debugging prints to serial monitor, 0 for no prints

#endif // VARS_H
