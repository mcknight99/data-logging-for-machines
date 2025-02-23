#ifndef FSR_HANDLER_H
#define FSR_HANDLER_H

#include <Arduino.h>
#include "VARS.h"
#include "state_handler.h"

int readFSR(int pin);                          // Read FSR value from pin
void analyzeFSR(int fsrValue1, int fsrValue2); // Analyze FSR values to determine machine state

#endif
