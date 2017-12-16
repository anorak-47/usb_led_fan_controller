#ifndef __SENSOR_H_INCLUDED__
#define __SENSOR_H_INCLUDED__

#include "config.h"
#include "sensor_type.h"
#include <inttypes.h>
#include <avr/eeprom.h>

extern Sensor sns[MAX_SNS];
extern Sensor EEMEM sns_eeprom[MAX_SNS];

void initSns(void);
void updateSns(void);

#endif
