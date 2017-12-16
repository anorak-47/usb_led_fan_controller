#ifndef __SHELL_POWERMETER_H_INCLUDED__
#define __SHELL_POWERMETER_H_INCLUDED__

#include "shell.h"
#include <avr/pgmspace.h>

#if POWER_METER_SUPPORTED
extern const struct _s_shell_cmd powermeter_shell_cmd[] PROGMEM;
#endif

#endif
