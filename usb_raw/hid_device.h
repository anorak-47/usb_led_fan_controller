#pragma once
#include <stdint.h>
#include <vector>
#include <string>

#ifdef _WIN32
#include "hidapi.h"
#include <windows.h>
#else
#include <hidapi/hidapi.h>
#include <unistd.h>
#endif

#define DEVICE_VID 0x1209
#define DEVICE_PID 0xfa6c
#define DEVICE_INTERFACE_NUMBER 0x00

#define RAW_HID_BUFFER_SIZE 8

bool hid_send_message(hid_device *device, uint8_t id, void *outMsg = NULL, uint8_t outMsgLength = 0, void *retMsg = NULL, uint8_t retMsgLength = 0);

hid_device *hid_open_device_by_path(std::string path);
hid_device *hid_open_first_found(unsigned short vendor_id, unsigned short product_id, unsigned short interface_number);
hid_device *hid_open(unsigned short vendor_id, unsigned short product_id, unsigned short interface_number);

std::vector<std::string> hid_get_device_paths(unsigned short vendor_id, unsigned short product_id, unsigned short interface_number);
