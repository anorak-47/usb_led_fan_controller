/*
    USB Fan Controller - Flexible PWM Fan Controller Library

    Copyright (C) 2012  Ivo Pullens (info@emmission.nl)

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

#ifndef USBFACE_H_INCLUDED
#define USBFACE_H_INCLUDED

#include "hid_device.h"
#include "types.h" // types exchanged with the USB device

// usbface error codes:
#define USBFACE_SUCCESS 0      // no error (equals opendevice's USBOPEN_SUCCESS)
#define USBFACE_ERR_ACCESS 1   // not enough permissions to open device (equals opendevice's USBOPEN_ERR_ACCESS)
#define USBFACE_ERR_IO 2       // I/O error (equals opendevice's USBOPEN_ERR_IO)
#define USBFACE_ERR_NOTFOUND 3 // device not found (equals opendevice's USBOPEN_ERR_NOTFOUND)
#define USBFACE_ERR_READ 4     // Read error
#define USBFACE_ERR_WRITE 5    // Write error
#define USBFACE_ERR_SPI_CLK 6  // Illegal SPI clock configuration
#define USBFACE_ERR_MALLOC 7   // Failed to allocate memory
#define USBFACE_ERR_UNSUPP 8   // Unsupported request

const char *usbfaceErrToString(const int err);
const char *usbfaceSensorTypeToString(const SNSTYPE type);
const char *usbfaceFanModeToString(const FANMODE mode);
const char *usbfaceFanOutModeToString(const FANOUTMODE mode);
const char *usbfaceFanTypeToString(const FANTYPE type, const int desc);

int usbfacePing(hid_device *device);
int usbfaceProtocolVersion(hid_device *device, unsigned char *version);
int usbfaceFirmwareVersion(hid_device *device, unsigned char *major, unsigned char *minor);
int usbfaceFuncsSupportedRead(hid_device *device, SUPPORTED *funcs);

int usbfaceGetNrOfFans();
int usbfaceGetNrOfFanOuts();
int usbfaceGetNrOfSensors();

int usbfaceFanRpsRead(hid_device *device, const unsigned char channel, double *rpm);
int usbfaceFanRpmRead(hid_device *device, const unsigned char channel, unsigned int *rpm);

/// Read the duty cycle of the PWM output for a fan.
/// \param[in] channel  Fan channel to read duty cycle for.
/// \param[out] duty    Actual duty cycle for fan. A value of 0 represents 0%, a value of 1 represents 100%.
int usbfaceFanDutyRead(hid_device *device, const unsigned char channel, double *duty);
int usbfaceFanDutyRawRead(hid_device *device, const unsigned char channel, unsigned char *duty);

/// Change the fixed duty cycle of the PWM output for a fan.
/// \param[in] channel  Fan channel to change duty cycle for.
/// \param[in] duty     Duty cycle for fan. A value of 0 represents 0%, a value of 1 represents 100%.
int usbfaceFanDutyFixedWrite(hid_device *device, const unsigned char channel, const double duty);
int usbfaceFanDutyFixedRawWrite(hid_device *device, const unsigned char channel, const unsigned char duty);
int usbfaceFanDutyFixedRead(hid_device *device, const unsigned char channel, double *duty);
int usbfaceFanDutyFixedRawRead(hid_device *device, const unsigned char channel, unsigned char *duty);

int usbfaceFanTypeWrite(hid_device *device, const unsigned char channel, const FANTYPE type);
int usbfaceFanTypeRead(hid_device *device, const unsigned char channel, FANTYPE *type);
int usbfaceFanModeWrite(hid_device *device, const unsigned char channel, const FANMODE mode);
int usbfaceFanModeRead(hid_device *device, const unsigned char channel, FANMODE *mode);
int usbfaceFanSetpointWrite(hid_device *device, const unsigned char channel, const double setpoint,
                            const int refsnsidx);
int usbfaceFanSetpointActualRead(hid_device *device, const unsigned char channel, double *setpoint);
int usbfaceFanSetpointRead(hid_device *device, const unsigned char channel, double *setpoint, int *refsnsidx);
int usbfaceFanPidWrite(hid_device *device, const unsigned char channel, const double kp, const double ki,
                       const double kt);
int usbfaceFanPidRead(hid_device *device, const unsigned char channel, double *kp, double *ki, double *kt);
int usbfaceFanGainOffsWrite(hid_device *device, const unsigned char channel, const double gain, const double offs);
int usbfaceFanGainOffsRead(hid_device *device, const unsigned char channel, double *gain, double *offs);
int usbfaceFanTripPointRead(hid_device *device, const unsigned char channel, const unsigned int point, unsigned int *value, unsigned int *duty);
int usbfaceFanTripPointWrite(hid_device *device, const unsigned char channel, const unsigned int point, unsigned int value, unsigned int duty);
int usbfaceFanSensorWrite(hid_device *device, const unsigned char channel, const int snsidx);
/// Read which sensor is currently used to automatically control the fan.
int usbfaceFanSensorRead(hid_device *device, const unsigned char channel, int *snsidx);
int usbfaceFanMinRpmWrite(hid_device *device, const unsigned char channel, const double minrpm);
int usbfaceFanMinRpmRead(hid_device *device, const unsigned char channel, double *minrpm);
int usbfaceFanStallRead(hid_device *device, const unsigned char channel, int *stalled);
int usbfaceFanStallDetectWrite(hid_device *device, const unsigned char channel, const int stalldetect);
int usbfaceFanStallDetectRead(hid_device *device, const unsigned char channel, int *stalldetect);

int usbfaceSnsTypeWrite(hid_device *device, const unsigned char channel, const SNSTYPE type);
int usbfaceSnsTypeRead(hid_device *device, const unsigned char channel, SNSTYPE *type);
int usbfaceSnsTypeIsSupported(hid_device *device, const SNSTYPE type);
int usbfaceSnsTypeIsSupportedByFunctions(int funcs, const SNSTYPE type);
int usbfaceSnsWrite(hid_device *device, const unsigned char channel, const double snsvalue);
int usbfaceSnsRead(hid_device *device, const unsigned char channel, double *snsvalue);

int usbfaceFanOutModeWrite(hid_device *device, const unsigned char channel, const FANOUTMODE mode);
int usbfaceFanOutModeRead(hid_device *device, const unsigned char channel, FANOUTMODE *mode);
int usbfaceFanOutModeIsSupported(hid_device *device, const FANOUTMODE mode);
int usbfaceFanOutRpmRead(hid_device *device, const unsigned char channel, double *rpm);

int usbfaceFastledAnimationIdRead(hid_device *device, const unsigned char channel, unsigned char *id);
int usbfaceFastledAnimationIdWrite(hid_device *device, const unsigned char channel, unsigned char id);
int usbfaceFastledAnimationOptionRead(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char *id);
int usbfaceFastledAnimationOptionWrite(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char option);
int usbfaceFastledStateRead(hid_device *device, const unsigned char channel, unsigned char *running);
int usbfaceFastledStateWrite(hid_device *device, const unsigned char channel, unsigned char running);
int usbfaceFastledColorRead(hid_device *device, const unsigned char channel, const unsigned char animation, const unsigned char colorid, unsigned char *colors);
int usbfaceFastledColorWrite(hid_device *device, const unsigned char channel, const unsigned char animation, const unsigned char colorid, unsigned char *colors);
int usbfaceFastledAutostartRead(hid_device *device, const unsigned char channel, unsigned char *start);
int usbfaceFastledAutostartWrite(hid_device *device, const unsigned char channel, unsigned char start);
int usbfaceFastledSnsIdRead(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char *snsId);
int usbfaceFastledSnsIdWrite(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char snsId);
int usbfaceFastledFPSRead(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char *fps);
int usbfaceFastledFPSWrite(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char fps);

int usbfacePowerMeterPowerRead(hid_device *device, const unsigned char channel, unsigned int *milliwatt);
int usbfacePowerMeterCurrentRead(hid_device *device, const unsigned char channel, unsigned int *milliampere);
int usbfacePowerMeterLoadRead(hid_device *device, const unsigned char channel, unsigned int *millivolt);

int usbfaceReadSettings(hid_device *device);
int usbfaceWriteSettings(hid_device *device);
int usbfaceDefaultSettings(hid_device *device);

// Enter the bootloader to update firmware. Call a number of times in a row (BOOTLOADER_ENTER_NUM_REQ) to actually enter
// it!
int usbfaceEnterBootloader(hid_device *device);
// Resets the device, causing re-enumeration. The handle will no longer be valid!
int usbfaceResetDevice(hid_device *device);

#endif
