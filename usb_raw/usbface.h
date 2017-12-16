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

#include <usb.h>
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

#ifdef __cplusplus
extern "C" {
#endif

const char *usbfaceErrToString(const int err);
const char *usbfaceSensorTypeToString(const SNSTYPE type);
const char *usbfaceFanModeToString(const FANMODE mode);
const char *usbfaceFanOutModeToString(const FANOUTMODE mode);
const char *usbfaceFanTypeToString(const FANTYPE type, const int desc);

int usbfaceOpen(usb_dev_handle **device);
int usbfaceClose(usb_dev_handle *device);
int usbfacePing(usb_dev_handle *device);
int usbfaceProtocolVersion(usb_dev_handle *device, unsigned char *version);
int usbfaceFirmwareVersion(usb_dev_handle *device, unsigned char *major, unsigned char *minor);
int usbfaceFuncsSupportedRead(usb_dev_handle *device, SUPPORTED *funcs);

int usbfaceGetNrOfFans();
int usbfaceGetNrOfFanOuts();
int usbfaceGetNrOfSensors();

int usbfaceFanRpsRead(usb_dev_handle *device, const unsigned char channel, double *rpm);
int usbfaceFanRpmRead(usb_dev_handle *device, const unsigned char channel, unsigned int *rpm);

/// Read the duty cycle of the PWM output for a fan.
/// \param[in] channel  Fan channel to read duty cycle for.
/// \param[out] duty    Actual duty cycle for fan. A value of 0 represents 0%, a value of 1 represents 100%.
int usbfaceFanDutyRead(usb_dev_handle *device, const unsigned char channel, double *duty);
int usbfaceFanDutyRawRead(usb_dev_handle *device, const unsigned char channel, unsigned char *duty);

/// Change the fixed duty cycle of the PWM output for a fan.
/// \param[in] channel  Fan channel to change duty cycle for.
/// \param[in] duty     Duty cycle for fan. A value of 0 represents 0%, a value of 1 represents 100%.
int usbfaceFanDutyFixedWrite(usb_dev_handle *device, const unsigned char channel, const double duty);
int usbfaceFanDutyFixedRawWrite(usb_dev_handle *device, const unsigned char channel, const unsigned char duty);
int usbfaceFanDutyFixedRead(usb_dev_handle *device, const unsigned char channel, double *duty);
int usbfaceFanDutyFixedRawRead(usb_dev_handle *device, const unsigned char channel, unsigned char *duty);

int usbfaceFanTypeWrite(usb_dev_handle *device, const unsigned char channel, const FANTYPE type);
int usbfaceFanTypeRead(usb_dev_handle *device, const unsigned char channel, FANTYPE *type);
int usbfaceFanModeWrite(usb_dev_handle *device, const unsigned char channel, const FANMODE mode);
int usbfaceFanModeRead(usb_dev_handle *device, const unsigned char channel, FANMODE *mode);
int usbfaceFanSetpointWrite(usb_dev_handle *device, const unsigned char channel, const double setpoint,
                            const int refsnsidx);
int usbfaceFanSetpointActualRead(usb_dev_handle *device, const unsigned char channel, double *setpoint);
int usbfaceFanSetpointRead(usb_dev_handle *device, const unsigned char channel, double *setpoint, int *refsnsidx);
int usbfaceFanPidWrite(usb_dev_handle *device, const unsigned char channel, const double kp, const double ki,
                       const double kt);
int usbfaceFanPidRead(usb_dev_handle *device, const unsigned char channel, double *kp, double *ki, double *kt);
int usbfaceFanGainOffsWrite(usb_dev_handle *device, const unsigned char channel, const double gain, const double offs);
int usbfaceFanGainOffsRead(usb_dev_handle *device, const unsigned char channel, double *gain, double *offs);
int usbfaceFanTripPointRead(usb_dev_handle *device, const unsigned char channel, const unsigned int point, unsigned int *value, unsigned int *duty);
int usbfaceFanTripPointWrite(usb_dev_handle *device, const unsigned char channel, const unsigned int point, unsigned int value, unsigned int duty);
int usbfaceFanSensorWrite(usb_dev_handle *device, const unsigned char channel, const int snsidx);
/// Read which sensor is currently used to automatically control the fan.
int usbfaceFanSensorRead(usb_dev_handle *device, const unsigned char channel, int *snsidx);
int usbfaceFanMinRpmWrite(usb_dev_handle *device, const unsigned char channel, const double minrpm);
int usbfaceFanMinRpmRead(usb_dev_handle *device, const unsigned char channel, double *minrpm);
int usbfaceFanStallRead(usb_dev_handle *device, const unsigned char channel, int *stalled);
int usbfaceFanStallDetectWrite(usb_dev_handle *device, const unsigned char channel, const int stalldetect);
int usbfaceFanStallDetectRead(usb_dev_handle *device, const unsigned char channel, int *stalldetect);

int usbfaceSnsTypeWrite(usb_dev_handle *device, const unsigned char channel, const SNSTYPE type);
int usbfaceSnsTypeRead(usb_dev_handle *device, const unsigned char channel, SNSTYPE *type);
int usbfaceSnsTypeIsSupported(usb_dev_handle *device, const SNSTYPE type);
int usbfaceSnsTypeIsSupportedByFunctions(int funcs, const SNSTYPE type);
int usbfaceSnsWrite(usb_dev_handle *device, const unsigned char channel, const double snsvalue);
int usbfaceSnsRead(usb_dev_handle *device, const unsigned char channel, double *snsvalue);

int usbfaceFanOutModeWrite(usb_dev_handle *device, const unsigned char channel, const FANOUTMODE mode);
int usbfaceFanOutModeRead(usb_dev_handle *device, const unsigned char channel, FANOUTMODE *mode);
int usbfaceFanOutModeIsSupported(usb_dev_handle *device, const FANOUTMODE mode);
int usbfaceFanOutRpmRead(usb_dev_handle *device, const unsigned char channel, double *rpm);

int usbfaceFastledAnimationIdRead(usb_dev_handle *device, const unsigned char channel, unsigned char *id);
int usbfaceFastledAnimationIdWrite(usb_dev_handle *device, const unsigned char channel, unsigned char id);
int usbfaceFastledStateRead(usb_dev_handle *device, const unsigned char channel, unsigned char *running);
int usbfaceFastledStateWrite(usb_dev_handle *device, const unsigned char channel, unsigned char running);
int usbfaceFastledColorRead(usb_dev_handle *device, const unsigned char channel, unsigned char colors[6]);
int usbfaceFastledColorWrite(usb_dev_handle *device, const unsigned char channel, unsigned char colors[6]);
int usbfaceFastledAutostartRead(usb_dev_handle *device, const unsigned char channel, unsigned char *start);
int usbfaceFastledAutostartWrite(usb_dev_handle *device, const unsigned char channel, unsigned char start);
int usbfaceFastledSnsIdRead(usb_dev_handle *device, const unsigned char channel, unsigned char *id);
int usbfaceFastledSnsIdWrite(usb_dev_handle *device, const unsigned char channel, unsigned char id);
int usbfaceFastledFPSRead(usb_dev_handle *device, const unsigned char channel, unsigned char *fps);
int usbfaceFastledFPSWrite(usb_dev_handle *device, const unsigned char channel, unsigned char fps);

int usbfacePowerMeterPowerRead(usb_dev_handle *device, const unsigned char channel, unsigned int *milliwatt);
int usbfacePowerMeterCurrentRead(usb_dev_handle *device, const unsigned char channel, unsigned int *milliampere);
int usbfacePowerMeterLoadRead(usb_dev_handle *device, const unsigned char channel, unsigned int *millivolt);

int usbfaceReadSettings(usb_dev_handle *device);
int usbfaceWriteSettings(usb_dev_handle *device);
int usbfaceDefaultSettings(usb_dev_handle *device);

// Enter the bootloader to update firmware. Call a number of times in a row (BOOTLOADER_ENTER_NUM_REQ) to actually enter
// it!
int usbfaceEnterBootloader(usb_dev_handle *device);
// Resets the device, causing re-enumeration. The handle will no longer be valid!
int usbfaceResetDevice(usb_dev_handle *device);

#ifdef __cplusplus
}; // extern "C"
#endif

#endif
