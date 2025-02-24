#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include <ctime>
#include "VARS.h"
#include "datetime_handler.h"
#include "wifi_handler.h"

// Global Variable Declarations; do not touch
extern int machine_state;
extern String latest_on_time;
extern time_t latest_on_time_ctime;

void machineOn(); // Machine ON detection handling
void machineOff(); // Machine OFF detection handling
bool isMachineOn(); // Check if machine is ON
bool isMachineOff(); // Check if machine is OFF

#endif
