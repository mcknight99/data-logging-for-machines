#include "fsr_handler.h"

// Read FSR value
// Pin must be a valid analog pin
// Returns the average of READINGS number of readings in time READ_DELAY*READINGS ms
int readFSR(int pin) {
    const int numSamples = READINGS;
    int sum = 0;
    for (int i = 0; i < numSamples; i++) {
        sum += analogRead(pin);
        delay(READ_DELAY);
    }
    return sum / numSamples;
}

// Analyze FSR values
// If the analog state is 1, the FSR is pressed when the FSR value is above the threshold
// If the analog state is 0, the FSR is pressed when the FSR value is below the threshold
void analyzeFSR(int fsrValue1, int fsrValue2) {
    
    if (ANALOG_STATE == 1) {
        if (fsrValue1 > THRESHOLD && isMachineOff()) machineOn();
        if (fsrValue2 > THRESHOLD && isMachineOn()) machineOff();
    } else {
        if (fsrValue1 < THRESHOLD && isMachineOff()) machineOn();
        if (fsrValue2 < THRESHOLD && isMachineOn()) machineOff();
    }
}
