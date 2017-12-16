#ifndef __FAN_OUT_TYPE_H_INCLUDED__
#define __FAN_OUT_TYPE_H_INCLUDED__

#include "config.h"
#include "types.h"
#include <inttypes.h>
#include <avr/eeprom.h>

#if FAN_OUT_SUPPORTED

typedef struct
{
    FANOUTMODE mode;        // mode, e.g. clone fan with minimum rps, maximum rps or a specific fan.
    uint8_t fanStallDetect; // bitmask defining which fans to include in the stall detection.
                            // An rps of 0 is output when any of the selected fans is stalled.
                            // Should contain at least MAX_FANS bits!
    uint8_t rps;            // current rps, as output on connector.
    uint16_t rpm;           // current rpm, as output on connector.
} Fan_out;

#endif
#endif
