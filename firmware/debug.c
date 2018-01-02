/*
 * debug.c
 *
 *  Created on: 30.06.2011
 *      Author: wenkm
 */

#include "debug.h"
#include "uart.h"
#include <avr/interrupt.h>

#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.

#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch!
#endif

static FILE _debug_file_handle;
FILE* _df = 0;

void debug_init_uart(void)
{
	fdev_setup_stream(&_debug_file_handle, debug_uart_put, NULL, _FDEV_SETUP_WRITE);
	_df = &_debug_file_handle;
}

int __attribute__((noinline))
debug_uart_put(char d, FILE *stream)
{
	//if (d == '\n') debug_uart_put('\r', stream);
	//if (d == 0x1b) d = '^'; /* replace escape sequences. */

	uart_putc(d);
	return 0;
}
