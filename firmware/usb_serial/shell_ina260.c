
#include "shell_ina260.h"
#include "ina260.h"
#include "lufa_virtual_serial.h"
#include <inttypes.h>
#include <stdbool.h>

#if POWER_METER_INA260 & DEBUG_FUNCTIONS_SUPPORTED

bool ina260_cmd_load(uint8_t argc, char **argv);
bool ina260_cmd_current(uint8_t argc, char **argv);
bool ina260_cmd_power(uint8_t argc, char **argv);

const struct _s_shell_cmd ina260_shell_cmd[] PROGMEM = {
	SHELLCMD("pwr", ina260_cmd_power, "", "read power in mW"),
    SHELLCMD("lod", ina260_cmd_load, "", "read bus load in mV"),
	SHELLCMD("cur", ina260_cmd_current, "", "read current in mA"),
	SHELLCMD(0, 0, 0, 0)};

bool ina260_cmd_load(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    fprintf_P(_vsf, PSTR(".ina %u\n"), channel);
    fprintf_P(_vsf, PSTR(".bus %u\n"), ina260_getBusVoltage_mV(&ina260_device[channel]));

    return true;
}

bool ina260_cmd_power(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    fprintf_P(_vsf, PSTR(".ina %u\n"), channel);
    fprintf_P(_vsf, PSTR(".pwr %u\n"), ina260_getPower_mW(&ina260_device[channel]));

    return true;
}

bool ina260_cmd_current(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    fprintf_P(_vsf, PSTR(".ina %u\n"), channel);
    fprintf_P(_vsf, PSTR(".cur %u\n"), ina260_getCurrent_mA(&ina260_device[channel]));

    return true;
}

#endif
