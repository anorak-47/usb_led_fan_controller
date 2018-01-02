/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the VirtualSerial demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include <ledstripe/animation.h>
#include "lufa.h"
#include "lufa_virtual_serial.h"
#include "fan.h"
#include "powermeter.h"
#include "i2cmaster.h"
#include "usb_generic_hid.h"
#include "uart.h"
#include "shell.h"
#include "debug.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
#ifdef USB_VIRTUAL_SERIAL_SUPPORTED
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
{
	.Config =
		{
			.ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
			.DataINEndpoint           =
				{
					.Address          = CDC_TX_EPADDR,
					.Size             = CDC_TXRX_EPSIZE,
					.Banks            = 1,
				},
			.DataOUTEndpoint =
				{
					.Address          = CDC_RX_EPADDR,
					.Size             = CDC_TXRX_EPSIZE,
					.Banks            = 1,
				},
			.NotificationEndpoint =
				{
					.Address          = CDC_NOTIFICATION_EPADDR,
					.Size             = CDC_NOTIFICATION_EPSIZE,
					.Banks            = 1,
				},
		},
};
#endif

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	//TODO: remove this
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	USB_USBTask();
#ifdef USB_VIRTUAL_SERIAL_SUPPORTED
	virtser_init();
#endif

	LS_("start");

	for (;;)
	{
		animation_loop();
		fanControlUpdate();

#ifdef USB_VIRTUAL_SERIAL_SUPPORTED
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		virtser_recv_task();
#endif
		//uart_recv_task();
		HID_Task();

#if !defined(INTERRUPT_CONTROL_ENDPOINT)
		USB_USBTask();
#endif
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Do not enable interrupts here! */
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	uart_init(UART_BAUD_SELECT(BAUD, F_CPU));
	debug_init();
	i2c_init();
	initPowerMeter();
	animation_init();
	fanControlInit();

	//TODO: remove this
	LEDs_Init();

    // Leonardo needs. Without this USB device is not recognized.
    //USB_Disable();
	USB_Init();

	//TODO: do not call
	//USB_Device_EnableSOFEvents();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

#ifdef USB_VIRTUAL_SERIAL_SUPPORTED
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
#endif

	/* Setup HID Report Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
#ifdef USB_VIRTUAL_SERIAL_SUPPORTED
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
#endif
}

/** Event handler for the USB device Start Of Frame event. */
/*
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Generic_HID_Interface);
}
*/

/** CDC class driver callback function the processing of changes to the virtual
 *  control lines sent from the host..
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo)
{
	/* You can get changes to the virtual CDC lines in this callback; a common
	   use-case is to use the Data Terminal Ready (DTR) flag to enable and
	   disable CDC communications in your application when set to avoid the
	   application blocking while waiting for a host to become ready and read
	   in the pending data from the USB endpoints.
	*/
	//bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
}

/** Function to process the last received report from the host.
 *
 *  \param[in] DataArray  Pointer to a buffer where the last received report has been stored
 */
void ProcessGenericHIDReport(uint8_t* DataArray) __attribute__((weak));
void ProcessGenericHIDReport(uint8_t* DataArray)
{
	/*
		This is where you need to process reports sent from the host to the device. This
		function is called each time the host has sent a new report. DataArray is an array
		holding the report sent from the host.
	*/
}

void SendGenericHIDReport(uint8_t* data, uint8_t length)
{
	// TODO: implement variable size packet
	if ( length != GENERIC_REPORT_SIZE )
		return;

	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	// TODO: decide if we allow calls to raw_hid_send() in the middle
	// of other endpoint usage.
	uint8_t ep = Endpoint_GetCurrentEndpoint();

	Endpoint_SelectEndpoint(GENERIC_IN_EPADDR);

	/* Check to see if the host is ready to accept another packet */
	if (Endpoint_IsINReady())
	{
		/* Write Generic Report Data */
		Endpoint_Write_Stream_LE(data, GENERIC_REPORT_SIZE, NULL);

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearIN();
	}

	Endpoint_SelectEndpoint(ep);
}

void HID_Task(void)
{
	/* Device must be connected and configured for the task to run */
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;

	/* Create a temporary buffer to hold the read in report from the host */
	uint8_t GenericData[GENERIC_REPORT_SIZE];
	bool dataRead = false;

	Endpoint_SelectEndpoint(GENERIC_OUT_EPADDR);

	/* Check to see if a packet has been sent from the host */
	if (Endpoint_IsOUTReceived())
	{
		/* Check to see if the packet contains data */
		if (Endpoint_IsReadWriteAllowed())
		{
			/* Read Generic Report Data */
			Endpoint_Read_Stream_LE(&GenericData, sizeof(GenericData), NULL);
			dataRead = true;
		}

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearOUT();

		if (dataRead)
		{
			/* Process Generic Report Data */
			ProcessGenericHIDReport(GenericData);
		}
	}
}
