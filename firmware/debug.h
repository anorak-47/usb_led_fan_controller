#pragma once
#include "config.h"
#include <stdio.h>
#include <avr/pgmspace.h>

#define BAUD 38400 // 9600 14400 19200 38400 57600 115200

/* define macros, if debug is enabled */
#if DEBUG_OUTPUT_USE_UART
extern FILE* _df;
#define debug_init() debug_init_uart()
#define debug_printf(s, args...) fprintf_P(_df, PSTR(s), ## args)
#else  /* not DEBUG */
#define debug_init() do {} while(0)
#define debug_printf(s, args...) do {} while(0)
#endif /* not DEBUG */

#if DEBUG_OUTPUT_SUPPORTED
#define LV_(s, args...) debug_printf(s "\n", args)
#define LS_(s) debug_printf(s "\n")
#define L__(s) debug_printf(#s "\n")
#else
#define LV_(s, args...) do {} while(0)
#define LS_(s) do {} while(0)
#define L__(s) do {} while(0)
#endif

void debug_init_uart(void);
int __attribute__((noinline)) debug_uart_put (char d, FILE *stream);
