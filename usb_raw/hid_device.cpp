
#include "hid_device.h"

#include <stdio.h>
#include <stdlib.h>

#include <cstring>
#include <iostream>
#include <map>

#include <QtCore/QDebug>

// Headers needed for sleeping.
#ifdef _WIN32
#include "hidapi.h"
#include <windows.h>
#else
#include <hidapi/hidapi.h>
#include <unistd.h>
#endif

#ifdef _MSC_VER
// Stop compiler bitching about sscanf() being unsafe.
// Prefer the portable sscanf() to the suggested, MSVC-only sscanf_c()
#pragma warning(disable : 4996)
#endif

#include "config.h"

hid_device *hid_open(unsigned short vendor_id, unsigned short product_id, unsigned short interface_number)
{
    qDebug("hid_open: %x:%x:%x\n", vendor_id, product_id, interface_number);

    hid_device *device = NULL;
    struct hid_device_info *deviceInfos;
    struct hid_device_info *currentDeviceInfo;
    struct hid_device_info *foundDeviceInfo = NULL;
    deviceInfos = hid_enumerate(vendor_id, product_id);
    currentDeviceInfo = deviceInfos;
    while (currentDeviceInfo)
    {
        if (currentDeviceInfo->interface_number == interface_number)
        {
            if (foundDeviceInfo)
            {
                // More than one matching device.
                // TODO: return error?
            }
            else
            {
                foundDeviceInfo = currentDeviceInfo;
            }
        }
        currentDeviceInfo = currentDeviceInfo->next;
    }

    if (foundDeviceInfo)
    {
        device = hid_open_path(foundDeviceInfo->path);
    }

    qDebug("hid_open: device is open now: %d\n", (device != 0));

    hid_free_enumeration(deviceInfos);

    return device;
}

std::vector<std::string> hid_get_device_paths(unsigned short vendor_id, unsigned short product_id, unsigned short interface_number)
{
    qDebug("hid_get_device_paths: filter: %x:%x:%x\n", vendor_id, product_id, interface_number);

    std::vector<std::string> devicePaths;
    struct hid_device_info *deviceInfos;
    struct hid_device_info *currentDeviceInfo;
    //struct hid_device_info *foundDeviceInfo = NULL;
    deviceInfos = hid_enumerate(vendor_id, product_id);
    currentDeviceInfo = deviceInfos;
    while (currentDeviceInfo)
    {
        if (currentDeviceInfo->interface_number == interface_number)
        {
            qDebug("hid_get_device_paths: found a device at path: %s\n", currentDeviceInfo->path);
            qDebug() << "hid_get_device_paths: product: " << QString::fromWCharArray(currentDeviceInfo->product_string);
            qDebug() << "hid_get_device_paths: serial: " << QString::fromWCharArray(currentDeviceInfo->serial_number);
            qDebug("hid_get_device_paths: release: %X\n", currentDeviceInfo->release_number);
            devicePaths.push_back(currentDeviceInfo->path);
        }
        currentDeviceInfo = currentDeviceInfo->next;
    }

    hid_free_enumeration(deviceInfos);

    qDebug("hid_get_device_paths: devices found: %lu\n", devicePaths.size());

    return devicePaths;
}

bool hid_send_message(hid_device *device, uint8_t id, void *outMsg, uint8_t outMsgLength, void *retMsg, uint8_t retMsgLength)
{
    // assert( outMsgLength <= RAW_HID_BUFFER_SIZE );
    if (outMsgLength > RAW_HID_BUFFER_SIZE)
    {
        qDebug("Message size %d is bigger than maximum %d\n", outMsgLength, RAW_HID_BUFFER_SIZE);
        return false;
    }

    if (retMsgLength > RAW_HID_BUFFER_SIZE - 1)
    {
        qDebug("Requested Message size %d is bigger than maximum %d\n", retMsgLength, RAW_HID_BUFFER_SIZE - 1);
        return false;
    }

    int res;
    uint8_t data[RAW_HID_BUFFER_SIZE + 1];
    memset(data, 0xFE, sizeof(data));
    data[0] = 0x00; // NULL report ID. IMPORTANT!
    data[1] = id;

    if (outMsg && outMsgLength > 0)
    {
        memcpy(&data[2], outMsg, outMsgLength);
    }

    //qDebug("hid_send_message: request: %u: %x %x %x %x %x %x %x %x %x\n", id, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[7]);

    res = 0;
    res = hid_write(device, data, RAW_HID_BUFFER_SIZE + 1);
    if (res < 0)
    {
        qDebug("hid_send_message: Unable to hid_write()\n");
        qDebug("hid_send_message: Error: %ls\n", hid_error(device));
        return false;
    }

    hid_set_nonblocking(device, 1);

    res = 0;
    // Timeout after 500ms
    for (int i = 0; i < 500; i++)
    {
        res = hid_read(device, data, RAW_HID_BUFFER_SIZE);
        if (res != 0)
        {
            break;
        }
        // waiting
#ifdef WIN32
        Sleep(1);
#else
        usleep(1 * 1000);
#endif
    }

    //qDebug("hid_send_message: hid_read returned: %d\n", res);

    if (res < 0)
    {
        qDebug("hid_send_message: Unable to hid_read()\n");
        qDebug("hid_send_message: Error: %ls\n", hid_error(device));
        return false;
    }

    if (res > 0)
    {
        //qDebug("hid_send_message: response: %x %x %x %x %x %x %x %x\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

        if (retMsg && retMsgLength > 0)
        {
            // data[0] contains the request id
            //qDebug("hid_send_message: copy response, %d\n", retMsgLength);
            memcpy(retMsg, data, retMsgLength);
        }
    }

    return true;
}

hid_device *hid_open_device_by_path(std::string path)
{
    if (path.empty())
        return 0;
    qDebug("hid_open_device_by_path: opening path: %s\n", path.c_str());
    hid_device *device = hid_open_path(path.c_str());
    if (device)
        hid_set_nonblocking(device, 1);
    return device;
}

hid_device *hid_open_first_found(unsigned short vendor_id, unsigned short product_id, unsigned short interface_number)
{
    std::vector<std::string> devicePaths = hid_get_device_paths(vendor_id, product_id, interface_number);

    if (devicePaths.size() >= 1)
    {
        qDebug("hid_open_first_found: opening: %s\n", devicePaths[0].c_str());
        hid_device *device = hid_open_path(devicePaths[0].c_str());
        if (device)
            hid_set_nonblocking(device, 1);
        return device;
    }

    return NULL;
}

#if 0


/*
bool protocol_version(hid_device *device, msg_protocol_version *msg)
{
    return send_message(device, id_protocol_version, msg, sizeof(msg_protocol_version), msg, sizeof(msg_protocol_version));
}
*/

bool send_led_pattern(hid_device *device, uint8_t led1, uint8_t led2, uint8_t led3, uint8_t led4)
{
    uint8_t msg[7];

    // Report data for the demo is the report ID (always zero) followed by the
    // LED on/off data
    // report_data = [0, led1, led2, led3, led4]

    msg[0] = led2;
    msg[1] = led3;
    msg[2] = led4;

    qDebug("send led pattern: %u %u %u %u\n", led1, led2, led3, led4);
    if (hid_send_message(device, msg[0], msg, 7, msg, 7))
    {
        qDebug("got led pattern: %u %u %u %u\n", msg[0], msg[1], msg[2], msg[3]);
        return true;
    }

    return false;
}

bool request_echo(hid_device *device, uint8_t led1, uint8_t led2, uint8_t led3, uint8_t led4)
{
    uint8_t msg[7];

    msg[0] = 0xdd; // channel
    msg[1] = led1;
    msg[2] = led2;
    msg[3] = led3;
    msg[4] = led3;

    qDebug("request_echo %x %x %x %x\n", msg[1], msg[2], msg[3], msg[4]);

	if (hid_send_message(device, 0, msg, 3, msg, 7))
	    {
            qDebug("got echo: %x %x %x %x %x\n", msg[0], msg[1], msg[2], msg[3], msg[4]);
	        return true;
	    }

	    return false;

}

int main(int argc, char **argv)
{
    if (hid_init())
    {
        std::cerr << "*** Error: hidapi initialization failed" << std::endl;
        return -1;
    }

    hid_device *device = hid_open_first_found(DEVICE_VID, DEVICE_PID, DEVICE_INTERFACE_NUMBER);
    if (!device)
    {
        std::cerr << "*** Error: Device not found" << std::endl;
        return -1;
    }

    uint8_t p = 0;
    while (1)
    {
    	request_echo(device, (p >> 3) & 1, (p >> 2) & 1, (p >> 1) & 1, (p >> 0) & 1);
        p = (p + 1) % 16;

// Delay a bit for visual effect
#ifdef WIN32
        Sleep(1);
#else
        usleep(1 * 100 * 1000);
#endif
    }

    usleep(1 * 1000 * 1000);

    /*

    send_led_pattern(device, 1, 0, 0, 0);

    uint8_t p = 0;
    while (true)
    {
        send_led_pattern(device, (p >> 3) & 1, (p >> 2) & 1, (p >> 1) & 1, (p >> 0) & 1);
        // Compute next LED pattern in sequence
        p = (p + 1) % 16;

// Delay a bit for visual effect
#ifdef WIN32
        Sleep(1);
#else
        usleep(1 * 1000 * 1000);
#endif
    }

    */

    /*
    bool res = false;
    msg_protocol_version msg;
    if (res == protocol_version(device, &msg))
    {
        std::cerr << "*** Error: Error getting protocol version" << std::endl;
        hid_close(device);
        return -1;
    }
    */

    qDebug("hid_close\n");
    hid_close(device);
    hid_exit();
    return 0;
}

#endif

