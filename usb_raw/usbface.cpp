/*
    USB Fan Controller - Flexible PWM Fan Controller Library

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

#include "usbface.h"
#include "hid_device.h"
#include "requests.h" // from firmware
#include <QtCore/QDebug>
#include <string.h>   // malloc, free
#include <assert.h>   // assert

#define TIMEOUT 5000

#define DATASIZE (sizeof(unsigned char) + sizeof(int))

int usbRequest(hid_device *device, const CUSTOM_RQ req, const int value, const int channel, const int len, int *val)
{
    int respMsgLen = len + 1;
    uint8_t *buff = (uint8_t *)malloc(respMsgLen);
    int ret = USBFACE_SUCCESS;
    bool read;
    assert(device != NULL);

    uint8_t data[DATASIZE];
    memset(data, 0xFE, sizeof(data));

    data[0] = channel;
    int *vp = (int*)&data[1];
    *vp = value;

    //qDebug() << "usbRequest: req: " << req << ", channel: " << index << ", value: " << value << ", len: " << len;

    read = hid_send_message(device, req, data, DATASIZE, buff, respMsgLen);

    if (val)
        *val = 0;

    if (!read)
    {
        ret = USBFACE_ERR_IO;
        qDebug() << "usbRequest: req: " << req << ", channel: " << channel << ", value: " << value << ", len: " << len;
        qDebug() << "usbRequest: USBFACE_ERR_IO";
    }
    else if (respMsgLen > 0 && buff[0] == CUSTOM_RQ_UNSUPPORTED)
    {
        ret = USBFACE_ERR_UNSUPP;
        qDebug() << "usbRequest: req: " << req << ", channel: " << channel << ", value: " << value << ", len: " << len;
        qDebug() << "usbRequest: USBFACE_ERR_UNSUPP";
    }
    else if (respMsgLen > 0 && buff[0] != req)
    {
        ret = USBFACE_ERR_READ;
        qDebug() << "usbRequest: req: " << req << ", channel: " << channel << ", value: " << value << ", len: " << len;
        qDebug() << "usbRequest: res: " << buff[0] << ", respMsgLen: " << respMsgLen;
        qDebug() << "usbRequest: USBFACE_ERR_READ";
    }
    else
    {
        if (val)
        {
            switch (len)
            {
            case 0:
                break; // nothing to read
            case 1:
                *val = buff[1];
                break;
            case 2:
                *val = *((unsigned short *)&buff[1]);
                break;
            case 4:
                *val = *((unsigned int *)&buff[1]);
                break;
            default:
                ret = USBFACE_ERR_READ;
                break;
            }
        }
    }

    free(buff);
    return ret;
}

int usbRequestData(hid_device *device, const CUSTOM_RQ req, const int channel, const int input_len, const unsigned char *input_data, const int output_len, unsigned char *output_data)
{
    assert(device != NULL);
    //assert(input_len <= DATASIZE);
    //qDebug() << "usbRequestData: req: " << req << ", channel: " << channel << ", input_len: " << input_len << ", output_len: " << output_len;

    int respMsgLen = output_len + 1;
    uint8_t *buff = (uint8_t *)malloc(respMsgLen);
    int ret = USBFACE_SUCCESS;
    bool read;

    uint8_t data[DATASIZE];
    memset(data, 0xFE, sizeof(data));

    data[0] = channel;

    if (input_len)
        memcpy(&data[1], input_data, input_len);

    read = hid_send_message(device, req, data, DATASIZE, buff, respMsgLen);

    if (!read)
    {
        ret = USBFACE_ERR_IO;
        qDebug() << "usbRequestData: req: " << req << ", channel: " << channel << ", input_len: " << input_len << ", output_len: " << output_len;
        qDebug() << "usbRequestData: USBFACE_ERR_IO";
    }
    else if (respMsgLen > 0 && buff[0] == CUSTOM_RQ_UNSUPPORTED)
    {
        ret = USBFACE_ERR_UNSUPP;
        qDebug() << "usbRequestData: req: " << req << ", channel: " << channel << ", input_len: " << input_len << ", output_len: " << output_len;
        qDebug() << "usbRequestData: USBFACE_ERR_UNSUPP";
    }
    else if (respMsgLen > 0 && buff[0] != req)
    {
        ret = USBFACE_ERR_READ;
        qDebug() << "usbRequestData: req: " << req << ", channel: " << channel << ", input_len: " << input_len << ", output_len: " << output_len;
        qDebug() << "usbRequestData: res: " << buff[0] << ", respMsgLen: " << respMsgLen;
        qDebug() << "usbRequestData: USBFACE_ERR_READ";
    }
    else
    {
        memcpy(output_data, &buff[1], output_len);
    }

    return ret;
}

// ---- Interface open/close/test ---------------------------------------------

int usbfacePing(hid_device *device)
{
    int check = 0xAAAA5555;
    int read;
    int res = usbRequest(device, CUSTOM_RQ_ECHO, check >> 16, check & 0xffff, 4, &read);
    if (res == USBFACE_SUCCESS && read != check)
        return USBFACE_ERR_READ;
    return res;
}

int usbfaceProtocolVersion(hid_device *device, unsigned char *version)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_PROTOCOL_VERSION, 0, 0, 1, &read);
    *version = read;
    return res;
}

int usbfaceFirmwareVersion(hid_device *device, unsigned char *major, unsigned char *minor)
{
    unsigned char read[2];
    int res = usbRequest(device, CUSTOM_RQ_FIRMWARE_VERSION, 0, 0, 2, (int*)&read[0]);
    *major = read[0];
    *minor = read[1];
    return res;
}

int usbfaceDeviceNameRead(hid_device *device, char *name)
{
    unsigned char output_data[21];

    int res;
    res = usbRequestData(device, CUSTOM_RQ_DEVICE_NAME_READ, 0, 0, 0, 7, output_data);
    res = usbRequestData(device, CUSTOM_RQ_DEVICE_NAME_READ, 0, 0, 0, 7, output_data+7);
    res = usbRequestData(device, CUSTOM_RQ_DEVICE_NAME_READ, 0, 0, 0, 6, output_data+7);

    output_data[20] = '\0';
    strncpy(name, (char*)output_data, 21);

    return res;
}

int usbfaceDeviceNameWrite(hid_device *device, char const *name)
{
    int res;
    unsigned char *string = (unsigned char*)name;
    res = usbRequestData(device, CUSTOM_RQ_DEVICE_NAME_WRITE, 0, 7, string, 0, 0);
    res = usbRequestData(device, CUSTOM_RQ_DEVICE_NAME_WRITE, 7, 7, string+7, 0, 0);
    res = usbRequestData(device, CUSTOM_RQ_DEVICE_NAME_WRITE, 14, 6, string+7, 0, 0);
    return res;
}

int usbfaceFuncsSupportedRead(hid_device *device, SUPPORTED *funcs)
{
    int res = USBFACE_SUCCESS;
    int read;
    static hid_device *cached_device = NULL;
    static SUPPORTED cached_funcs = (SUPPORTED)0;
    // Cache the supported functions until the device handle changes (device gets reconnected),
    // as this function might get called a lot of times.
    if (cached_device != device)
    {
        if ((res = usbRequest(device, CUSTOM_RQ_FUNCS_SUPPORTED, 0, 0, 2, &read)) == USBFACE_SUCCESS)
        {
            cached_funcs = (SUPPORTED)read;
            cached_device = device;
        }
    }
    *funcs = cached_funcs;
    return res;
}

int usbfaceGetNrOfFans()
{
    return MAX_FANS;
}

int usbfaceGetNrOfFanOuts()
{
    return MAX_FAN_OUTS;
}

int usbfaceGetNrOfSensors()
{
    return MAX_SNS;
}

int usbfaceFanRpsRead(hid_device *device, const unsigned char channel, double *rpm)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANRPS_READ, 0, channel, 2, &read);
    *rpm = 60.0 * (((double)(read >> 8)) + ((double)(read & 0xff)) / 256.0);
    return res;
}

int usbfaceFanRpmRead(hid_device *device, const unsigned char channel, unsigned int *rpm)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANRPM_READ, 0, channel, 2, &read);
    *rpm = (unsigned int)read;
    return res;
}

int usbfaceFanDutyRead(hid_device *device, const unsigned char channel, double *duty)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANDUTY_READ, 0, channel, 1, &read);    
    // Buffer returned contains single byte indicating duty cycle; [0..255] = [0%..100%]
    *duty = ((double)(unsigned char)read) / 255.0;
    return res;
}

int usbfaceFanDutyRawRead(hid_device *device, const unsigned char channel, unsigned char *duty)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANDUTY_READ, 0, channel, 1, &read);
    *duty = (unsigned char)read;
    return res;
}

int usbfaceFanDutyFixedWrite(hid_device *device, const unsigned char channel, const double duty)
{
    return usbRequest(device, CUSTOM_RQ_FANDUTYFIXED_WRITE, (unsigned char)(255.0 * (double)duty), channel, 0, NULL);
}

int usbfaceFanDutyFixedRawWrite(hid_device *device, const unsigned char channel, const unsigned char duty)
{    
    return usbRequest(device, CUSTOM_RQ_FANDUTYFIXED_WRITE, duty, channel, 0, NULL);
}

int usbfaceFanDutyFixedRead(hid_device *device, const unsigned char channel, double *duty)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANDUTYFIXED_READ, 0, channel, 1, &read);
    // Buffer returned contains single byte indicating duty cycle; [0..255] = [0%..100%]
    *duty = ((double)(unsigned char)read) / 255.0;
    return res;
}

int usbfaceFanDutyFixedRawRead(hid_device *device, const unsigned char channel, unsigned char *duty)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANDUTYFIXED_READ, 0, channel, 1, &read);
    // Buffer returned contains single byte indicating duty cycle; [0..255] = [0%..100%]
    *duty = (unsigned char)read;
    return res;
}

int usbfaceFanTypeWrite(hid_device *device, const unsigned char channel, const FANTYPE type)
{
    return usbRequest(device, CUSTOM_RQ_FANTYPE_WRITE, (int)type, channel, 0, NULL);
}

int usbfaceFanTypeRead(hid_device *device, const unsigned char channel, FANTYPE *type)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANTYPE_READ, 0, channel, 1, &read);
    *type = (FANTYPE)read;
    return res;
}

int usbfaceFanModeWrite(hid_device *device, const unsigned char channel, const FANMODE mode)
{
    return usbRequest(device, CUSTOM_RQ_FANMODE_WRITE, (int)mode, channel, 0, NULL);
}

int usbfaceFanModeRead(hid_device *device, const unsigned char channel, FANMODE *mode)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANMODE_READ, 0, channel, 1, &read);
    *mode = (FANMODE)read;
    return res;
}

int usbfaceFanMinRpmWrite(hid_device *device, const unsigned char channel, const double minrpm)
{
    // Value should contain revolutions/sec -> convert from rpm to rps
    return usbRequest(device, CUSTOM_RQ_FANMINRPS_WRITE, (int)(0.5 + minrpm / 60.0), channel, 0, NULL);
}

int usbfaceFanMinRpmRead(hid_device *device, const unsigned char channel, double *minrpm)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANMINRPS_READ, 0, channel, 1, &read);
    // Convert value returned from rps to rpm
    *minrpm = 60.0 * (double)read;
    return res;
}

int usbfaceFanSetpointWrite(hid_device *device, const unsigned char channel, const double setpoint,
                            const int refsnsidx)
{
    int res = usbRequest(device, CUSTOM_RQ_FANSETP_DELTA_WRITE, (unsigned char)setpoint, channel, 0, NULL);
    if (res == USBFACE_SUCCESS)
        res = usbRequest(device, CUSTOM_RQ_FANSETP_REFSNS_WRITE, refsnsidx, channel, 0, NULL);
    return res;
}

int usbfaceFanSetpointActualRead(hid_device *device, const unsigned char channel, double *setpoint)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANSETP_READ, 0, channel, 1, &read);
    *setpoint = (double)read;
    return res;
}

int usbfaceFanSetpointRead(hid_device *device, const unsigned char channel, double *setpoint, int *refsnsidx)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANSETP_DELTA_READ, 0, channel, 1, &read);
    *setpoint = (double)read;
    if (res == USBFACE_SUCCESS)
    {
        res = usbRequest(device, CUSTOM_RQ_FANSETP_REFSNS_READ, 0, channel, 1, &read);
        *refsnsidx = (SNSTYPE)read;
    }
    return res;
}

int usbfaceFanPidWrite(hid_device *device, const unsigned char channel, const double kp, const double ki,
                       const double kt)
{
    int res = usbRequest(device, CUSTOM_RQ_FANKP_WRITE, (int)(kp + 0.5), channel, 0, NULL);
    if (res == USBFACE_SUCCESS)
        res = usbRequest(device, CUSTOM_RQ_FANKI_WRITE, (int)(ki + 0.5), channel, 0, NULL);
    if (res == USBFACE_SUCCESS)
        res = usbRequest(device, CUSTOM_RQ_FANKT_WRITE, (int)(kt + 0.5), channel, 0, NULL);
    return res;
}

int usbfaceFanPidRead(hid_device *device, const unsigned char channel, double *kp, double *ki, double *kt)
{
    int read, res;
    *kp = 0.0;
    *ki = 0.0;
    *kt = 0.0;
    if ((res = usbRequest(device, CUSTOM_RQ_FANKP_READ, 0, channel, 1, &read)) != USBFACE_SUCCESS)
        return res;
    *kp = (double)read;
    if ((res = usbRequest(device, CUSTOM_RQ_FANKI_READ, 0, channel, 1, &read)) != USBFACE_SUCCESS)
        return res;
    *ki = (double)read;
    if ((res = usbRequest(device, CUSTOM_RQ_FANKT_READ, 0, channel, 1, &read)) != USBFACE_SUCCESS)
        return res;
    *kt = (double)read;
    return res;
}

int usbfaceFanGainOffsWrite(hid_device *device, const unsigned char channel, const double gain, const double offs)
{
    // Gain & offset are multiplied by 256/100, because one expects a gain of 1 and offset 0 to result in the same
    // duty cycle value as the sensor input (e.g. 24 degrees results in 24% PWM)
    int res = usbRequest(device, CUSTOM_RQ_FANGAIN_WRITE, (int)(gain * 64.0 * (256.0 / 100.0) + 0.5), channel, 0,
                         NULL); // times 64, as lower 6 bits contain the fraction
    if (res == USBFACE_SUCCESS)
        res = usbRequest(device, CUSTOM_RQ_FANOFFS_WRITE, (int)(offs * (256.0 / 100.0) + 0.5), channel, 0, NULL);
    return res;
}

int usbfaceFanGainOffsRead(hid_device *device, const unsigned char channel, double *gain, double *offs)
{
    // Gain & offset are divided by 256/100, because one expects a gain of 1 and offset 0 to result in the same
    // duty cycle value as the sensor input (e.g. 24 degrees results in 24% PWM)
    int read, res;
    *gain = 0.0;
    *offs = 0.0;
    if ((res = usbRequest(device, CUSTOM_RQ_FANGAIN_READ, 0, channel, 2, &read)) != USBFACE_SUCCESS)
        return res;
    *gain = (((double)read) / 64.0) / (256.0 / 100.0); // divide by 64, as lower 6 bits contain the fraction
    if ((res = usbRequest(device, CUSTOM_RQ_FANOFFS_READ, 0, channel, 2, &read)) != USBFACE_SUCCESS)
        return res;
    *offs = ((double)read) / (256.0 / 100.0);
    return res;
}

int usbfaceFanTripPointRead(hid_device *device, const unsigned char channel, const unsigned int point, unsigned int *value, unsigned int *duty)
{
    unsigned char read[2];
	int res;
    if ((res = usbRequest(device, CUSTOM_RQ_FANTRIPPOINT_READ, point, channel, 2, (int*)&read[0])) != USBFACE_SUCCESS)
	        return res;
	*value = read[0];
	*duty = read[1];
	return res;
}

int usbfaceFanTripPointWrite(hid_device *device, const unsigned char channel, const unsigned int point, unsigned int value, unsigned int duty)
{
    unsigned char data[3];
    data[0] = point;
    data[1] = value;
    data[2] = duty;
    int res = usbRequestData(device, CUSTOM_RQ_FANTRIPPOINT_WRITE, channel, 3, data, 0, NULL);
	return res;
}

int usbfaceFanSensorWrite(hid_device *device, const unsigned char channel, const int snsidx)
{
    return usbRequest(device, CUSTOM_RQ_FANSNS_WRITE, snsidx, channel, 0, NULL);
}

int usbfaceFanSensorRead(hid_device *device, const unsigned char channel, int *snsidx)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANSNS_READ, 0, channel, 1, &read);
    *snsidx = read;
    return res;
}

int usbfaceFanStallRead(hid_device *device, const unsigned char channel, int *stalled)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANSTALL_READ, 0, channel, 1, &read);
    *stalled = read;
    return res;
}

int usbfaceFanStallDetectWrite(hid_device *device, const unsigned char channel, const int stalldetect)
{
    return usbRequest(device, CUSTOM_RQ_FANSTALLDETECT_WRITE, stalldetect != 0, channel, 0, NULL);
}

int usbfaceFanStallDetectRead(hid_device *device, const unsigned char channel, int *stalldetect)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANSTALLDETECT_READ, 0, channel, 1, &read);
    *stalldetect = read != 0;
    return res;
}

int usbfaceSnsTypeWrite(hid_device *device, const unsigned char channel, const SNSTYPE type)
{
    return usbRequest(device, CUSTOM_RQ_SNSTYPE_WRITE, (int)type, channel, 0, NULL);
}

int usbfaceSnsTypeRead(hid_device *device, const unsigned char channel, SNSTYPE *type)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_SNSTYPE_READ, 0, channel, 1, &read);
    *type = (SNSTYPE)read;
    return res;
}

int usbfaceSnsTypeIsSupportedByFunctions(int funcs, const SNSTYPE type)
{
    switch (type)
    {
    case SNSTYPE_NONE:
    case SNSTYPE_EXT0:
    case SNSTYPE_EXT1:
    case SNSTYPE_EXT2:
    case SNSTYPE_EXT3:
    case SNSTYPE_RPS_FAN0:
    case SNSTYPE_RPS_FAN1:
    case SNSTYPE_RPS_FAN2:
    case SNSTYPE_RPS_FAN3:
    case SNSTYPE_RPS_FAN4:
    case SNSTYPE_RPS_FAN5:
        return 1;
    case SNSTYPE_TEMP_NTC0:
    case SNSTYPE_TEMP_NTC1:
    case SNSTYPE_TEMP_NTC2:
    case SNSTYPE_TEMP_NTC3:
    case SNSTYPE_TEMP_NTC4:
    case SNSTYPE_TEMP_NTC5:
        return funcs & SUPPORTED_SNS_ANALOG;
    case SNSTYPE_TEMP_I2C_ADDR0:
    case SNSTYPE_TEMP_I2C_ADDR1:
    case SNSTYPE_TEMP_I2C_ADDR2:
    case SNSTYPE_TEMP_I2C_ADDR3:
    case SNSTYPE_TEMP_I2C_ADDR4:
    case SNSTYPE_TEMP_I2C_ADDR5:
    case SNSTYPE_TEMP_I2C_ADDR6:
    case SNSTYPE_TEMP_I2C_ADDR7:
        return funcs & SUPPORTED_SNS_I2C;
    case SNSTYPE_DUTY_IN0:
    case SNSTYPE_DUTY_IN1:
        return funcs & SUPPORTED_FAN_OUT;
    case SNSTYPE_POWER0:
    case SNSTYPE_POWER1:
        return funcs & SUPPORTED_SNS_POWER;
    default:
        return 0;
    }
}

int usbfaceSnsTypeIsSupported(hid_device *device, const SNSTYPE type)
{
    SUPPORTED funcs;
    if (USBFACE_SUCCESS != usbfaceFuncsSupportedRead(device, &funcs))
    {
        return 0;
    }

    return usbfaceSnsTypeIsSupportedByFunctions(funcs, type);
}

int usbfaceSnsWrite(hid_device *device, const unsigned char channel, const double snsvalue)
{
    return usbRequest(device, CUSTOM_RQ_SNS_WRITE, (unsigned char)(snsvalue + 0.5), channel, 0, NULL);
}

int usbfaceSnsRead(hid_device *device, const unsigned char channel, double *snsvalue)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_SNS_READ, 0, channel, 1, &read);
    *snsvalue = (double)read;
    return res;
}

int usbfaceFanOutModeWrite(hid_device *device, const unsigned char channel, const FANOUTMODE mode)
{
    return usbRequest(device, CUSTOM_RQ_FANOUTMODE_WRITE, (int)mode, channel, 0, NULL);
}

int usbfaceFanOutModeRead(hid_device *device, const unsigned char channel, FANOUTMODE *mode)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANOUTMODE_READ, 0, channel, 1, &read);
    *mode = (FANOUTMODE)read;
    return res;
}

int usbfaceFanOutModeIsSupported(hid_device *device, const FANOUTMODE mode)
{
    Q_UNUSED(mode);

    SUPPORTED funcs;
    if (USBFACE_SUCCESS != usbfaceFuncsSupportedRead(device, &funcs))
    {
        return 0;
    }
    // It's all or nothing; doesn't depend on the mode passed.
    return funcs && SUPPORTED_FAN_OUT;
}

int usbfaceFanOutRpmRead(hid_device *device, const unsigned char channel, double *rpm)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FANOUTRPM_READ, 0, (int)channel, 2, &read);
    *rpm = (double)read;
    return res;
}

int usbfaceFastledAnimationIdRead(hid_device *device, const unsigned char channel, unsigned char *id)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FASTLEDANIID_READ, 0, (int)channel, 1, &read);
    *id = read;
    return res;
}

int usbfaceFastledAnimationIdWrite(hid_device *device, const unsigned char channel, unsigned char id)
{
	return usbRequest(device, CUSTOM_RQ_FASTLEDANIID_WRITE, (int)id, (int)channel, 0, NULL);
}

int usbfaceFastledAnimationOptionRead(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char *option)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FASTLEDOPTION_READ, animation, (int)channel, 1, &read);
    *option = read;
    return res;
}

int usbfaceFastledAnimationOptionWrite(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char option)
{
    //qDebug() << __PRETTY_FUNCTION__ << " channel: " << channel << ", animation: " << animation << ", option: " << option;
    uint8_t request[2];
    request[0] = animation;
    request[1] = option;
    return usbRequestData(device, CUSTOM_RQ_FASTLEDOPTION_WRITE, channel, 2, &request[0], 0, 0);
}

int usbfaceFastledStateRead(hid_device *device, const unsigned char channel, unsigned char *running)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FASTLEDRUNNING_READ, 0, (int)channel, 1, &read);
    *running = read;
    return res;
}

int usbfaceFastledStateWrite(hid_device *device, const unsigned char channel, unsigned char running)
{
    return usbRequest(device, CUSTOM_RQ_FASTLEDRUNNING_WRITE, (int)running, (int)channel, 0, NULL);
}

int usbfaceFastledColorRead(hid_device *device, const unsigned char channel, const unsigned char animation, const unsigned char colorid, unsigned char *colors)
{    
    uint8_t request[2];
    request[0] = animation;
    request[1] = colorid;
    return usbRequestData(device, CUSTOM_RQ_FASTLEDCOLOR_READ, channel, 2, &request[0], 3, colors);
}

int usbfaceFastledColorWrite(hid_device *device, const unsigned char channel, const unsigned char animation, const unsigned char colorid, unsigned char *colors)
{
    //qDebug() << __PRETTY_FUNCTION__ << " channel: " << channel << ", animation: " << animation << ", colorid: " << colorid;
    //qDebug() << __PRETTY_FUNCTION__ << " " << colors[0] << " " << colors[1] << " " << colors[2];

    uint8_t request[5];
    request[0] = animation;
    request[1] = colorid;
    request[2] = colors[0];
    request[3] = colors[1];
    request[4] = colors[2];
    return usbRequestData(device, CUSTOM_RQ_FASTLEDCOLOR_WRITE, channel, 5, &request[0], 0, 0);
}

int usbfaceFastledAutostartRead(hid_device *device, const unsigned char channel, unsigned char *start)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FASTLEDACTIVE_READ, 0, (int)channel, 1, &read);
    *start = read;
    return res;
}

int usbfaceFastledAutostartWrite(hid_device *device, const unsigned char channel, unsigned char start)
{
    return usbRequest(device, CUSTOM_RQ_FASTLEDACTIVE_WRITE, (int)start, (int)channel, 0, NULL);
}

int usbfaceFastledSnsIdRead(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char *snsId)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FASTLEDSNSID_READ, (int)animation, (int)channel, 1, &read);
    *snsId = read;
    return res;
}

int usbfaceFastledSnsIdWrite(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char snsId)
{
    uint8_t request[2];
    request[0] = animation;
    request[1] = snsId;
    return usbRequestData(device, CUSTOM_RQ_FASTLEDSNSID_WRITE, channel, 2, &request[0], 0, 0);
}

int usbfaceFastledFPSRead(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char *fps)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_FASTLEDFPS_READ, (int)animation, (int)channel, 1, &read);
    *fps = read;
    return res;
}

int usbfaceFastledFPSWrite(hid_device *device, const unsigned char channel, const unsigned char animation, unsigned char fps)
{
    //qDebug() << __PRETTY_FUNCTION__ << " channel: " << channel << ", animation: " << animation << ", fps: " << fps;
    uint8_t request[2];
    request[0] = animation;
    request[1] = fps;
    return usbRequestData(device, CUSTOM_RQ_FASTLEDFPS_WRITE, channel, 2, &request[0], 0, 0);
}

int usbfacePowerMeterPowerRead(hid_device *device, const unsigned char channel, unsigned int *milliwatt)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_POWERMTR_POWER_READ, 0, (int)channel, 2, &read);
    *milliwatt = read;
    return res;
}

int usbfacePowerMeterCurrentRead(hid_device *device, const unsigned char channel, unsigned int *milliampere)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_POWERMTR_CURRENT_READ, 0, (int)channel, 2, &read);
    *milliampere = read;
    return res;
}

int usbfacePowerMeterLoadRead(hid_device *device, const unsigned char channel, unsigned int *millivolt)
{
    int read;
    int res = usbRequest(device, CUSTOM_RQ_POWERMTR_LOAD_READ, 0, (int)channel, 2, &read);
    *millivolt = read;
    return res;
}

int usbfaceReadSettings(hid_device *device)
{
    return usbRequest(device, CUSTOM_RQ_EEPROM_READ, 0, 0, 0, NULL);
}

int usbfaceWriteSettings(hid_device *device)
{
    return usbRequest(device, CUSTOM_RQ_EEPROM_WRITE, 0, 0, 0, NULL);
}

int usbfaceDefaultSettings(hid_device *device)
{
    return usbRequest(device, CUSTOM_RQ_LOAD_DEFAULTS, 0, 0, 0, NULL);
}

int usbfaceEnterBootloader(hid_device *device)
{
    return usbRequest(device, CUSTOM_RQ_ENTER_BOOTLOAD, 0, 0, 0, NULL);
}

int usbfaceResetDevice(hid_device *device)
{
    Q_UNUSED(device);
    //return usb_reset(device);
    return USBFACE_ERR_ACCESS;
}

// ---- Miscellaneous ---------------------------------------------------------

const char *usbfaceErrToString(const int err)
{
    switch (err)
    {
    case USBFACE_SUCCESS:
        return "Success";
    case USBFACE_ERR_ACCESS:
        return "Access not allowed. Insufficient permissions to open device";
    case USBFACE_ERR_IO:
        return "I/O error";
    case USBFACE_ERR_NOTFOUND:
        return "Device not found";
    case USBFACE_ERR_READ:
        return "Read error";
    case USBFACE_ERR_WRITE:
        return "Write error";
    case USBFACE_ERR_SPI_CLK:
        return "Illegal SPI clock value";
    case USBFACE_ERR_MALLOC:
        return "Illegal SPI clock value";
    default:
        return "Unknown error";
    }
}

const char *usbfaceSensorTypeToString(const SNSTYPE type)
{
    switch (type)
    {
    case SNSTYPE_NONE:
        return "None";
    case SNSTYPE_TEMP_NTC0:
        return "Temp_NTC0";
    case SNSTYPE_TEMP_NTC1:
        return "Temp_NTC1";
    case SNSTYPE_TEMP_NTC2:
        return "Temp_NTC2";
    case SNSTYPE_TEMP_NTC3:
        return "Temp_NTC3";
    case SNSTYPE_TEMP_NTC4:
        return "Temp_NTC4";
    case SNSTYPE_TEMP_NTC5:
        return "Temp_NTC5";
    case SNSTYPE_TEMP_I2C_ADDR0:
        return "Temp_I2C_Addr0";
    case SNSTYPE_TEMP_I2C_ADDR1:
        return "Temp_I2C_Addr1";
    case SNSTYPE_TEMP_I2C_ADDR2:
        return "Temp_I2C_Addr2";
    case SNSTYPE_TEMP_I2C_ADDR3:
        return "Temp_I2C_Addr3";
    case SNSTYPE_TEMP_I2C_ADDR4:
        return "Temp_I2C_Addr4";
    case SNSTYPE_TEMP_I2C_ADDR5:
        return "Temp_I2C_Addr5";
    case SNSTYPE_TEMP_I2C_ADDR6:
        return "Temp_I2C_Addr6";
    case SNSTYPE_TEMP_I2C_ADDR7:
        return "Temp_I2C_Addr7";
    case SNSTYPE_EXT0:
        return "Ext0";
    case SNSTYPE_EXT1:
        return "Ext1";
    case SNSTYPE_EXT2:
        return "Ext2";
    case SNSTYPE_EXT3:
        return "Ext3";
    case SNSTYPE_RPS_FAN0:
        return "Rps_Fan0";
    case SNSTYPE_RPS_FAN1:
        return "Rps_Fan1";
    case SNSTYPE_RPS_FAN2:
        return "Rps_Fan2";
    case SNSTYPE_RPS_FAN3:
        return "Rps_Fan3";
    case SNSTYPE_RPS_FAN4:
        return "Rps_Fan4";
    case SNSTYPE_RPS_FAN5:
        return "Rps_Fan5";
    case SNSTYPE_DUTY_IN0:
        return "Duty_In0";
    case SNSTYPE_DUTY_IN1:
        return "Duty_In1";
    case SNSTYPE_POWER0:
    	return "Power0";
    case SNSTYPE_POWER1:
    	return "Power1";
    default:
        return "";
    }
}

const char *usbfaceFanOutModeToString(const FANOUTMODE mode)
{
    switch (mode)
    {
    case FANOUTMODE_RPS_MINFAN:
        return "Rps_MinFan";
    case FANOUTMODE_RPS_MAXFAN:
        return "Rps_MaxFan";
    case FANOUTMODE_RPS_CLONEFAN0:
        return "Rps_CloneFan0";
    case FANOUTMODE_RPS_CLONEFAN1:
        return "Rps_CloneFan1";
    case FANOUTMODE_RPS_CLONEFAN2:
        return "Rps_CloneFan2";
    case FANOUTMODE_RPS_CLONEFAN3:
        return "Rps_CloneFan3";
    case FANOUTMODE_RPS_CLONEFAN4:
        return "Rps_CloneFan4";
    case FANOUTMODE_RPS_CLONEFAN5:
        return "Rps_CloneFan5";
    case FANOUTMODE_RPS_FIXED_50:
        return "Rps_Fixed_50";
    default:
        return "";
    }
}

const char *usbfaceFanTypeToString(const FANTYPE type, const int desc)
{
    switch (type)
    {
    case FANTYPE_NONE:
        if (!desc)
            return "None";
        else
            return "No fan";
    case FANTYPE_2WIRE:
        if (!desc)
            return "2-wire";
        else
            return "No RPM, no PWM";
    case FANTYPE_3WIRE:
        if (!desc)
            return "3-wire";
        else
            return "RPM, no PWM";
    case FANTYPE_4WIRE:
        if (!desc)
            return "4-wire";
        else
            return "RPM, PWM";
    default:
        return "";
        break;
    }
}

const char *usbfaceFanModeToString(const FANMODE mode)
{
    switch (mode)
    {
    case FANMODE_FIXED_DUTY:
        return "FixedDuty";
    case FANMODE_PI:
        return "PiControl";
    case FANMODE_LINEAR:
        return "LinearControl";
    case FANMODE_TRIP_POINTS:
        return "TripPoints";
    case FANMODE_LINEAR_TRIP_POINTS:
        return "LinearTripPoints";
    case FANMODE_FUZZY:
    	return "FuzzyControl";
    default:
        return "";
    }
}

