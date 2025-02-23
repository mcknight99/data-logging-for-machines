#ifndef STATE_H
#define STATE_H

#include "VARS.h"
#include "datetime_handler.h"
#include "wifi_handler.h"

void machineOn(); // Machine ON detection handling
void machineOff(); // Machine OFF detection handling
bool isMachineOn(); // Check if machine is ON
bool isMachineOff(); // Check if machine is OFF

#endif
