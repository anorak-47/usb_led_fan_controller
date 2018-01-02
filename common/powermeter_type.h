#ifndef __POWERMETER_TYPE_H_INCLUDED__
#define __POWERMETER_TYPE_H_INCLUDED__

#include "config.h"
#include <inttypes.h>

// Definition of struct to hold all data for a single sensor.
typedef struct
{
    uint16_t power;   // power in mW
    uint16_t load;    // load in mV
    uint16_t shunt;   // shunt in mV
    uint16_t bus;     // bus load in mV
    uint16_t current; // current in mA
} PowerMeter;

#endif
