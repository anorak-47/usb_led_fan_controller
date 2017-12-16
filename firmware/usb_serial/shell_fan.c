
#include "shell_fan.h"
#include "fan.h"
#include "sensor.h"
#include "ctrl.h"
#include "lufa_virtual_serial.h"
#include <inttypes.h>
#include <stdbool.h>


/*
 * Get/set fan configuration
 *
 * fan cfg
 * fan cfg #
 * fan cfg # fix   :: show fixed dutycycle configuration
 * fan cfg # lin   :: show linear controller configuration
 * fan cfg # tp    :: show trip point controller configuration
 * fan cfg # pi    :: show PI-controller configuration
 * fan cfg # sp    :: show setpoint configuraton for PI-controller / Fuzzy controller
 *
 * fan cfg # typ <u8> :: set type of fan (FANTYPE_*)
 * fan cfg # mod <u8> :: set controller to calculate a fan's dutycycle (FANMODE_*)
 * fan cfg # sns <u8> :: set index of associated sensor for controller (i. e. this is the sensor controlling the fan speed)
 * fan cfg # mrs <u8> :: set minimum number of revolutions per second, otherwise the fan will be seen as stalled.
 * fan cfg # min <u8> :: set minimum duty cycle
 * fan cfg # max <u8> :: set maximum duty cycle
 *
 * Controller FANMODE_FIXED_DUTY
 * fan cfg # fix <u8> :: set fixed duty cycle for controller FANMODE_FIXED_DUTY
 *
 * Controller FANMODE_LINEAR
 * fan cfg # gin <i16> :: set duty cycle gain for linear controller (FANMODE_LINEAR)
 * fan cfg # off <i16> :: set offset for linear controller
 *
 * Controller FANMODE_PI
 * fan cfg # ki <i8>   :: set Ki for PI-controller (FANMODE_PI)
 * fan cfg # kp <i8>   :: set Kp for PI-controller
 * fan cfg # kt <i8>   :: set Kt for PI-controller
 *
 * Controller FANMODE_PI, FAN_MODE_FUZZY_SUPPORTED
 * fan cfg # sps <u8>  :: set index of setpoint sensor for PI-controller
 * fan cfg # dlt <i8>  :: set setpoint sensor delta for PI-controller
 *
 * Controller FAN_MODE_TRIP_POINTS_SUPPORTED, FAN_MODE_LIN_TRIP_POINTS_SUPPORTED
 * fan cfg # tp <tp#> <i8> <u8>  :: set value and duty for trip point <tp#>
 *
 *
 * Show status of fan
 *
 * fan sta
 * fan sta #
 * fan sta # rpm  :: show current revolutions per minute for fan
 * fan sta # stl  :: show stall state for fan
 * fan sta # sns  :: show current value of sensor associated with this fan
 *
 *
 * Rest PI-Controller
 *
 * fan rst #
 *
 */

bool fan_cmd_cfg(uint8_t argc, char **argv);
bool fan_cmd_sta(uint8_t argc, char **argv);

#if FAN_MODE_PI_SUPPORTED
bool fan_cmd_rst(uint8_t argc, char **argv);
#endif

const struct _s_shell_cmd fan_shell_cmd[] PROGMEM = {SHELLCMD("cfg", fan_cmd_cfg, "#", "fan configuration"),
#if FAN_MODE_PI_SUPPORTED
                                                     SHELLCMD("rst", fan_cmd_rst, "#", "reset PI-controller"),
#endif
                                                     SHELLCMD("sta", fan_cmd_sta, "[# [rpm|stl|sns]]", "fan status"), SHELLCMD(0, 0, 0, 0)};

static void dump_fan_config(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".typ %u\n"), fans[fan].config.fanType);
    fprintf_P(_sf, PSTR(".mod %u\n"), fans[fan].config.fanMode);
    fprintf_P(_sf, PSTR(".sns %u\n"), fans[fan].config.snsIdx);
    fprintf_P(_sf, PSTR(".sty %u\n"), sns[fans[fan].config.snsIdx].type);
    fprintf_P(_sf, PSTR(".sva %u\n"), sns[fans[fan].config.snsIdx].status.valid);
    fprintf_P(_sf, PSTR(".min %u\n"), fans[fan].dutyMin);
    fprintf_P(_sf, PSTR(".max %u\n"), fans[fan].dutyMax);
    fprintf_P(_sf, PSTR(".mrs %u\n"), fans[fan].min_rps);
}

static void dump_fan_fixed_duty(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".fix %u\n"), fans[fan].dutyFixed);
}

#if FAN_MODE_TRIP_POINTS_SUPPORTED || FAN_MODE_LIN_TRIP_POINTS_SUPPORTED
static void dump_fan_trip_points(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    for (uint8_t t = 0; t < MAX_TRIP_POINTS; ++t)
        fprintf_P(_sf, PSTR(".tp %u %d %u\n"), t, fans[fan].trip_point[t].value, fans[fan].trip_point[t].duty);
}
#endif

#if FAN_MODE_LINEAR_SUPPORTED
static void dump_fan_linear_duty(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".gin %u\n"), fans[fan].dutyGain);
    fprintf_P(_sf, PSTR(".off %u\n"), fans[fan].dutyOffs);
}
#endif

#if FAN_MODE_PI_SUPPORTED
static void dump_fan_pi_control(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".Ki %d\n"), fans[fan].Ki);
    fprintf_P(_sf, PSTR(".Kp %d\n"), fans[fan].Kp);
    fprintf_P(_sf, PSTR(".Kt %d\n"), fans[fan].Kt);
    fprintf_P(_sf, PSTR(".es %d\n"), fans[fan].es);
    fprintf_P(_sf, PSTR(".i %d\n"), fans[fan].i);
#if CTRL_DEBUG
    fprintf_P(_sf, PSTR(".e %d\n"), fans_dbg[fan].e);
    fprintf_P(_sf, PSTR(".p %d\n"), fans_dbg[fan].p);
#endif
}
#endif

#if FAN_MODE_PI_SUPPORTED || FAN_MODE_FUZZY_SUPPORTED
static void dump_fan_setpoint(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".sns %u\n"), fans[fan].snsSetp.snsIdx);
    fprintf_P(_sf, PSTR(".dlt %d\n"), fans[fan].snsSetp.delta);
    fprintf_P(_sf, PSTR(".val %d\n"), sns[fans[fan].snsSetp.snsIdx].value);
    // setpoint: actual value; sum of reference sensor value & delta
    fprintf_P(_sf, PSTR(".stp %d\n"), sns[fans[fan].snsSetp.snsIdx].value + fans[fan].snsSetp.delta);
}
#endif

bool fan_cmd_cfg(uint8_t argc, char **argv)
{
    uint8_t fan = 0;

    if (argc > 0)
    {
        fan = atoi(argv[0]);

        if (fan >= MAX_FANS)
            return false;
    }

    if (argc == 0)
    {
        for (uint8_t i = 0; i < MAX_FANS; i++)
            dump_fan_config(i);
    }
    else if (argc == 1)
    {
        dump_fan_config(fan);
    }
    else if (argc == 2)
    {
        if (strcmp_P(PSTR("fix"), argv[1]) == 0)
        {
            dump_fan_fixed_duty(fan);
        }
#if FAN_MODE_LINEAR_SUPPORTED
        else if (strcmp_P(PSTR("lin"), argv[1]) == 0)
        {
            dump_fan_linear_duty(fan);
        }
#endif
#if FAN_MODE_TRIP_POINTS_SUPPORTED || FAN_MODE_LIN_TRIP_POINTS_SUPPORTED
        else if (strcmp_P(PSTR("tp"), argv[1]) == 0)
        {
            dump_fan_trip_points(fan);
        }
#endif
#if FAN_MODE_PI_SUPPORTED
        else if (strcmp_P(PSTR("pi"), argv[1]) == 0)
        {
            dump_fan_pi_control(fan);
        }
#endif
#if FAN_MODE_PI_SUPPORTED || FAN_MODE_FUZZY_SUPPORTED
        else if (strcmp_P(PSTR("sp"), argv[1]) == 0)
        {
            dump_fan_setpoint(fan);
        }
#endif
        else
        {
            return false;
        }
    }
    else if (argc == 3)
    {
        uint8_t val = atoi(argv[2]);

        return
#if FAN_MODE_LINEAR_SUPPORTED
            set_if_match_i16(argv[1], PSTR("gin"), val, &fans[fan].dutyGain) |
            set_if_match_i16(argv[1], PSTR("off"), val, &fans[fan].dutyOffs) |
#endif
#if FAN_MODE_PI_SUPPORTED
            set_if_match_i8(argv[1], PSTR("ki"), val, &fans[fan].Ki) | set_if_match_i8(argv[1], PSTR("kp"), val, &fans[fan].Kp) |
            set_if_match_i8(argv[1], PSTR("kt"), val, &fans[fan].Kt) |
#endif
#if FAN_MODE_PI_SUPPORTED || FAN_MODE_FUZZY_SUPPORTED
            set_if_match_8(argv[1], PSTR("sps"), val, &fans[fan].snsSetp.snsIdx) |
            set_if_match_i8(argv[1], PSTR("dlt"), val, &fans[fan].snsSetp.delta) |
#endif
            set_if_match_8(argv[1], PSTR("typ"), val, &fans[fan].config.fanType) |
            set_if_match_8(argv[1], PSTR("mod"), val, &fans[fan].config.fanMode) |
            set_if_match_8(argv[1], PSTR("sns"), val, &fans[fan].config.snsIdx) |
            set_if_match_8(argv[1], PSTR("mrs"), val, &fans[fan].min_rps) | set_if_match_8(argv[1], PSTR("min"), val, &fans[fan].dutyMin) |
            set_if_match_8(argv[1], PSTR("max"), val, &fans[fan].dutyMax) | set_if_match_8(argv[1], PSTR("fix"), val, &fans[fan].dutyFixed);
    }
#if FAN_MODE_TRIP_POINTS_SUPPORTED || FAN_MODE_LIN_TRIP_POINTS_SUPPORTED
    else if (argc == 5)
    {
        uint8_t t = atoi(argv[3]);

        if (t >= MAX_TRIP_POINTS)
            return false;

        int8_t v = atoi(argv[4]);
        uint8_t d = atoi(argv[5]);

        if (strcmp_P(PSTR("tp"), argv[2]) == 0)
        {
            fans[fan].trip_point[t].value = v;
            fans[fan].trip_point[t].duty = d;
        }
        else
        {
            return false;
        }
    }
#endif
    else
    {
        return false;
    }

    return true;
}

static void dump_fan_status(uint8_t fan)
{
    fprintf_P(_sf, PSTR(".fan %u\n"), fan);
    fprintf_P(_sf, PSTR(".dty %u\n"), fans[fan].duty);
    fprintf_P(_sf, PSTR(".rps %u\n"), fans[fan].rps);
    fprintf_P(_sf, PSTR(".rpm %u\n"), fans[fan].rpm);
    fprintf_P(_sf, PSTR(".stl %u\n"), fans[fan].status.stalled);
    fprintf_P(_sf, PSTR(".sns %u\n"), sns[fans[fan].config.snsIdx].value);
}

bool fan_cmd_sta(uint8_t argc, char **argv)
{
    uint8_t fan = 0;

    if (argc > 0)
    {
        fan = atoi(argv[0]);

        if (fan >= MAX_FANS)
            return false;
    }

    if (argc == 0)
    {
        for (uint8_t i = 0; i < MAX_FANS; i++)
            dump_fan_status(i);
    }
    else if (argc == 1)
    {
        dump_fan_status(fan);
    }

    else if (argc == 2)
    {
        if (strcmp_P(PSTR("rpm"), argv[1]) == 0)
        {
            fprintf_P(_sf, PSTR(".rpm %u\n"), fans[fan].rpm);
        }
        else if (strcmp_P(PSTR("stl"), argv[1]) == 0)
        {
            fprintf_P(_sf, PSTR(".stl %u\n"), fans[fan].status.stalled);
        }
        else if (strcmp_P(PSTR("sns"), argv[1]) == 0)
        {
            fprintf_P(_sf, PSTR(".sns %u\n"), sns[fans[fan].config.snsIdx].value);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

#if FAN_MODE_PI_SUPPORTED
bool fan_cmd_rst(uint8_t argc, char **argv)
{
    if (argc < 1)
        return false;

    uint8_t fan = atoi(argv[0]);

    if (fan >= MAX_FANS)
        return false;

    restartCtrl(fan);

    return true;
}
#endif
