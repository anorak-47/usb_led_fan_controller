/*
    USB Fan Controller - Flexible PWM Fan Controller firmware

    Copyright (C) 2012  Ivo Pullens (info@emmission.nl)
    Copyright (C) 2017  Moritz Wenk (moritzwenk@web.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __TYPES_H_INCLUDED__
#define __TYPES_H_INCLUDED__

#include "config.h"

// Functions supported by the firmware.
// Bitmasks, so multiple functions can be reported
// to be enabled/disabled at a time.
typedef enum
{
  SUPPORTED_NONE             = 0x0000,
  SUPPORTED_SNS_ANALOG       = 0x0001,
  SUPPORTED_SNS_I2C          = 0x0002,
  SUPPORTED_FAN_OUT          = 0x0004,
  SUPPORTED_USBCOMM_WATCHDOG = 0x0008,
  SUPPORTED_FANMODE_LINEAR   = 0x0010,
  SUPPORTED_FANMODE_PI       = 0x0020,
  SUPPORTED_FANMODE_FUZZY    = 0x0040,
  SUPPORTED_EEPROM_UPDOWNLD  = 0x0080,
  SUPPORTED_FANMODE_TP       = 0x0100,
  SUPPORTED_FANMODE_LIN_TP   = 0x0200,
  SUPPORTED_LED_FASTLED      = 0x0400,
  SUPPORTED_SNS_POWER        = 0x0800,
  SUPPORTED_CTRL_DEBUG       = 0x8000
} SUPPORTED;

// Status of the fancontroller.
// Bitmasks, so multiple states can be reported
// at a time.
typedef enum
{
  STATUS_WATCHDOG_RESET      = 0x01,  // A watchdog reset has occurred
  STATUS_BROWNOUT_RESET      = 0x02,  // A brownout reset has occurred
  STATUS_USBCOMM_WATCHDOG_TO = 0x04,  // The USB comm watchdog has been triggered.
} STATUS;


// Definition of all possible sensor types.
typedef enum
{
  SNSTYPE_MIN                 = -1,
  SNSTYPE_NONE                = 0,
  SNSTYPE_TEMP_NTC0           = 10,
  SNSTYPE_TEMP_NTC1           = 11,
  SNSTYPE_TEMP_NTC2           = 12,
  SNSTYPE_TEMP_NTC3           = 13,
  SNSTYPE_TEMP_NTC4           = 14,
  SNSTYPE_TEMP_NTC5           = 15,
  SNSTYPE_TEMP_I2C_ADDR0      = 20,
  SNSTYPE_TEMP_I2C_ADDR1      = 21,
  SNSTYPE_TEMP_I2C_ADDR2      = 22,
  SNSTYPE_TEMP_I2C_ADDR3      = 23,
  SNSTYPE_TEMP_I2C_ADDR4      = 24,
  SNSTYPE_TEMP_I2C_ADDR5      = 25,
  SNSTYPE_TEMP_I2C_ADDR6      = 26,
  SNSTYPE_TEMP_I2C_ADDR7      = 27,
  SNSTYPE_EXT0                = 40,
  SNSTYPE_EXT1                = 41,
  SNSTYPE_EXT2                = 42,
  SNSTYPE_EXT3                = 43,
  SNSTYPE_RPS_FAN0            = 50,
  SNSTYPE_RPS_FAN1            = 51,
  SNSTYPE_RPS_FAN2            = 52,
  SNSTYPE_RPS_FAN3            = 53,
  SNSTYPE_RPS_FAN4            = 54,
  SNSTYPE_RPS_FAN5            = 55,
  SNSTYPE_DUTY_IN0            = 70,
  SNSTYPE_DUTY_IN1            = 71,
  SNSTYPE_POWER0              = 80,
  SNSTYPE_POWER1              = 81,
  SNSTYPE_MAX
} SNSTYPE;

// Type of fan connected
typedef enum
{
#ifndef __AVR__
  FANTYPE_MIN      = -1,
#endif
  FANTYPE_NONE     = 0,            // no fan connected
  FANTYPE_2WIRE    = 1,            // 2 wire - no tacho, no PWM control
  FANTYPE_3WIRE    = 2,            // 3 wire - tacho, no PWM control
  FANTYPE_4WIRE    = 3,            // 4 wire - tacho & PWM control
#ifndef __AVR__
  FANTYPE_MAX
#endif
} FANTYPE;
#define MIN_FANTYPE_WITH_TACHO (FANTYPE_3WIRE)  // Starting from 3-wire fan, all fans have a tacho output.


// Fan controller to calculate a fan's dutycycle
typedef enum
{
#ifndef __AVR__
  FANMODE_MIN          = -1,
#endif
  FANMODE_FIXED_DUTY           = 0, // fixed dutycycle
  FANMODE_PI                   = 1, // PI-controller
  FANMODE_LINEAR               = 2, // linear controller
  FANMODE_FUZZY                = 3, // fuzzy logic controller
  FANMODE_TRIP_POINTS          = 4, // trip points
  FANMODE_LINEAR_TRIP_POINTS   = 5, // trip points with linear interpolation
#ifndef __AVR__
  FANMODE_MAX
#endif
} FANMODE;


// Tacho for simulated-fan output
typedef enum
{
  FANOUTMODE_MIN             = -1,
  FANOUTMODE_RPS_MINFAN      = 0,  // cloned from fan with minimum rps
  FANOUTMODE_RPS_MAXFAN,           // cloned from fan with maximum rps
  FANOUTMODE_RPS_CLONEFAN0,        // cloned from fan 0
  FANOUTMODE_RPS_CLONEFAN1,        // cloned from fan 1
  FANOUTMODE_RPS_CLONEFAN2,        // cloned from fan 2
  FANOUTMODE_RPS_CLONEFAN3,        // cloned from fan 3
  FANOUTMODE_RPS_CLONEFAN4,        // cloned from fan 4
  FANOUTMODE_RPS_CLONEFAN5,        // cloned from fan 5
  FANOUTMODE_RPS_FIXED_50,         // fixed at 50 rps (== 3000 rpm)
  FANOUTMODE_MAX
} FANOUTMODE;

#endif /* __TYPES_H_INCLUDED__ */
