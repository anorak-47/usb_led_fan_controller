/*
    USB Fan Controller - Flexible PWM Fan Controller firmware

    Copyright (C) 2012  Ivo Pullens (info@emmission.nl)
    Copyright (C) 2008  Objective Development Software GmbH

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

/* This header is shared between the firmware and the host software. It
 * defines the USB request numbers (and optionally data types) used to
 * communicate between the host and the device.
 */

#ifndef __REQUESTS_H_INCLUDED__
#define __REQUESTS_H_INCLUDED__

#include "config.h"

typedef enum
{
  CUSTOM_RQ_MIN                   = -1,
  CUSTOM_RQ_ECHO                  = 0,    // Test communication. Can e.g. also be used to reset the communication watchdog.
  CUSTOM_RQ_FUNCS_SUPPORTED       = 1,
  CUSTOM_RQ_STATUS_READ           = 2,

  CUSTOM_RQ_FANTYPE_WRITE         = 10,
  CUSTOM_RQ_FANTYPE_READ          = 11,
  CUSTOM_RQ_FANMODE_WRITE         = 12,
  CUSTOM_RQ_FANMODE_READ          = 13,
  CUSTOM_RQ_FANSNS_WRITE          = 14,
  CUSTOM_RQ_FANSNS_READ           = 15,

  CUSTOM_RQ_FANSETP_DELTA_WRITE   = 20,   // Write PI controller setpoint delta (added to reference sensor value) (FANMODE_PI)
  CUSTOM_RQ_FANSETP_DELTA_READ    = 21,   // Read PI controller setpoint delta (added to reference sensor value) (FANMODE_PI)
  CUSTOM_RQ_FANSETP_READ          = 22,   // Read PI controller setpoint (actual value; sum of reference sensor value & delta) (FANMODE_PI)
  CUSTOM_RQ_FANSETP_REFSNS_WRITE  = 23,   // Write PI controller reference sensor (this sensor's value will be added to the stored setpoint delta to get the real setpoint) (FANMODE_PI)
  CUSTOM_RQ_FANSETP_REFSNS_READ   = 24,   // Read PI controller reference sensor (this sensor's value will be added to the stored setpoint delta to get the real setpoint)

  CUSTOM_RQ_FANKP_WRITE           = 30,   // Write PI controller Kp parameter (FANMODE_PI)
  CUSTOM_RQ_FANKP_READ            = 31,   // Read PI controller Kp parameter (FANMODE_PI)
  CUSTOM_RQ_FANKI_WRITE           = 32,   // Write PI controller Ki parameter (FANMODE_PI)
  CUSTOM_RQ_FANKI_READ            = 33,   // Read PI controller Ki parameter (FANMODE_PI)
  CUSTOM_RQ_FANKT_WRITE           = 34,   // Write PI controller Kt parameter (FANMODE_PI)
  CUSTOM_RQ_FANKT_READ            = 35,   // Read PI controller Kt parameter (FANMODE_PI)

  CUSTOM_RQ_FANGAIN_WRITE         = 36,   // Write linear controller gain parameter (FANMODE_LINEAR)
  CUSTOM_RQ_FANGAIN_READ          = 37,   // Read linear controller gain parameter (FANMODE_LINEAR)
  CUSTOM_RQ_FANOFFS_WRITE         = 38,   // Write linear controller offset parameter (FANMODE_LINEAR)
  CUSTOM_RQ_FANOFFS_READ          = 39,   // Read linear controller offset parameter (FANMODE_LINEAR)

  CUSTOM_RQ_FANDUTYFIXED_WRITE    = 50,   // Write fixed fan duty cycle (FANMODE_FIXED_DUTY)
  CUSTOM_RQ_FANDUTYFIXED_READ     = 51,   // Read fixed fan duty cycle (FANMODE_FIXED_DUTY)

  CUSTOM_RQ_FANMINRPS_WRITE       = 60,
  CUSTOM_RQ_FANMINRPS_READ        = 61,

  CUSTOM_RQ_FANSTALL_READ         = 70,   // Read stall-status of fan
  CUSTOM_RQ_FANRPS_READ           = 71,   // Read fan revolutions/sec
  CUSTOM_RQ_FANRPM_READ           = 72,   // Read fan revolutions/min
  CUSTOM_RQ_FANDUTY_READ          = 73,   // Read fan duty cycle

#if FAN_OUT_SUPPORTED
  CUSTOM_RQ_FANSTALLDETECT_WRITE  = 80,   // Write if fan is included in stall detection for output fan
  CUSTOM_RQ_FANSTALLDETECT_READ   = 81,   // Read if fan is included in stall detection for output fan
#endif

  CUSTOM_RQ_SNSTYPE_WRITE         = 100,
  CUSTOM_RQ_SNSTYPE_READ          = 101,

  CUSTOM_RQ_SNS_WRITE             = 110,
  CUSTOM_RQ_SNS_READ              = 111,

#if FAN_OUT_SUPPORTED
  CUSTOM_RQ_FANOUTMODE_WRITE      = 140,
  CUSTOM_RQ_FANOUTMODE_READ       = 141,

  CUSTOM_RQ_FANOUTRPS_READ        = 150,
  CUSTOM_RQ_FANOUTRPM_READ        = 151,
#endif

#if FASTLED_SUPPORTED
  CUSTOM_RQ_FLED_READ_STATUS      = 160,
  CUSTOM_RQ_FLED_WRITE_STATUS     = 161,
#endif

  CUSTOM_RQ_EEPROM_WRITE          = 200,
  CUSTOM_RQ_EEPROM_READ           = 201,
  CUSTOM_RQ_LOAD_DEFAULTS         = 202,

#if EEPROM_UPDOWNLOAD
  CUSTOM_RQ_EEPROM_DOWNLOAD       = 203,  // Download full eeprom contents from eeprom
  CUSTOM_RQ_EEPROM_UPLOAD         = 204,  // Upload full eeprom contents to eeprom, and activate as new settings.
#endif

  #if CTRL_DEBUG
  CUSTOM_RQ_FANE_READ             = 240,
  CUSTOM_RQ_FANP_READ             = 241,
  CUSTOM_RQ_FANI_READ             = 242,
#endif

#if ENTER_BOOTLOADER_REMOTELY
  CUSTOM_RQ_ENTER_BOOTLOAD        = 250,	// Enter bootloader (when present). Call 3 times in a row to actually reset AVR and enter bootloader.
#endif

  CUSTOM_RQ_UNSUPPORTED           = 255,
  CUSTOM_RQ_MAX
} CUSTOM_RQ;


#endif /* __REQUESTS_H_INCLUDED__ */
