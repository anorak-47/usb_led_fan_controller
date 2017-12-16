
#include "shell_max31790.h"
#include "lufa_virtual_serial.h"
#include "max31790.h"
#include <inttypes.h>
#include <stdbool.h>

#if FAN_PWM_MAX31790_SUPPORTED & DEBUG_FUNCTIONS_SUPPORTED

bool mx_cmd_reset(uint8_t argc, char **argv);
bool mx_cmd_standby(uint8_t argc, char **argv);
bool mx_cmd_update(uint8_t argc, char **argv);
bool mx_cmd_cfg(uint8_t argc, char **argv);
bool mx_cmd_fan(uint8_t argc, char **argv);

const struct _s_shell_cmd max_shell_cmd[] PROGMEM = {SHELLCMD("rst", mx_cmd_reset, "", "reset"),
                                                     SHELLCMD("sby", mx_cmd_standby, "0|1", "standby"),
                                                     SHELLCMD("upd", mx_cmd_update, "", "update tach/rpm"),
                                                     SHELLCMD("cfg", mx_cmd_cfg, "0|1", "configuration"),
                                                     SHELLCMD("sat", mx_cmd_fan, "0|1", "fan status"),
                                                     SHELLCMD(0, 0, 0, 0)};

bool mx_cmd_cfg(uint8_t argc, char **argv)
{
    if (argc == 2)
    {
        //uint8_t val = atoi(argv[1]);
        /*
        return set_if_match_8(argv[0], PSTR("osc"), val, &max31790device.config.config.ext_OSC) |
               set_if_match_8(argv[0], PSTR("wdt"), val, &max31790device.config.config.i2c_WD_time) |
               set_if_match_8(argv[0], PSTR("wdt"), val, &max31790device.config.config.i2c_bus_timeout);
		*/
        max31790_configure(&max31790device, &max31790device.config);
    }
    else
    {
    	fprintf_P(_sf, PSTR(".cfg\n"));
        fprintf_P(_sf, PSTR(".raw %b\n"), max31790device.config.config.raw);
        fprintf_P(_sf, PSTR(".OSC %u\n"), max31790device.config.config.ext_OSC);
        fprintf_P(_sf, PSTR(".WDt %u\n"), max31790device.config.config.i2c_WD_time);
        fprintf_P(_sf, PSTR(".ibt %u\n"), max31790device.config.config.i2c_bus_timeout);
        fprintf_P(_sf, PSTR(".WDf %u\n"), max31790device.config.config.wd_fault);
        fprintf_P(_sf, PSTR(".frq %u\n"), max31790device.config.fan_freq.raw);
    }

    return true;
}

void dump_fan_config(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".raw %b\n"), max31790device.fan_config[fan].fan_config.raw);
    fprintf_P(_sf, PSTR(".mod %u\n"), max31790device.fan_config[fan].fan_config.mode);
    fprintf_P(_sf, PSTR(".spi %u\n"), max31790device.fan_config[fan].fan_config.spin_up);
    fprintf_P(_sf, PSTR(".ien %u\n"), max31790device.fan_config[fan].fan_config.tach_input_en);
    fprintf_P(_sf, PSTR(".pwm %u\n"), max31790device.fan_config[fan].fan_config.pwm_tach);
    fprintf_P(_sf, PSTR(".dyn %b\n"), max31790device.fan_config[fan].fan_dynamics.raw);
}

void dump_fan_status(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".tac %u\n"), max31790device.fan_status[fan].tach[0]);
    fprintf_P(_sf, PSTR(".tac %u\n"), max31790device.fan_status[fan].tach[1]);
    fprintf_P(_sf, PSTR(".dty %u\n"), max31790device.fan_status[fan].duty);
    fprintf_P(_sf, PSTR(".pwm %u\n"), max31790device.fan_status[fan].pwm);
    fprintf_P(_sf, PSTR(".sr %u\n"), max31790device.fan_status[fan].sr);

    fprintf_P(_sf, PSTR(".rpm %u\n"), max31790_get_fan_rpm(&max31790device, fan));
    fprintf_P(_sf, PSTR(".pwm %u\n"), max31790_get_fan_pwm(&max31790device, fan));
}

void dump_fan_duty(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".rpm %u\n"), max31790_get_fan_rpm(&max31790device, fan));
    fprintf_P(_sf, PSTR(".rmt %u\n"), max31790_get_fan_rpm_target(&max31790device, fan));
    fprintf_P(_sf, PSTR(".pwm %u\n"), max31790_get_fan_pwm(&max31790device, fan));
    fprintf_P(_sf, PSTR(".dty %u\n"), max31790_get_fan_duty(&max31790device, fan));
}

void dump_fan_mode(uint8_t fan)
{
	fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".mod %u\n"), max31790_get_fan_mode(&max31790device, fan));
}

bool mx_cmd_reset(uint8_t argc, char **argv)
{
    max31790_reset(&max31790device);
    return true;
}

bool mx_cmd_standby(uint8_t argc, char **argv)
{
    if (argc == 0)
        return false;

    bool standby = atoi(argv[0]);
    max31790_standby(&max31790device, standby);

    return true;
}

bool mx_cmd_update(uint8_t argc, char **argv)
{
    if (argc == 0)
    {
        max31790_read_fan_states(&max31790device);
    }
    else if (argc == 1)
    {
        uint8_t fan = atoi(argv[0]);
        if (fan >= NR_CHANNEL)
            return false;

        max31790_read_fan_status(&max31790device, fan);
    }
    else
    {
        return false;
    }

    return true;
}

bool mx_cmd_fan(uint8_t argc, char **argv)
{
    bool success = true;

    if (argc == 0)
    {
        for (uint8_t i = 0; i < NR_CHANNEL; i++)
            dump_fan_status(i);
    }
    else if (argc == 1)
    {
        uint8_t fan = atoi(argv[0]);
        if (fan >= NR_CHANNEL)
            return false;
        dump_fan_status(fan);
    }
    else if (argc == 2)
    {
        uint8_t fan = atoi(argv[0]);

        if (fan >= NR_CHANNEL)
            return false;

        if (strcmp_P(PSTR("cfg"), argv[1]) == 0)
        {
            dump_fan_config(fan);
        }
        else if (strcmp_P(PSTR("mod"), argv[1]) == 0)
        {
            dump_fan_mode(fan);
        }
        else
        {
            success = false;
        }
    }
    else if (argc == 3)
    {
        uint8_t fan = atoi(argv[0]);
        uint16_t val = atoi(argv[2]);

        if (fan >= NR_CHANNEL)
            return false;

        if (strcmp_P(PSTR("mod"), argv[1]) == 0)
        {
            max31790_set_fan_mode(&max31790device, val, fan);
            dump_fan_mode(fan);
        }
        else if (strcmp_P(PSTR("pwm"), argv[1]) == 0)
        {
            max31790_set_fan_pwm(&max31790device, val, fan);
            dump_fan_duty(fan);
        }
        else if (strcmp_P(PSTR("rpm"), argv[1]) == 0)
        {
            max31790_set_fan_rpm(&max31790device, val, fan);
            dump_fan_duty(fan);
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
