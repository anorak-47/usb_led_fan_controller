
#include "shell_animation.h"
#include "debug.h"
#include "lufa_virtual_serial.h"
#include <inttypes.h>
#include <ledstripe/animation.h>
#include <stdbool.h>

#if FASTLED_SUPPORTED

bool animation_cmd_status(uint8_t argc, char **argv);
bool animation_cmd_run(uint8_t argc, char **argv);
bool animation_cmd_configuration(uint8_t argc, char **argv);
bool animation_cmd_global_configuration(uint8_t argc, char **argv);

#ifdef DEBUG_FUNCTIONS_SUPPORTED
bool animation_cmd_settings(uint8_t argc, char **argv);
#endif

const struct _s_shell_cmd animation_shell_cmd[] PROGMEM = {
    SHELLCMD("sta", animation_cmd_status, "#", "animation status"),
    SHELLCMD("run", animation_cmd_run, "# [0/1]", "start/stop animation"),
    SHELLCMD("cfg", animation_cmd_configuration, "# ani|fps|apl|sns|opt #", "animation configuration"),
    SHELLCMD("glo", animation_cmd_global_configuration, "[bri #]", "global animation configuration"),
#ifdef DEBUG_FUNCTIONS_SUPPORTED
    SHELLCMD("set", animation_cmd_settings, "[#] load|save", "load/save animation settings"),
#endif
    SHELLCMD(0, 0, 0, 0)};

void animation_dump_status(uint8_t stripe)
{
    fprintf_P(_vsf, PSTR(".str %u\n"), stripe);
    fprintf_P(_vsf, PSTR(".ani %u\n"), animation_get_current(stripe));
    fprintf_P(_vsf, PSTR(".run %u\n"), animation_get_running(stripe));
    fprintf_P(_vsf, PSTR(".apl %u\n"), animation_get_autoplay(stripe));
    fprintf_P(_vsf, PSTR(".fps %u\n"), animation_get_current_fps(stripe));
    fprintf_P(_vsf, PSTR(".sns %u\n"), animation_get_current_sensor_index(stripe));
    fprintf_P(_vsf, PSTR(".opt %u\n"), animation_get_current_option(stripe));

    uint8_t hsv[3];
    animation_get_current_color(stripe, 0, hsv);
    fprintf_P(_vsf, PSTR(".co0 %x %x %x\n"), hsv[0], hsv[1], hsv[2]);
    animation_get_current_color(stripe, 1, hsv);
    fprintf_P(_vsf, PSTR(".co1 %x %x %x\n"), hsv[0], hsv[1], hsv[2]);
}

#ifdef DEBUG_FUNCTIONS_SUPPORTED
bool animation_cmd_settings(uint8_t argc, char **argv)
{
    if (argc == 0)
        return false;

    if (argc == 1)
    {
        if (strcmp_P(argv[0], PSTR("load")) == 0)
        {
            animation_load(false);
        }
        else if (strcmp_P(argv[0], PSTR("clear")) == 0)
        {
            animation_load(true);
        }
        else
        {
            animation_save();
        }
    }
    else
    {
        uint8_t stripe = atoi(argv[0]);
        if (stripe >= MAX_LED_STRIPES)
            return false;

        if (strcmp_P(argv[1], PSTR("load")) == 0)
        {
            animation_load_stripe(stripe);
        }
        else
        {
            animation_save_stripe(stripe);
        }

        fprintf_P(_vsf, PSTR(".str %u\n"), stripe);
    }

    return true;
}
#endif

bool animation_cmd_global_configuration(uint8_t argc, char **argv)
{
    if (argc == 0)
    {
        fprintf_P(_vsf, PSTR(".bri %u\n"), animation_get_global_brightness());
    }

    if (argc == 2)
    {
        uint8_t value = atoi(argv[1]);

        if (strcmp_P(argv[0], PSTR("bri")) == 0)
        {
            animation_set_global_brightness(value);
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

bool animation_cmd_configuration(uint8_t argc, char **argv)
{
    if (argc < 3)
        return false;

    uint8_t stripe = atoi(argv[0]);
    if (stripe >= MAX_LED_STRIPES)
        return false;

    if (argc == 3)
    {
        uint8_t argv3 = atoi(argv[2]);

        LV_("cmd %u %s %u", stripe, argv[1], argv3);

        LV_("cmd %u", strcmp_P(argv[1], PSTR("ani")));
        LV_("cmd %u", strcmp_P(argv[1], PSTR("fps")));

        if (strcmp_P(argv[1], PSTR("ani")) == 0)
        {
            LS_("cmd ani");
            animation_set_current(stripe, argv3);
        }

        else if (strcmp_P(argv[1], PSTR("fps")) == 0)
        {
            LS_("cmd fps");
            animation_set_current_fps(stripe, argv3);
        }

        else if (strcmp_P(argv[1], PSTR("apl")) == 0)
        {
            animation_set_autoplay(stripe, argv3);
        }

        else if (strcmp_P(argv[1], PSTR("sns")) == 0)
        {
            animation_set_current_sensor_index(stripe, argv3);
        }

        else if (strcmp_P(argv[1], PSTR("opt")) == 0)
        {
            animation_set_current_option(stripe, argv3);
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

    animation_dump_status(stripe);

    return true;
}

bool animation_cmd_run(uint8_t argc, char **argv)
{
    if (argc < 1)
        return false;

    uint8_t stripe = atoi(argv[0]);
    if (stripe >= MAX_LED_STRIPES)
        return false;

    if (argc >= 2)
    {
        bool run = atoi(argv[1]);
        animation_set_running(stripe, run);
    }

    fprintf_P(_vsf, PSTR(".str %u\n"), stripe);
    fprintf_P(_vsf, PSTR(".run %u\n"), animation_get_running(stripe));

    return true;
}

bool animation_cmd_status(uint8_t argc, char **argv)
{
    if (argc != 1)
        return false;

    uint8_t stripe = atoi(argv[0]);
    if (stripe >= MAX_LED_STRIPES)
        return false;

    animation_dump_status(stripe);

    return true;
}

#endif
