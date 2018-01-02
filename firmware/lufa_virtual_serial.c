
#include "descriptors.h"
#include "lufa_virtual_serial.h"
#include <LUFA/Platform/Platform.h>
#include <LUFA/Drivers/USB/USB.h>

/*******************************************************************************
 * VIRTUAL SERIAL
 ******************************************************************************/

//#define VIRTUAL_SERIAL_STDOUT_ONLY

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;
FILE *_vsf;

extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

void virtser_init(void)
{
	VirtualSerial_CDC_Interface.State.ControlLineStates.DeviceToHost = CDC_CONTROL_LINE_IN_DSR;
    CDC_Device_SendControlLineStateChange(&VirtualSerial_CDC_Interface);

#ifdef VIRTUAL_SERIAL_STDOUT_ONLY
    //fdev_setup_stream(&USBSerialStream, virtser_putchar, NULL, _FDEV_SETUP_WRITE);
    USBSerialStream = (FILE)FDEV_SETUP_STREAM(virtser_putchar, NULL, _FDEV_SETUP_WRITE);
#else
	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	//CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
#endif
    _vsf = &USBSerialStream;
}

void virtser_recv(uint8_t c) __attribute__((weak));
void virtser_recv(uint8_t c)
{
    // Echoed by default
#ifdef VIRTUAL_SERIAL_STDOUT_ONLY
	virtser_putchar(c, 0);
#else
	fputc(c, &USBSerialStream);
#endif
}

void virtser_recv_task(void)
{
#ifdef VIRTUAL_SERIAL_STDOUT_ONLY
	uint8_t ch;
    uint16_t count = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);

    if (count)
    {
        ch = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
        virtser_recv(ch);
    }
#else
	int c = fgetc(&USBSerialStream);
	if (c != EOF)
	{
		virtser_recv(c);
	}
#endif
}

#ifdef VIRTUAL_SERIAL_STDOUT_ONLY
int virtser_putchar(char c, FILE *unused)
{
    uint8_t timeout = 255;
    uint8_t ep = Endpoint_GetCurrentEndpoint();

    if (VirtualSerial_CDC_Interface.State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR)
    {
        /* IN packet */
        Endpoint_SelectEndpoint(VirtualSerial_CDC_Interface.Config.DataINEndpoint.Address);

        if (!Endpoint_IsEnabled() || !Endpoint_IsConfigured())
        {
            Endpoint_SelectEndpoint(ep);
            return 1;
        }

        while (timeout-- && !Endpoint_IsReadWriteAllowed())
            _delay_us(40);

        Endpoint_Write_8(c);
        CDC_Device_Flush(&VirtualSerial_CDC_Interface);

        if (Endpoint_IsINReady())
        {
            Endpoint_ClearIN();
        }

        Endpoint_SelectEndpoint(ep);
        return 1;
    }

    return 0;
}
#endif
