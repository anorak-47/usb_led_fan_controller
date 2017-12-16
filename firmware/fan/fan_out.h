#ifndef __FAN_OUT_H_INCLUDED__
#define __FAN_OUT_H_INCLUDED__

#include "config.h"
#include "types.h"
#include "fan_out_type.h"
#include <inttypes.h>
#include <avr/eeprom.h>

#if FAN_OUT_SUPPORTED

extern Fan_out fan_out[MAX_FAN_OUTS];
extern Fan_out EEMEM fan_out_eeprom[MAX_FAN_OUTS];

void initFanOut(void);
void setFanOut(void);
void resetFanOutCounter(void);
void updateFanOut(void);
uint16_t ovf_interval(uint8_t channel);

#endif
#endif
