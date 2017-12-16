#ifndef __SENSOR_TYPE_H_INCLUDED__
#define __SENSOR_TYPE_H_INCLUDED__

#include "config.h"
#include <inttypes.h>

// Definition of struct to hold all data for a single sensor.
typedef struct
{
    struct
    {
#if USE_BITFIELDS
        uint8_t valid : 1; // current temperature reading is valid
#else
        uint8_t valid;
#endif
    } status;
    uint8_t type; // temperature sensor type, e.g SnsType_Temp_NTC1.
    int8_t value; // current value (e.g. temperature in C) of sensor.
} Sensor;

#endif
