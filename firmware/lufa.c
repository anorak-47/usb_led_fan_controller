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

#include "lufa.h"
#include "lufa_virtual_serial.h"
#include "led_control.h"
#include "fan.h"
#include "power.h"
#include "i2cmaster.h"
#include "usb_generic_hid.h"

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

#ifndef USB_GENERIC_HID_RAW_SUPPORTED
/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevHIDReportBuffer[GENERIC_REPORT_SIZE];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Generic_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_GenericHID,
				.ReportINEndpoint             =
					{
						.Address              = GENERIC_IN_EPADDR,
						.Size                 = GENERIC_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevHIDReportBuffer),
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

	for (;;)
	{
		led_control_task();
		fanControlUpdate();

#ifdef USB_VIRTUAL_SERIAL_SUPPORTED
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		virtser_recv_task();
#endif

#ifdef USB_GENERIC_HID_RAW_SUPPORTED
		HID_Task();
#else
		HID_Device_USBTask(&Generic_HID_Interface);
#endif

#if !defined(INTERRUPT_CONTROL_ENDPOINT)
		USB_USBTask();
#endif
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	i2c_init();
	powermeter_init();
	led_control_setup();
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

#ifdef USB_GENERIC_HID_RAW_SUPPORTED
	/* Setup HID Report Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
#else
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Generic_HID_Interface);
	USB_Device_EnableSOFEvents();
#endif

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
#if defined(USB_GENERIC_HID_RAW_SUPPORTED)
#if 0
	/* Handle HID Class specific requests */
	switch (USB_ControlRequest.bRequest)
	{
		case HID_REQ_GetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
#if 0
				uint8_t GenericData[GENERIC_REPORT_SIZE];
				CreateGenericHIDReport(GenericData);

				Endpoint_ClearSETUP();

				/* Write the report data to the control endpoint */
				Endpoint_Write_Control_Stream_LE(&GenericData, sizeof(GenericData));
				Endpoint_ClearOUT();
#else
				Endpoint_ClearSETUP();

				/* Write the report data to the control endpoint */
				Endpoint_Write_Control_Stream_LE(0, 0);
				Endpoint_ClearOUT();
#endif
			}

			break;
		case HID_REQ_SetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
#if 0
				uint8_t GenericData[GENERIC_REPORT_SIZE];

				Endpoint_ClearSETUP();

				/* Read the report data from the control endpoint */
				Endpoint_Read_Control_Stream_LE(&GenericData, sizeof(GenericData));
				Endpoint_ClearIN();

				ProcessGenericHIDReport(GenericData);
#else
				Endpoint_ClearSETUP();

				/* Read the report data from the control endpoint */
				Endpoint_Read_Control_Stream_LE(0, 0);
				Endpoint_ClearIN();
#endif
			}
			break;
	}
#endif
#else
	HID_Device_ProcessControlRequest(&Generic_HID_Interface);
#endif

#ifdef USB_VIRTUAL_SERIAL_SUPPORTED
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
#endif
}

#ifndef USB_GENERIC_HID_RAW_SUPPORTED
/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Generic_HID_Interface);
}
#endif

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

#ifdef USB_GENERIC_HID_RAW_SUPPORTED
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
#else
/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	uint8_t* Data        = (uint8_t*)ReportData;
	uint8_t  CurrLEDMask = LEDs_GetLEDs();

	Data[0] = ((CurrLEDMask & LEDS_LED1) ? 1 : 0);
	Data[1] = ((CurrLEDMask & LEDS_LED2) ? 1 : 0);
	Data[2] = ((CurrLEDMask & LEDS_LED3) ? 1 : 0);
	Data[3] = ((CurrLEDMask & LEDS_LED4) ? 1 : 0);

	*ReportSize = GENERIC_REPORT_SIZE;
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	uint8_t* Data       = (uint8_t*)ReportData;
	uint8_t  NewLEDMask = LEDS_NO_LEDS;

	if (Data[0])
	  NewLEDMask |= LEDS_LED1;

	if (Data[1])
	  NewLEDMask |= LEDS_LED2;

	if (Data[2])
	  NewLEDMask |= LEDS_LED3;

	if (Data[3])
	  NewLEDMask |= LEDS_LED4;

	LEDs_SetAllLEDs(NewLEDMask);
}
#endif
