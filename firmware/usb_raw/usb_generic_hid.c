
#include "usb_generic_hid.h"
#include "config.h"
#include "ctrl.h"
#include "descriptors.h"
#include "fan.h"
#include "fan_out.h"
#include "powermeter.h"
#include "requests.h"
#include "sensor.h"
#include "settings.h"
#include "animation.h"
#include "debug.h"

#if EEPROM_UPDOWNLOAD
static uint16_t usbFunctionBytesRemain;
#endif

#if ENTER_BOOTLOADER_REMOTELY
unsigned char EEMEM eeprom_bootload_trigger = 0;
uint8_t eeprom_bootload_counter = 0;
#endif

static uint8_t responseBuffer[GENERIC_REPORT_SIZE]; // buffer must stay valid when usbFunctionSetup returns

// USB request handler
void ProcessGenericHIDReport(uint8_t *DataArray)
{
    uint8_t requestId = DataArray[0];
    int channel = DataArray[1]; // Most calls use rq->wIndex.bytes[0] as fan/sensor index; store in channel.
    uint8_t *requestData = &DataArray[2];

    memset(responseBuffer, 0xFE, GENERIC_REPORT_SIZE);

    responseBuffer[0] = requestId;
    uint8_t *dataBuffer = &responseBuffer[1];

    //LV_("req %u - %u", requestId, channel);
    //LV_("req %x %x %x %x", requestData[0], requestData[1], requestData[2], requestData[3]);

#if (USB_COMM_WATCHDOG_TIMEOUT > 0)
    // Reset the usb communication watchdog timer.
    usb_comm_watchdog_count = 0;
#endif

#if ENTER_BOOTLOADER_REMOTELY
    if (CUSTOM_RQ_ENTER_BOOTLOAD == rq->bRequest)
    {
        // Bootloader will only be entered when this call is made a successive number of times.
        eeprom_bootload_counter++;
        if (eeprom_bootload_counter > BOOTLOADER_ENTER_NUM_REQ)
        {
            eeprom_write_byte(&eeprom_bootload_trigger, BOOTLOADER_MAGIC);
            // Endless loop; triggering watchdog stops to force reset and enter bootloader.
            for (;;)
            {
            };
        }
    }
    else
    {
        // Non CUSTOM_RQ_ENTER_BOOTLOAD call: reset bootload call counter.
        eeprom_bootload_counter = 0;
    }
#endif

    switch (requestId)
    {
    case CUSTOM_RQ_ECHO:
        // echo -- used for communication reliability tests
        // return ones complement of input
        dataBuffer[0] = ~requestData[0];
        dataBuffer[1] = ~requestData[1];
        dataBuffer[2] = requestData[0];
        dataBuffer[3] = requestData[1];
        break;
    case CUSTOM_RQ_FUNCS_SUPPORTED:
        *((uint16_t *)dataBuffer) = 0
#if SNS_ANALOG_SUPPORTED
                                    | SUPPORTED_SNS_ANALOG
#endif
#if SNS_I2C_SUPPORTED
                                    | SUPPORTED_SNS_I2C
#endif
#if POWER_METER_SUPPORTED
                                    | SUPPORTED_SNS_POWER
#endif
#if FAN_OUT_SUPPORTED
                                    | SUPPORTED_FAN_OUT
#endif
#if CTRL_DEBUG
                                    | SUPPORTED_CTRL_DEBUG
#endif
#if (SUPPORTED_USBCOMM_WATCHDOG > 0)
                                    | SUPPORTED_USBCOMM_WATCHDOG
#endif
#if FAN_MODE_LINEAR_SUPPORTED
                                    | SUPPORTED_FANMODE_LINEAR
#endif
#if FAN_MODE_PI_SUPPORTED
                                    | SUPPORTED_FANMODE_PI
#endif
#if FAN_MODE_FUZZY_SUPPORTED
                                    | SUPPORTED_FANMODE_FUZZY
#endif
#if FAN_MODE_TRIP_POINTS_SUPPORTED
                                    | SUPPORTED_FANMODE_TP
#endif
#if FAN_MODE_LIN_TRIP_POINTS_SUPPORTED
                                    | SUPPORTED_FANMODE_LIN_TP
#endif
#if FASTLED_SUPPORTED
                                    | SUPPORTED_LED_FASTLED
#endif
#if EEPROM_UPDOWNLOAD
                                    | SUPPORTED_EEPROM_UPDOWNLD
#endif
            ;
        break;
    case CUSTOM_RQ_STATUS_READ:
        *dataBuffer = getStatus();
        break;
    case CUSTOM_RQ_PROTOCOL_VERSION:
        *dataBuffer = USB_PROTOCOL_VERSION;
        break;
    case CUSTOM_RQ_FIRMWARE_VERSION:
        dataBuffer[0] = VERSION_MAJOR;
        dataBuffer[1] = VERSION_MINOR;
        break;
    case CUSTOM_RQ_FANRPS_READ:
        // rq->wIndex.bytes[0]   Channel
        *((uint16_t *)dataBuffer) = fans[channel].rps;
        break;
    case CUSTOM_RQ_FANRPM_READ:
        // rq->wIndex.bytes[0]   Channel
        *((uint16_t *)dataBuffer) = fans[channel].rpm;
        break;
    case CUSTOM_RQ_FANDUTY_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fans[channel].duty;
        break;
    case CUSTOM_RQ_FANTYPE_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   Type
        fans[channel].config.fanType = requestData[0];
        break;
    case CUSTOM_RQ_FANTYPE_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fans[channel].config.fanType;
        break;
    case CUSTOM_RQ_FANMODE_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   Mode
        fans[channel].config.fanMode = requestData[0];
        break;
    case CUSTOM_RQ_FANMODE_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fans[channel].config.fanMode;
        break;
#if FAN_MODE_PI_SUPPORTED || FAN_MODE_FUZZY_SUPPORTED
    case CUSTOM_RQ_FANSETP_DELTA_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   Setpoint delta
        fans[channel].snsSetp.delta = requestData[0];
        // Restart control loop
        restartCtrl(channel);
        break;
    case CUSTOM_RQ_FANSETP_DELTA_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fans[channel].snsSetp.delta;
        break;
    case CUSTOM_RQ_FANSETP_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = sns[fans[channel].snsSetp.snsIdx].value + fans[channel].snsSetp.delta;
        break;
    case CUSTOM_RQ_FANSETP_REFSNS_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   Reference sensor index
        fans[channel].snsSetp.snsIdx = requestData[0];
        // Restart control loop
        restartCtrl(channel);
        break;
    case CUSTOM_RQ_FANSETP_REFSNS_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fans[channel].snsSetp.snsIdx;
        break;
#endif
#if FAN_MODE_PI_SUPPORTED
    case CUSTOM_RQ_FANKP_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // rq->wValue.word       Kp
        fans[channel].Kp = *((uint16_t *)requestData);
        break;
    case CUSTOM_RQ_FANKP_READ:
        *((uint16_t *)dataBuffer) = fans[channel].Kp;
        break;
    case CUSTOM_RQ_FANKI_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // rq->wValue.word       Ki
        fans[channel].Ki = *((uint16_t *)requestData);
        break;
    case CUSTOM_RQ_FANKI_READ:
        *((uint16_t *)dataBuffer) = fans[channel].Ki;
        break;
    case CUSTOM_RQ_FANKT_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // rq->wValue.word       Kt
        fans[channel].Kt = *((uint16_t *)requestData);
        break;
    case CUSTOM_RQ_FANKT_READ:
        *((uint16_t *)dataBuffer) = fans[channel].Kt;
        break;
#endif
    case CUSTOM_RQ_FANDUTYFIXED_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   Dutycycle. [0%..100%] = [0..255]
        fans[channel].dutyFixed = requestData[0];
        break;
    case CUSTOM_RQ_FANDUTYFIXED_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fans[channel].dutyFixed;
        break;
#if FAN_MODE_LINEAR_SUPPORTED
    case CUSTOM_RQ_FANGAIN_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // rq->wValue.word       Gain, lower 6 bits contain fraction
        fans[channel].dutyGain = *((uint16_t *)requestData);
        break;
    case CUSTOM_RQ_FANGAIN_READ:
        *((int16_t *)dataBuffer) = fans[channel].dutyGain;
        break;
    case CUSTOM_RQ_FANOFFS_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // rq->wValue.word       Offset
        fans[channel].dutyOffs = *((uint16_t *)requestData);
        break;
    case CUSTOM_RQ_FANOFFS_READ:
        *((int16_t *)dataBuffer) = fans[channel].dutyOffs;
        break;
#endif
#if FAN_MODE_PI_SUPPORTED || FAN_MODE_LIN_TRIP_POINTS_SUPPORTED
    case CUSTOM_RQ_FANTRIPPOINT_READ:
        dataBuffer[0] = fans[channel].trip_point[requestData[0]].value;
        dataBuffer[1] = fans[channel].trip_point[requestData[0]].duty;
        break;
    case CUSTOM_RQ_FANTRIPPOINT_WRITE:
        fans[channel].trip_point[requestData[0]].value = dataBuffer[1];
        fans[channel].trip_point[requestData[0]].value = dataBuffer[2];
        break;
#endif
    case CUSTOM_RQ_FANSNS_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   Sensor index
        fans[channel].config.snsIdx = requestData[0]; // sensor index to use
        break;
    case CUSTOM_RQ_FANSNS_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fans[channel].config.snsIdx;
        break;
    case CUSTOM_RQ_FANMINRPS_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   Minimum rps before fan is seen as stalled.
        fans[channel].min_rps = requestData[0];
        break;
    case CUSTOM_RQ_FANMINRPS_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fans[channel].min_rps;
        break;
    case CUSTOM_RQ_FANSTALL_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fans[channel].status.stalled;
        break;
    case CUSTOM_RQ_SNSTYPE_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   Type
        sns[channel].type = requestData[0]; // see SnsType for values
        break;
    case CUSTOM_RQ_SNSTYPE_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = sns[channel].type;
        break;
    case CUSTOM_RQ_SNS_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   Value
        sns[channel].value = requestData[0];
        break;
    case CUSTOM_RQ_SNS_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = sns[channel].value;
        break;
#if FAN_OUT_SUPPORTED
    case CUSTOM_RQ_FANSTALLDETECT_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[0]   When != 0, the fan rps will be included in the stall detection for the output fan
        if (requestData[0])
            fan_out[0].fanStallDetect |= 1 << channel;
        else
            fan_out[0].fanStallDetect &= ~(1 << channel);
        break;
    case CUSTOM_RQ_FANSTALLDETECT_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fan_out[0].fanStallDetect & (1 << channel);
        break;
    case CUSTOM_RQ_FANOUTMODE_WRITE:
        // rq->wIndex.bytes[0]   Channel
        // requestData[1]   Mode, see FANOUTMODE
        fan_out[channel].mode = requestData[0];
        break;
    case CUSTOM_RQ_FANOUTMODE_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fan_out[channel].mode;
        break;
    case CUSTOM_RQ_FANOUTRPS_READ:
        // rq->wIndex.bytes[0]   Channel
        *dataBuffer = fan_out[channel].rps;
        break;
    case CUSTOM_RQ_FANOUTRPM_READ:
        // rq->wIndex.bytes[0]   Channel
        *((uint16_t *)dataBuffer) = fan_out[channel].rpm;
        break;
#endif
#if POWER_METER_SUPPORTED
    case CUSTOM_RQ_POWERMTR_POWER_READ:
        *((uint16_t *)dataBuffer) = powermeters[channel].power;
        break;
    case CUSTOM_RQ_POWERMTR_CURRENT_READ:
        *((uint16_t *)dataBuffer) = powermeters[channel].current;
        break;
    case CUSTOM_RQ_POWERMTR_LOAD_READ:
        *((uint16_t *)dataBuffer) = powermeters[channel].bus;
        break;
#endif
#if FASTLED_SUPPORTED
    case CUSTOM_RQ_FASTLEDACTIVE_READ:
    	*dataBuffer = animation_get_autoplay(channel);
        break;
    case CUSTOM_RQ_FASTLEDACTIVE_WRITE:
    	animation_set_autoplay(channel, requestData[0]);
        break;
    case CUSTOM_RQ_FASTLEDANIID_READ:
    	*dataBuffer = animation_get_current(channel);
        break;
    case CUSTOM_RQ_FASTLEDANIID_WRITE:
    	animation_set_current(channel, requestData[0]);
        break;
    case CUSTOM_RQ_FASTLEDCOLOR_READ:
    	animation_get_color(channel, requestData[0], requestData[1], dataBuffer);
        break;
    case CUSTOM_RQ_FASTLEDCOLOR_WRITE:
    	animation_set_color(channel, requestData[0], requestData[1], &requestData[2]);
        break;
    case CUSTOM_RQ_FASTLEDSNSID_READ:
    	*dataBuffer = animation_get_sensor_index(channel, requestData[0]);
        break;
    case CUSTOM_RQ_FASTLEDSNSID_WRITE:
    	animation_set_sensor_index(channel, requestData[0], requestData[1]);
        break;
    case CUSTOM_RQ_FASTLEDFPS_READ:
    	*dataBuffer = animation_get_fps(channel, requestData[0]);
        break;
    case CUSTOM_RQ_FASTLEDFPS_WRITE:
    	animation_set_fps(channel, requestData[0], requestData[1]);
        break;
    case CUSTOM_RQ_FASTLEDOPTION_READ:
    	*dataBuffer = animation_get_option(channel, requestData[0]);
    	break;
    case CUSTOM_RQ_FASTLEDOPTION_WRITE:
    	animation_set_option(channel, requestData[0], requestData[1]);
		break;
    case CUSTOM_RQ_FASTLEDBRIGHT_READ:
    	*dataBuffer = animation_get_global_brightness();
    	break;
    case CUSTOM_RQ_FASTLEDBRIGHT_WRITE:
    	animation_set_global_brightness(requestData[0]);
    	break;
    case CUSTOM_RQ_FASTLEDRUNNING_READ:
    	*dataBuffer = animation_get_running(channel);
        break;
    case CUSTOM_RQ_FASTLEDRUNNING_WRITE:
    	animation_set_running(channel, requestData[0]);
        break;
#endif
#if EEPROM_UPDOWNLOAD
    case CUSTOM_RQ_EEPROM_DOWNLOAD:
        usbFunctionBytesRemain = rq->wLength.word;
        return USB_NO_MSG; // tell driver to use usbFunctionRead()
    case CUSTOM_RQ_EEPROM_UPLOAD:
        usbFunctionBytesRemain = rq->wLength.word;
        return USB_NO_MSG; // tell driver to use usbFunctionWrite()
#endif
#if CTRL_DEBUG
    case CUSTOM_RQ_FANE_READ:
        *dataBuffer = fans_dbg[channel].e;
        break;
    case CUSTOM_RQ_FANP_READ:
        *((uint16_t *)dataBuffer) = fans_dbg[channel].p;
        break;
    case CUSTOM_RQ_FANI_READ:
        *((uint16_t *)dataBuffer) = fans[channel].i;
        break;
#endif
    default:
        LV_("hid req %u unsupported", requestId);
        responseBuffer[0] = CUSTOM_RQ_UNSUPPORTED;
        responseBuffer[1] = channel;
        break;
    }

    //LV_("rsp %u", responseBuffer[0]);
    //LV_("rsp %x %x %x", responseBuffer[1], responseBuffer[2], responseBuffer[3]);

    SendGenericHIDReport(responseBuffer, sizeof(responseBuffer));

    switch (requestId)
    {
    case CUSTOM_RQ_EEPROM_READ:
        // Load settings from eeprom, or revert to defaults when eeprom is invalid.
        loadSettings(0);
#if FASTLED_SUPPORTED
        animation_load(false);
#endif
        break;
    case CUSTOM_RQ_EEPROM_WRITE:
        // Save settings to eeprom.
        saveSettings();
#if FASTLED_SUPPORTED
        animation_save();
#endif
        break;
    case CUSTOM_RQ_LOAD_DEFAULTS:
        // Load default settings.
        loadSettings(1);
#if FASTLED_SUPPORTED
        animation_load(true);
#endif
        break;
    }
}

#if USB_CFG_IMPLEMENT_FN_READ
// Deferred handling of USB read request.
uchar usbFunctionRead(uchar *data, uchar len)
{
    // Request was: CUSTOM_RQ_EEPROM_DOWNLOAD
    if (len > usbFunctionBytesRemain) // len is max chunk size
        len = usbFunctionBytesRemain; // send an incomplete chunk
    usbFunctionBytesRemain -= len;
    eeprom_read_block((void *)data, (const void *)(EEPROM_SIZE - usbFunctionBytesRemain), len);
    return len; // return real chunk size
}
#endif

#if USB_CFG_IMPLEMENT_FN_WRITE
// Deferred handling of USB write request.
uchar usbFunctionWrite(uchar *data, uchar len)
{
    // Request was: CUSTOM_RQ_EEPROM_UPLOAD
    if (len > usbFunctionBytesRemain) // if this is the last incomplete chunk
        len = usbFunctionBytesRemain; // limit to the amount we can store
    usbFunctionBytesRemain -= len;
    eeprom_write_block((const void *)data, (void *)(EEPROM_SIZE - usbFunctionBytesRemain), len);
    if (usbFunctionBytesRemain == 0)
    {
        loadSettings(0); // activate new eeprom contents
        return 1;        // return 1 to indicate we got all data
    }
    return 0; // more to go...
}
#endif
