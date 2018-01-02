
#include "shell_animation.h"
#include "animation.h"
#include "lufa_virtual_serial.h"
#include <inttypes.h>
#include <stdbool.h>

#if FASTLED_SUPPORTED

bool animation_cmd_status(uint8_t argc, char **argv);
bool animation_cmd_run(uint8_t argc, char **argv);
bool animation_cmd_configuration(uint8_t argc, char **argv);

const struct _s_shell_cmd animation_shell_cmd[] PROGMEM = {
	SHELLCMD("sta", animation_cmd_status, "#", "animation status"),
	SHELLCMD("run", animation_cmd_run, "# 0/1", "start/stop animation"),
	SHELLCMD("cfg", animation_cmd_configuration, "# ani|fps|apl #", "animation configuration"),
	SHELLCMD(0, 0, 0, 0)};


void animation_dump_status(uint8_t stripe)
{
    fprintf_P(_vsf, PSTR(".sti %u\n"), stripe);
    fprintf_P(_vsf, PSTR(".ani %u\n"), animation_get_current(stripe));
    fprintf_P(_vsf, PSTR(".run %u\n"), animation_get_current_running(stripe));
    fprintf_P(_vsf, PSTR(".fps %u\n"), animation_get_current_fps(stripe));
    fprintf_P(_vsf, PSTR(".apl %u\n"), animation_get_autoplay(stripe));
    fprintf_P(_vsf, PSTR(".sns %u\n"), animation_get_current_sensor_index(stripe));

    uint8_t hsv[3];
    animation_get_current_color(stripe, 0, hsv);
    fprintf_P(_vsf, PSTR(".co0 %x %x %x\n"), hsv[0], hsv[1], hsv[2]);
    animation_get_current_color(stripe, 1, hsv);
    fprintf_P(_vsf, PSTR(".co1 %x %x %x\n"), hsv[0], hsv[1], hsv[2]);
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
		if (strcmp_P(PSTR("ani"), argv[1]) == 0)
		{
			animation_set_current(stripe, argv3);
		}

		else if (strcmp_P(PSTR("fps"), argv[1]) == 0)
		{
			animation_set_current_fps(stripe, argv3);
		}

		else if (strcmp_P(PSTR("apl"), argv[1]) == 0)
		{
			animation_set_autoplay(stripe, argv3);
		}

		else if (strcmp_P(PSTR("sns"), argv[1]) == 0)
		{
			animation_set_current_sensor_index(stripe, argv3);
		}

		else
		{
			return false;
		}
    }

    animation_dump_status(stripe);

    return true;
}
bool animation_cmd_run(uint8_t argc, char **argv)
{
    if (argc != 2)
        return false;

    uint8_t stripe = atoi(argv[0]);
    if (stripe >= MAX_LED_STRIPES)
        return false;

    bool run = atoi(argv[1]);

    animation_set_running(stripe, run);

    fprintf_P(_vsf, PSTR(".sti %u\n"), stripe);
    fprintf_P(_vsf, PSTR(".run %u\n"), animation_get_current_running(stripe));

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
