
#include "power.h"
#include "config.h"
#include "i2c.h"
#include "wire.h"
#include "ina219.h"
#include "ina260.h"
#include "lufa_virtual_serial.h"

enum power_meter_device_types
{
    pm_no_device,
    pm_ina219,
    pm_ina260
};

enum power_meter_device_types power_meter_channels[MAX_POWERMETER];
uint8_t power_meter_channel_to_device[MAX_POWERMETER];

void powermeter_init(void)
{
#if POWER_METER_INA219
    if (i2c_scan_address(INA219_ADDRESS))
    {
        power_meter_channels[0] = pm_ina219;
        power_meter_channel_to_device[0] = 0;
        ina219_init(&ina219_device[0], INA219_ADDRESS);
    }
#endif
}

void powermeter_scan_for_devices(void)
{
#if POWER_METER_INA219
    if (i2c_scan_address(INA219_ADDRESS))
    {
        fprintf_P(_sf, PSTR("ina 0x%x\n"), INA219_ADDRESS);

        uint16_t id = 0;
        wireReadRegister(INA219_ADDRESS, INA260_MANUFACTURER_ID, &id);

        fprintf_P(_sf, PSTR("mid 0x%x\n"), id);

        if (id == INA260_DEFAULT_MANUFACTURER_ID)
        {

            wireReadRegister(INA219_ADDRESS, INA260_DIE_ID, &id);

            fprintf_P(_sf, PSTR("did 0x%x\n"), id);

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

uint32_t powermeter_getBusVoltage_mV(uint8_t channel)
{
    if (power_meter_channels[channel] == pm_ina219)
    {
        return ina219_getBusVoltage_mV(&ina219_device[power_meter_channel_to_device[channel]]);
    }

    return 0;
}

uint32_t powermeter_getShuntVoltage_mV(uint8_t channel)
{
    if (power_meter_channels[channel] == pm_ina219)
    {
        return ina219_getShuntVoltage_mV(&ina219_device[power_meter_channel_to_device[channel]]);
    }

    return 0;
}

uint32_t powermeter_getCurrent_mA(uint8_t channel)
{
    if (power_meter_channels[channel] == pm_ina219)
    {
        return ina219_getCurrent_mA(&ina219_device[power_meter_channel_to_device[channel]]);
    }

    return 0;
}

uint32_t powermeter_getPower_mW(uint8_t channel)
{
    if (power_meter_channels[channel] == pm_ina219)
    {
        return ina219_getPower_mW(&ina219_device[power_meter_channel_to_device[channel]]);
    }

    return 0;
}
