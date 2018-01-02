
#include "powermeter.h"
#include "power.h"

PowerMeter powermeters[MAX_POWERMETER];
//PowerMeter EEMEM powermeters_eeprom[MAX_POWERMETER];

void initPowerMeter(void)
{
#if POWER_METER_SUPPORTED
	powermeter_init();
#endif
}

void updatePowerMeter(void)
{
#if POWER_METER_SUPPORTED
    uint8_t i;
    for (i = 0; i < MAX_POWERMETER; i++)
    {
    	powermeters[i].bus = powermeter_getBusVoltage_mV(i);
    	powermeters[i].current = powermeter_getCurrent_mA(i);
    	powermeters[i].power = powermeter_getPower_mW(i);
    }
#endif
}
