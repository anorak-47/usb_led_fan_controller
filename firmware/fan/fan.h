#ifndef __FAN_H_INCLUDED__
#define __FAN_H_INCLUDED__

#include "config.h"
#include "fan_type.h"
#include <inttypes.h>
#include <avr/eeprom.h>

extern Fan fans[MAX_FANS];
extern Fan EEMEM fans_eeprom[MAX_FANS]; // first entry in eeprom; start of crc calculation.

void fanControlInit(void);
void fanControlUpdate(void);

uint8_t getStatus(void);

#endif
