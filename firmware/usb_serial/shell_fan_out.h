#ifndef __SHELL_FAN_OUT_H_INCLUDED__
#define __SHELL_FAN_OUT_H_INCLUDED__

#include "shell.h"
#include <avr/pgmspace.h>

#if FAN_OUT_SUPPORTED

extern const struct _s_shell_cmd fan_out_shell_cmd[] PROGMEM;

#endif
#endif
