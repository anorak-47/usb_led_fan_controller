
#include "shell_ina219.h"
#include "ina219.h"
#include "lufa_virtual_serial.h"
#include <inttypes.h>
#include <stdbool.h>

#if POWER_METER_INA219 & DEBUG_FUNCTIONS_SUPPORTED

bool ina219_cmd_set_calibration(uint8_t argc, char **argv);
bool ina219_cmd_load(uint8_t argc, char **argv);
bool ina219_cmd_current(uint8_t argc, char **argv);
bool ina219_cmd_power(uint8_t argc, char **argv);

const struct _s_shell_cmd ina219_shell_cmd[] PROGMEM = {SHELLCMD("cal", ina219_cmd_set_calibration, "", "calibration 32V_2A|32V_1A|16V_400mA"),
                                                        SHELLCMD("pwr", ina219_cmd_power, "", "read power in mW"),
                                                        SHELLCMD("lod", ina219_cmd_load, "", "read shunt/bus load in mV"),
                                                        SHELLCMD("cur", ina219_cmd_current, "", "read current in mA"), SHELLCMD(0, 0, 0, 0)};

bool ina219_cmd_set_calibration(uint8_t argc, char **argv)
{
    if (argc != 2)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    uint8_t cal = atoi(argv[1]);

    switch (cal)
    {
    case 0:
        ina219_setCalibration_32V_2A(&ina219_device[channel]);
        break;
    case 1:
        ina219_setCalibration_32V_1A(&ina219_device[channel]);
        break;
    case 2:
        ina219_setCalibration_16V_400mA(&ina219_device[channel]);
        break;
    default:
        break;
    }

    return true;
}

bool ina219_cmd_load(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    fprintf_P(_sf, PSTR(".ina %u\n"), channel);
    fprintf_P(_sf, PSTR(".bus %u\n"), ina219_getBusVoltage_mV(&ina219_device[channel]));
    fprintf_P(_sf, PSTR(".sht %u\n"), ina219_getShuntVoltage_mV(&ina219_device[channel]));

    return true;
}

bool ina219_cmd_power(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    fprintf_P(_sf, PSTR(".ina %u\n"), channel);
    fprintf_P(_sf, PSTR(".pwr %u\n"), ina219_getPower_mW(&ina219_device[channel]));

    return true;
}

bool ina219_cmd_current(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t channel = atoi(argv[0]);
    if (channel >= MAX_POWERMETER)
        return false;

    fprintf_P(_sf, PSTR(".ina %u\n"), channel);
    fprintf_P(_sf, PSTR(".cur %u\n"), ina219_getCurrent_mA(&ina219_device[channel]));

    return true;
}

#endif
