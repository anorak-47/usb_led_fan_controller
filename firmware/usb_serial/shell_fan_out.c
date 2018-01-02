
#include "shell_fan_out.h"
#include "lufa_virtual_serial.h"
#include "fan_out.h"
#include <inttypes.h>
#include <stdbool.h>

#if FAN_OUT_SUPPORTED

/*
 * Get/set fan out configuration
 *
 * fao cfg
 * fao cfg #
 * fao cfg # mod <FANOUTMODE_RPS_*>
 * fao cfg # stl <u8> :: set fan stall detect mask
 *
 *
 * Get fan out status
 *
 * fao sta
 * fao sta #
 * fao sta # rpm
 *
 */

bool fanout_cmd_cfg(uint8_t argc, char **argv);
bool fanout_cmd_sta(uint8_t argc, char **argv);

const struct _s_shell_cmd fan_out_shell_cmd[] PROGMEM = {SHELLCMD("cfg", fanout_cmd_cfg, "[# [sty|stl u8]]", "fan out configuration"),
                                                         SHELLCMD("sta", fanout_cmd_sta, "[# [val]", "fan out status"), SHELLCMD(0, 0, 0, 0)};

static void dump_fanout_config(uint8_t channel)
{
    fprintf_P(_vsf, PSTR(".fao %u\n"), channel);
    fprintf_P(_vsf, PSTR(".mod %u\n"), fan_out[channel].mode);
    fprintf_P(_vsf, PSTR(".stl %u\n"), fan_out[channel].fanStallDetect);
#ifdef CTRL_DEBUG
    fprintf_P(_vsf, PSTR(".ovf %u\n"), ovf_interval(channel));
#endif
}

bool fanout_cmd_cfg(uint8_t argc, char **argv)
{
    uint8_t channel = 0;

    if (argc > 0)
    {
        channel = atoi(argv[0]);

        if (channel >= MAX_FAN_OUTS)
            return false;
    }

    if (argc == 0)
    {
        for (uint8_t i = 0; i < MAX_FAN_OUTS; i++)
            dump_fanout_config(i);
    }
    else if (argc == 1)
    {
        dump_fanout_config(channel);
    }
    else if (argc == 3)
    {
        uint8_t val = atoi(argv[2]);
        return set_if_match_8(argv[1], PSTR("mod"), val, (uint8_t*)&fan_out[channel].mode) |
               set_if_match_8(argv[1], PSTR("stl"), val, &fan_out[channel].fanStallDetect);
    }
    else
    {
        return false;
    }

    return true;
}

static void dump_fanout_status(uint8_t channel)
{
    fprintf_P(_vsf, PSTR(".fao %u\n"), channel);
    fprintf_P(_vsf, PSTR(".rps %u\n"), fan_out[channel].rps);
    fprintf_P(_vsf, PSTR(".rpm %u\n"), fan_out[channel].rpm);
}

bool fanout_cmd_sta(uint8_t argc, char **argv)
{
    bool success = true;
    uint8_t channel = 0;

    if (argc > 0)
    {
        channel = atoi(argv[0]);

        if (channel >= MAX_FAN_OUTS)
            return false;
    }

    if (argc == 0)
    {
        for (uint8_t i = 0; i < MAX_FAN_OUTS; i++)
            dump_fanout_status(i);
    }
    else if (argc == 1)
    {
        dump_fanout_status(channel);
    }

    else if (argc == 2)
    {
        if (strcmp_P(argv[1], PSTR("rpm")) == 0)
        {
            fprintf_P(_vsf, PSTR(".rpm %u\n"), fan_out[channel].rpm);
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
#endif
