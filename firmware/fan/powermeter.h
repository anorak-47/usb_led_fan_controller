#ifndef __POWERMETER_H_INCLUDED__
#define __POWERMETER_H_INCLUDED__

#include "config.h"
#include "powermeter_type.h"
#include <inttypes.h>
#include <avr/eeprom.h>

void initPowerMeter(void);
void updatePowerMeter(void);

extern PowerMeter powermeters[MAX_POWERMETER];
//extern PowerMeter EEMEM powermeters_eeprom[MAX_POWERMETER];

#endif
