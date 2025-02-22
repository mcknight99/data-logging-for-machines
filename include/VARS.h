#ifndef VARS_H
#define VARS_H

#define MACHINE_NAME "Dust Collection"

#define FSR1_PIN 2  // GPIO2 (A0) - Machine ON
#define FSR2_PIN 3 // GPIO3 (A1) - Machine OFF
#define LED_PIN 8 // GPIO8 - LED

#define HIGH1 10 // GPIO10 - Always HIGH for FSR1
#define HIGH2 9 // GPIO9 - Always HIGH for FSR2


#define READINGS 5 // Samples to take for running average
#define READ_DELAY 5  // Sample rate delay in ms
#define THRESHOLD 200  // Threshold for sample average to have a pressed state (analog range from 0-4095; 0V-3.3V)
#define ANALOG_STATE 0 // Detection mode for FSRs.
                       // 1 = expecting HIGH/3.3V for pressed state, will be testing for values above threshold
                       // 0 = expecting LOW/GND for pressed state, will be testing for values below threshold

#define WIFI_TIMEOUT 10000 // Timeout for WiFi connection in ms
#define API_TIMEOUT 3 // Attempts to connect to worldtimeapi.org
#define DEBUG 1 // 1 = debug mode; print to console

#endif