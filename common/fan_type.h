#ifndef __FAN_TYPE_H_INCLUDED__
#define __FAN_TYPE_H_INCLUDED__

#include "config.h"
#include "types.h"
#include <inttypes.h>
#include <avr/eeprom.h>

// Definition of struct to hold all data (configuration, status etc.) for a single fan.
typedef struct
{
    struct
    {
#if USE_BITFIELDS
        uint8_t snsIdx : 3;  // index of sensor controlling fan speed. The number of bits must be able to hold MAX_SNS
                             // values.
        FANTYPE fanType : 2; // type of fan connected (if any). The number of bits must be able to hold all FANTYPE values!
        FANMODE fanMode : 2; // mode of fan connected. The number of bits must be able to hold all FANMODE values!
#else
        uint8_t snsIdx;
        FANTYPE fanType;
        FANMODE fanMode;
#endif
    } config;
    struct
    {
#if USE_BITFIELDS
        uint8_t stalled : 1; // when set, fan is regarded stalled (rps < min_rps)
#else
        uint8_t stalled;
#endif
    } status;
    uint8_t duty;            // duty cycle of fan; calculated in updateCtrl. [0..255] = [0%..100%]
    uint8_t pinrise_count;   // counts number of pinrises/second.
    uint8_t inc_per_pinrise; // fractional increments per low/high transition in pinchange interrupt. Most fans generate
                             // 2 pulses/rev, resulting in 2 rises. 256/2=128.
    uint16_t rps;            // current revolutions/sec; updated once per second. High byte = full revolutions, low byte = fraction.
    uint8_t min_rps;         // minimum number of revolutions, otherwise the fan will be seen as stalled.
    uint16_t rpm;            // current revolutions/min; updated once per second.
    // -- Parameters for fan in mode fixed duty cycle.
    uint8_t dutyFixed;
#if FAN_MODE_TRIP_POINTS_SUPPORTED || FAN_MODE_LIN_TRIP_POINTS_SUPPORTED
    struct
	{
    	int8_t value;
    	uint8_t duty;
	} trip_point[MAX_TRIP_POINTS];
#endif
#if FAN_MODE_LINEAR_SUPPORTED
    // -- Parameters for fan in mode linear.
    int16_t dutyOffs;
    int16_t dutyGain; // lower 6(!) bits = fraction.
#endif
#if FAN_MODE_PI_SUPPORTED || FAN_MODE_FUZZY_SUPPORTED
    // -- Parameters for fan in mode using a setpoint based on sensor data
    struct
    {
        int8_t delta; // setpoint delta value for sensor (will be added to reference sensor's value) (when under sensor
                      // control)
#if USE_BITFIELDS
        uint8_t snsIdx : 3; // reference sensor index for setpoint value; the setpoint will be added to this sensor's
                            // value to get the actual setpoint
                            // (when under sensor control and reference sensor != SNSTYPE_NONE). The number of bits must be able to hold MAX_SNS
                            // values.
#else
        uint8_t snsIdx;
#endif
    } snsSetp;
#endif
#if FAN_MODE_PI_SUPPORTED
    // -- Parameters for fan in mode PI control.
    int16_t i; // integral portion of PI controller
    int8_t Kp;
    int8_t Ki;
    int8_t Kt;
    int16_t es;
#endif
    // --
    uint8_t dutyMin;
    uint8_t dutyMax;
} Fan;

#if CTRL_DEBUG
typedef struct
{
    int8_t e;
    int16_t p;
} Fan_dbg;
static volatile Fan_dbg fans_dbg[MAX_FANS];
#endif

#endif
