#ifndef __SHELL_INA219_H_INCLUDED__
#define __SHELL_INA219_H_INCLUDED__

#include "shell.h"
#include <avr/pgmspace.h>

#if POWER_METER_INA219 & DEBUG_FUNCTIONS_SUPPORTED
extern const struct _s_shell_cmd ina219_shell_cmd[] PROGMEM;
#endif

#endif
