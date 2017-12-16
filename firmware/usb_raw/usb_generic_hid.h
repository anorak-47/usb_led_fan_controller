#ifndef __USB_GENEIRC_HID_H_INCLUDED__
#define __USB_GENEIRC_HID_H_INCLUDED__

/*
        This is where you need to process reports sent from the host to the device. This
        function is called each time the host has sent a new report. DataArray is an array
        holding the report sent from the host.
*/
void ProcessGenericHIDReport(uint8_t *DataArray);

void SendGenericHIDReport(uint8_t* data, uint8_t length);

#endif
