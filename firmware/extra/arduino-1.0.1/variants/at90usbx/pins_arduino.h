/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

/*
Ports ABCDEF (6 * 8 = 48)
*/

#define NUM_DIGITAL_PINS  48
#define NUM_ANALOG_INPUTS 8  // PF0..7

// Only use TX LED
#define TX_RX_LED_INIT	DDRE |= (1<<7)
#define TXLED0			PORTE |= (1<<7)
#define TXLED1			PORTE &= ~(1<<7)
#define RXLED0			(1)
#define RXLED1			(1)

static const uint8_t SDA = 2;
static const uint8_t SCL = 3;

// Map SPI port to 'new' pins PB0..3
static const uint8_t SS   = 8;
static const uint8_t MOSI = 10;
static const uint8_t MISO = 11;
static const uint8_t SCK  = 9;

// Mapping of analog pins as digital I/O
// A6-A11 share with digital pins
static const uint8_t A0 = 40;
static const uint8_t A1 = 41;
static const uint8_t A2 = 42;
static const uint8_t A3 = 43;
static const uint8_t A4 = 44;
static const uint8_t A5 = 45;
static const uint8_t A6 = 46;	// D4
static const uint8_t A7 = 47;	// D6

//#define digitalPinToPCICR(p)    ((((p) >= 8 && (p) <= 11) || ((p) >= 14 && (p) <= 17) || ((p) >= A8 && (p) <= A10)) ? (&PCICR) : ((uint8_t *)0))
//#define digitalPinToPCICRbit(p) 0
//#define digitalPinToPCMSK(p)    ((((p) >= 8 && (p) <= 11) || ((p) >= 14 && (p) <= 17) || ((p) >= A8 && (p) <= A10)) ? (&PCMSK0) : ((uint8_t *)0))
//#define digitalPinToPCMSKbit(p) ( ((p) >= 8 && (p) <= 11) ? (p) - 4 : ((p) == 14 ? 3 : ((p) == 15 ? 1 : ((p) == 16 ? 2 : ((p) == 17 ? 0 : (p - A8 + 4))))))

#ifdef ARDUINO_MAIN



// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &DDRA,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
	(uint16_t) &DDRE,
	(uint16_t) &DDRF,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &PORTA,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
	(uint16_t) &PORTE,
	(uint16_t) &PORTF,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &PINA,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
	(uint16_t) &PINE,
	(uint16_t) &PINF,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	PA, PA, PA, PA, 
	PA, PA, PA, PA, 

	PB, PB, PB, PB,
	PB, PB, PB, PB,

	PC, PC, PC, PC,
	PC, PC, PC, PC,
	
	PD, PD, PD, PD,
	PD, PD, PD, PD,

	PE, PE, PE, PE,
	PE, PE, PE, PE,

	PF, PF, PF, PF,
	PF, PF, PF, PF
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(0), _BV(1), _BV(2), _BV(3), 
	_BV(4), _BV(5), _BV(6), _BV(7), 

	_BV(0), _BV(1), _BV(2), _BV(3), 
	_BV(4), _BV(5), _BV(6), _BV(7), 

	_BV(0), _BV(1), _BV(2), _BV(3), 
	_BV(4), _BV(5), _BV(6), _BV(7), 

	_BV(0), _BV(1), _BV(2), _BV(3), 
	_BV(4), _BV(5), _BV(6), _BV(7), 

	_BV(0), _BV(1), _BV(2), _BV(3), 
	_BV(4), _BV(5), _BV(6), _BV(7), 

	_BV(0), _BV(1), _BV(2), _BV(3), 
	_BV(4), _BV(5), _BV(6), _BV(7), 
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	// PortA    0
	NOT_ON_TIMER,   
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	// PortB  8
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER2A,
	TIMER1A, 
	TIMER1B,
	TIMER1C,
	// PortC  16
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER3C,
	TIMER3B,
	TIMER3A,
	NOT_ON_TIMER,
	// PortD  24
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	// PortE  32
	NOT_ON_TIMER,
	NOT_ON_TIMER,    
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,  // LED1
	NOT_ON_TIMER,  // LED2
	// PortF  40
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
};

#endif /* ARDUINO_MAIN */
#endif /* Pins_Arduino_h */
