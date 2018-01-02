
#include "shell_sensor.h"
#include "config.h"
#include "types.h"
#include "lufa_virtual_serial.h"
#include "sensor.h"
#include <inttypes.h>
#include <stdbool.h>

/*
 * Get/set sensor configuration
 *
 * sns cfg
 * sns cfg #
 * sns cfg # sty <SNSTYPE_*>
 *
 *
 * Get sensor status
 *
 * sns sta
 * sns sta #
 * sns sta # val
 *
 *
 * Set sensor value (if of type SNSTYPE_EXT[0-3])
 *
 * sns set # <int8_t value>
 *
 */

bool sensor_cmd_cfg(uint8_t argc, char **argv);
bool sensor_cmd_sta(uint8_t argc, char **argv);
bool sensor_cmd_set(uint8_t argc, char **argv);

const struct _s_shell_cmd sensor_shell_cmd[] PROGMEM = {SHELLCMD("cfg", sensor_cmd_cfg, "[# [sty u8]]", "sensor configuration"),
                                                        SHELLCMD("sta", sensor_cmd_sta, "[# [val]", "sensor status"),
														SHELLCMD("set", sensor_cmd_set, "# set i8", "set sensor value"),
														SHELLCMD(0, 0, 0, 0)};

static void dump_sensor_config(uint8_t channel)
{
    fprintf_P(_vsf, PSTR(".sns %u\n"), channel);
    fprintf_P(_vsf, PSTR(".sty %u\n"), sns[channel].type);
}

bool sensor_cmd_cfg(uint8_t argc, char **argv)
{
    uint8_t channel = 0;

    if (argc > 0)
    {
        channel = atoi(argv[0]);

        if (channel >= MAX_FANS)
            return false;
    }

    if (argc == 0)
    {
        for (uint8_t i = 0; i < MAX_FANS; i++)
        	dump_sensor_config(i);
    }
    else if (argc == 1)
    {
    	dump_sensor_config(channel);
    }
    else if (argc == 3)
    {
        uint8_t val = atoi(argv[2]);
        return set_if_match_8(argv[1], PSTR("sty"), val, &sns[channel].type);
    }
    else
    {
        return false;
    }

    return true;
}

static void dump_sensor_status(uint8_t channel)
{
    fprintf_P(_vsf, PSTR(".sns %u\n"), channel);
    fprintf_P(_vsf, PSTR(".val %d\n"), sns[channel].value);
    fprintf_P(_vsf, PSTR(".vld %u\n"), sns[channel].status.valid);
}

bool sensor_cmd_sta(uint8_t argc, char **argv)
{
    bool success = true;
    uint8_t channel = 0;

    if (argc > 0)
    {
        channel = atoi(argv[0]);

        if (channel >= MAX_SNS)
            return false;
    }

    if (argc == 0)
    {
        for (uint8_t i = 0; i < MAX_SNS; i++)
            dump_sensor_status(i);
    }
    else if (argc == 1)
    {
        dump_sensor_status(channel);
    }

    else if (argc == 2)
    {
        if (strcmp_P(PSTR("val"), argv[1]) == 0)
        {
            fprintf_P(_vsf, PSTR(".val %d\n"), sns[channel].value);
        }
        else
        {
            success = false;
        }
    }
    else
    {
        success = false;
    }

    return success;
}

bool sensor_cmd_set(uint8_t argc, char **argv)
{
	if (argc != 2)
		return false;

    uint8_t channel = atoi(argv[0]);

	if (channel >= MAX_SNS || sns[channel].type > SNSTYPE_EXT3 || sns[channel].type < SNSTYPE_EXT0 )
		return false;

	int8_t val = atoi(argv[1]);
	sns[channel].value = val;

	return true;
}

