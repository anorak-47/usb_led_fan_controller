
#include "power.h"
#include "config.h"
#include "debug.h"
#include "i2c.h"
#include "ina219.h"
#include "ina260.h"
#include "wire.h"

#if POWER_METER_SUPPORTED

#define FIRST_INA_ADDRESS 0x80

enum power_meter_device_types
{
    pm_no_device,
#if POWER_METER_INA219
    pm_ina219,
#endif
#if POWER_METER_INA260
    pm_ina260
#endif
};

#if POWER_METER_INA219
enum power_meter_device_types power_meter_channels[MAX_POWERMETER] = {pm_ina219, pm_no_device};
#elif POWER_METER_INA260
enum power_meter_device_types power_meter_channels[MAX_POWERMETER] = {pm_ina260, pm_ina260, pm_ina260};
#elif POWER_METER_INA260 && POWER_METER_INA219
enum power_meter_device_types power_meter_channels[MAX_POWERMETER] = {pm_ina219, pm_no_device};
#endif

void powermeter_init(void)
{
    for (uint8_t d = 0; d < MAX_POWERMETER; d++)
    {
        uint8_t address = FIRST_INA_ADDRESS | (d << 1);
        LV_("ina scan 0x%x", address);
        if (i2c_scan_address(address))
        {
            LV_("ina found 0x%x", address);
            switch (power_meter_channels[d])
            {
#if POWER_METER_INA219
            case pm_ina219:
                ina219_init(&ina219_device[d], address);
                break;
#endif
#if POWER_METER_INA260
            case pm_ina260:
                ina260_init(&ina260_device[d], address);
                break;
#endif
            case pm_no_device:
            	power_meter_channels[d] = pm_no_device;
                break;
            }
        }
    }
}

#if 0
void powermeter_scan_for_devices(void)
{
#if POWER_METER_INA219
    if (i2c_scan_address(INA219_ADDRESS))
    {
        fprintf_P(_vsf, PSTR("ina 0x%x\n"), INA219_ADDRESS);

        uint16_t id = 0;
        wireReadRegister(INA219_ADDRESS, INA260_MANUFACTURER_ID, &id);

        fprintf_P(_vsf, PSTR("mid 0x%x\n"), id);

        if (id == INA260_DEFAULT_MANUFACTURER_ID)
        {

            wireReadRegister(INA219_ADDRESS, INA260_DIE_ID, &id);

            fprintf_P(_vsf, PSTR("did 0x%x\n"), id);

            if (id == INA260_DEFAULT_DIE_ID)
            {
                // --> must be an INA260
            }
        }

        // power_meter_channels[0] = pm_ina219;
        // power_meter_channel_to_device[0] = 0;
        // ina219_init(&ina219_device[0], INA219_ADDRESS);
    }
#endif
}
#endif

uint32_t powermeter_getBusVoltage_mV(uint8_t channel)
{
    switch (power_meter_channels[channel])
    {
#if POWER_METER_INA219
    case pm_ina219:
        return ina219_getBusVoltage_mV(&ina219_device[channel]);
        break;
#endif
#if POWER_METER_INA260
    case pm_ina260:
        return ina260_getBusVoltage_mV(&ina260_device[channel]);
        break;
#endif
    case pm_no_device:
        break;
    }

    return 0;
}

uint32_t powermeter_getCurrent_mA(uint8_t channel)
{
    switch (power_meter_channels[channel])
    {
#if POWER_METER_INA219
    case pm_ina219:
        return ina219_getCurrent_mA(&ina219_device[channel]);
        break;
#endif
#if POWER_METER_INA260
    case pm_ina260:
        return ina260_getCurrent_mA(&ina260_device[channel]);
        break;
#endif
    case pm_no_device:
        break;
    }

    return 0;
}

uint32_t powermeter_getPower_mW(uint8_t channel)
{
    switch (power_meter_channels[channel])
    {
#if POWER_METER_INA219
    case pm_ina219:
        return ina219_getPower_mW(&ina219_device[channel]);
        break;
#endif
#if POWER_METER_INA260
    case pm_ina260:
        return ina260_getPower_mW(&ina260_device[channel]);
        break;
#endif
    case pm_no_device:
        break;
    }

    return 0;
}

#endif
