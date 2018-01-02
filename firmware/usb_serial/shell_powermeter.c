
#include "shell_powermeter.h"
#include "lufa_virtual_serial.h"
#include "power.h"
#include <inttypes.h>
#include <stdbool.h>

#if POWER_METER_SUPPORTED

bool powermeter_cmd_load(uint8_t argc, char **argv);
bool powermeter_cmd_current(uint8_t argc, char **argv);
bool powermeter_cmd_power(uint8_t argc, char **argv);

const struct _s_shell_cmd powermeter_shell_cmd[] PROGMEM = {SHELLCMD("pwr", powermeter_cmd_power, "", "read power in mW"),
                                                            SHELLCMD("lod", powermeter_cmd_load, "", "read shunt/bus load in mV"),
                                                            SHELLCMD("cur", powermeter_cmd_current, "", "read current in mA"),
															SHELLCMD(0, 0, 0, 0)};

bool powermeter_cmd_load(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    fprintf_P(_vsf, PSTR(".pmt %u\n"), channel);
    fprintf_P(_vsf, PSTR(".bus %u\n"), powermeter_getBusVoltage_mV(channel));

    return true;
}

bool powermeter_cmd_power(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    fprintf_P(_vsf, PSTR(".pmt %u\n"), channel);
    fprintf_P(_vsf, PSTR(".pwr %u\n"), powermeter_getPower_mW(channel));

    return true;
}

bool powermeter_cmd_current(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    fprintf_P(_vsf, PSTR(".pmt %u\n"), channel);
    fprintf_P(_vsf, PSTR(".cur %u\n"), powermeter_getCurrent_mA(channel));

    return true;
}

#endif
