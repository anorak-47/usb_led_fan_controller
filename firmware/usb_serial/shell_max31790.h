#ifndef __SHELL_MAX31790_H_INCLUDED__
#define __SHELL_MAX31790_H_INCLUDED__

#include "shell.h"
#include <avr/pgmspace.h>

#if FAN_PWM_MAX31790_SUPPORTED && CTRL_DEBUG
extern const struct _s_shell_cmd max_shell_cmd[] PROGMEM;
#endif

#endif
