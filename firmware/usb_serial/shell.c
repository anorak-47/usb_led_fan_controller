
#include <ledstripe/animation.h>
#include "shell.h"
#include "fan_out_type.h"
#include "fan_type.h"
#include "i2c.h"
#include "led.h"
#include "lufa_virtual_serial.h"
#include "max31790.h"
#include "sensor_type.h"
#include "settings.h"
#include "uart.h"
#include "shell_fan.h"
#include "shell_fan_out.h"
#include "shell_ina219.h"
#include "shell_ina260.h"
#include "shell_powermeter.h"
#include "shell_max31790.h"
#include "shell_animation.h"
#include "shell_sensor.h"

#define VIRTSER_ENABLE_ECHO
//#define VIRT_SER_PRINTF_BUFFER_SIZE 80

#define DATAGRAM_USER_START ':'
#define DATAGRAM_USER_STOP '\n'

#define MAX_MSG_LENGTH 32

uint8_t recv_buffer[MAX_MSG_LENGTH];

enum recvStatus
{
    recvStatusIdle = 0,
    recvStatusFoundUserStart = 2,
    recvStatusFindStop = 4
};

enum recvStatus recv_status = recvStatusIdle;

bool cmd_nfo(uint8_t argc, char **argv);
bool cmd_sns(uint8_t argc, char **argv);
bool cmd_fan(uint8_t argc, char **argv);
bool cmd_save_settings(uint8_t argc, char **argv);
bool cmd_load_settings(uint8_t argc, char **argv);
bool shell_command_help(uint8_t argc, char **argv);

#if FAN_OUT_SUPPORTED
bool cmd_fao(uint8_t argc, char **argv);
#endif

#if DEBUG_FUNCTIONS_SUPPORTED
bool cmd_i2c(uint8_t argc, char **argv);
bool cmd_dbg(uint8_t argc, char **argv);
#endif

#if FAN_PWM_MAX31790_SUPPORTED && DEBUG_FUNCTIONS_SUPPORTED
bool cmd_mx(uint8_t argc, char **argv);
#endif

#if FASTLED_SUPPORTED
bool cmd_fastled(uint8_t argc, char **argv);
#endif

#if POWER_METER_SUPPORTED && POWER_METER_INA219 && DEBUG_FUNCTIONS_SUPPORTED
bool cmd_ina219(uint8_t argc, char **argv);
#endif

#if POWER_METER_SUPPORTED && POWER_METER_INA260 && DEBUG_FUNCTIONS_SUPPORTED
bool cmd_ina260(uint8_t argc, char **argv);
#endif

#if POWER_METER_SUPPORTED
bool cmd_powermeter(uint8_t argc, char **argv);
#endif

#if LED_PCA8574_SUPPORTED && DEBUG_FUNCTIONS_SUPPORTED
bool cmd_led(uint8_t argc, char **argv);
#endif

const struct _s_shell_cmd shell_cmd[] PROGMEM = {SHELLCMD("?", shell_command_help, "", "help"),
                                                 SHELLCMD("nfo", cmd_nfo, "", "info"),
#if DEBUG_FUNCTIONS_SUPPORTED
                                                 SHELLCMD("dbg", cmd_dbg, "", "debug"),
                                                 SHELLCMD("i2c", cmd_i2c, "", "scan i2c bus"),
#endif
#if FAN_CONTROL_SUPPORTED
                                                 SHELLCMD("fan", cmd_fan, "", "fan"),
                                                 SHELLCMD("sns", cmd_sns, "", "sensor"),
#endif
#if FAN_OUT_SUPPORTED
                                                 SHELLCMD("fao", cmd_fao, "", "fan out"),
#endif
#if LED_PCA8574_SUPPORTED && DEBUG_FUNCTIONS_SUPPORTED
                                                 SHELLCMD("led", cmd_led, "", "led"),
#endif
#if FASTLED_SUPPORTED
                                                 SHELLCMD("fld", cmd_fastled, "", "fastled"),
#endif
#if FAN_CONTROL_SUPPORTED && FAN_PWM_MAX31790_SUPPORTED && DEBUG_FUNCTIONS_SUPPORTED
                                                 SHELLCMD("mx", cmd_mx, "", "max31790"),
#endif
#if POWER_METER_SUPPORTED && POWER_METER_INA219 && DEBUG_FUNCTIONS_SUPPORTED
                                                 SHELLCMD("219", cmd_ina219, "", "ina219"),
#endif
#if POWER_METER_SUPPORTED && POWER_METER_INA260 && DEBUG_FUNCTIONS_SUPPORTED
                                                 SHELLCMD("260", cmd_ina260, "", "ina260"),
#endif
#if POWER_METER_SUPPORTED
												 SHELLCMD("pmt", cmd_powermeter, "", "powermeter"),
#endif
                                                 SHELLCMD("load", cmd_load_settings, "", "load settings"),
                                                 SHELLCMD("save", cmd_save_settings, "", "save settings"),
                                                 SHELLCMD(0, 0, 0, 0)};

bool cmd_nfo(uint8_t argc, char **argv)
{
    fprintf_P(_vsf, PSTR(".info\n"));
    fprintf_P(_vsf, PSTR(".sns %u\n"), MAX_SNS);
    fprintf_P(_vsf, PSTR(".fan %u\n"), MAX_FANS);
    fprintf_P(_vsf, PSTR(".fao %u\n"), MAX_FAN_OUTS);
    fprintf_P(_vsf, PSTR(".pwr %u\n"), MAX_POWERMETER);
    fprintf_P(_vsf, PSTR(".led %u\n"), 4);
    return true;
}

#if DEBUG_FUNCTIONS_SUPPORTED
int freeRam(void)
{
    // https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

bool cmd_dbg(uint8_t argc, char **argv)
{
	uint16_t size = 0;
    fprintf_P(_vsf, PSTR(".dbg\n"));
    fprintf_P(_vsf, PSTR(".sns %u\n"), sizeof(Sensor) * MAX_SNS);
    size += sizeof(Sensor) * MAX_SNS;
    fprintf_P(_vsf, PSTR(".fan %u\n"), sizeof(Fan) * MAX_FANS);
    size += sizeof(Fan) * MAX_FANS;

#if FAN_OUT_SUPPORTED
    fprintf_P(_vsf, PSTR(".fao %u\n"), sizeof(Fan_out) * MAX_FAN_OUTS);
    size += sizeof(Fan_out) * MAX_FAN_OUTS;
#endif

#if FAN_PWM_MAX31790_SUPPORTED
    fprintf_P(_vsf, PSTR(".mx3 %u\n"), sizeof(struct _s_max31790_device));
    size += sizeof(struct _s_max31790_device);
#endif

    fprintf_P(_vsf, PSTR(".tot %u\n"), size);

    fprintf_P(_vsf, PSTR(".ram %d\n"), freeRam());
    return true;
}

bool cmd_i2c(uint8_t argc, char **argv)
{
    if (argc == 1)
    {
        uint8_t addr = atoi(argv[0]);
        fprintf_P(_vsf, PSTR("dev at %u 0x%x: %u\n"), addr, addr, i2c_scan_address(addr));
    }
    else
    {
        for (uint8_t addr = 1; addr < 128; addr++)
        {
        	uint8_t waddr = addr << 1;
        	if (i2c_scan_address(waddr))
        		fprintf_P(_vsf, PSTR("dev at %u 0x%x\n"), waddr, waddr);
        }
    }

    return true;
}
#endif

#if FAN_CONTROL_SUPPORTED
bool cmd_sns(uint8_t argc, char **argv)
{
    return sub_shell_command(sensor_shell_cmd, argc, argv);
}

#if FAN_PWM_MAX31790_SUPPORTED && DEBUG_FUNCTIONS_SUPPORTED
bool cmd_mx(uint8_t argc, char **argv)
{
    return sub_shell_command(max_shell_cmd, argc, argv);
}
#endif

bool cmd_fan(uint8_t argc, char **argv)
{
    return sub_shell_command(fan_shell_cmd, argc, argv);
}
#endif

#if FAN_OUT_SUPPORTED
bool cmd_fao(uint8_t argc, char **argv)
{
    return sub_shell_command(fan_out_shell_cmd, argc, argv);
}
#endif

#if POWER_METER_SUPPORTED & POWER_METER_INA219 & DEBUG_FUNCTIONS_SUPPORTED
bool cmd_ina219(uint8_t argc, char **argv)
{
    return sub_shell_command(ina219_shell_cmd, argc, argv);
}
#endif

#if POWER_METER_SUPPORTED & POWER_METER_INA260 & DEBUG_FUNCTIONS_SUPPORTED
bool cmd_ina260(uint8_t argc, char **argv)
{
    return sub_shell_command(ina260_shell_cmd, argc, argv);
}
#endif

#if POWER_METER_SUPPORTED
bool cmd_powermeter(uint8_t argc, char **argv)
{
    return sub_shell_command(powermeter_shell_cmd, argc, argv);
}
#endif

#if LED_PCA8574_SUPPORTED && DEBUG_FUNCTIONS_SUPPORTED
bool cmd_led(uint8_t argc, char **argv)
{
    if (argc != 2)
        return false;

    uint8_t led = atoi(argv[0]);
    bool on = atoi(argv[1]);

    if (on)
        set_led(_BV(led));
    else
        unset_led(_BV(led));

    return true;
}
#endif

#if FASTLED_SUPPORTED
bool cmd_fastled(uint8_t argc, char **argv)
{
    return sub_shell_command(animation_shell_cmd, argc, argv);
}
#endif

bool cmd_save_settings(uint8_t argc, char **argv)
{
#if FAN_CONTROL_SUPPORTED
    saveSettings();
#endif
#if FASTLED_SUPPORTED
    animation_save();
#endif
    return true;
}

bool cmd_load_settings(uint8_t argc, char **argv)
{
    uint8_t clear = 0;
    if (argc > 0)
        clear = atoi(argv[0]);

#if FAN_CONTROL_SUPPORTED
    loadSettings(clear);
#endif
#if FASTLED_SUPPORTED
    animation_load(clear);
#endif
    return true;
}

bool shell_command_help(uint8_t argc, char **argv)
{
    uint8_t pos = 0;
    struct _s_shell_cmd cmd;

    memcpy_P(&cmd, (const void *)&shell_cmd[pos++], sizeof(struct _s_shell_cmd));

    while (cmd.name)
    {
#if defined(SHELL_ARGS_HELP_SUPPORTED) && defined(SHELL_HELP_SUPPORTED)
        fprintf_P(_vsf, PSTR("%s [%s]: %s\n"), cmd.name, cmd.args_help, cmd.cmd_help);
#else
#ifdef SHELL_ARGS_HELP_SUPPORTED
        fprintf_P(_vsf, PSTR("%s [%s]\n"), cmd.name, cmd.args_help);
#else
#ifdef SHELL_HELP_SUPPORTED
        fprintf_P(_vsf, PSTR("%s: %s\n"), cmd.name, cmd.cmd_help);
#else
        fprintf_P(_vsf, PSTR("%s\n"), cmd.name);
#endif
#endif
#endif

        memcpy_P(&cmd, (const void *)&shell_cmd[pos++], sizeof(struct _s_shell_cmd));
    }

    fprintf_P(_vsf, PSTR("\n"));
    return true;
}

bool set_if_match_8(const char *arg, const char *cmd, uint8_t value, uint8_t *variable)
{
    if (strcmp_P(arg, cmd) == 0)
    {
        *variable = value;
        return true;
    }

    return false;
}

bool set_if_match_i8(const char *arg, const char *cmd, int8_t value, int8_t *variable)
{
    if (strcmp_P(arg, cmd) == 0)
    {
        *variable = value;
        return true;
    }

    return false;
}

bool set_if_match_i16(const char *arg, const char *cmd, int16_t value, int16_t *variable)
{
    if (strcmp_P(arg, cmd) == 0)
    {
        *variable = value;
        return true;
    }

    return false;
}

bool datagram_is_userdata_start(uint8_t ucData)
{
    return (ucData == DATAGRAM_USER_START && recv_status == recvStatusIdle);
}

bool sub_shell_command(const struct _s_shell_cmd *sub_cmd, uint8_t argc, char **argv)
{
    if (argc == 0)
        return false;

    struct _s_shell_cmd shell_cmd;
    const char *cmd = argv[0];
    uint8_t cmdid = 0;

    memcpy_P(&shell_cmd, &sub_cmd[cmdid++], sizeof(struct _s_shell_cmd));

    while (shell_cmd.name != 0)
    {
        // fprintf_P(_sf, PSTR("subc %s\n"), shell_cmd.name);
        if (strcmp(cmd, shell_cmd.name) == 0)
        {
            return shell_cmd.func(argc - 1, argv + 1);
        }

        memcpy_P(&shell_cmd, &sub_cmd[cmdid++], sizeof(struct _s_shell_cmd));
    }

    return false;
}

void shell_command(uint8_t *buffer, uint8_t length)
{
    uint8_t pos = 0;
    char *str = (char *)buffer;
    char *token;
    char *command;
    uint8_t argc = 0;
    char *argv[10];
    struct _s_shell_cmd user_command;

    // dprintf("buffer: <%s> l:%u\n", buffer, length);
    command = strsep(&str, " ");

    while ((token = strsep(&str, " ")))
    {
        argv[argc] = token;
        argc++;
    }

    memcpy_P(&user_command, &shell_cmd[pos++], sizeof(struct _s_shell_cmd));

    // dprintf("uc: [%s] [%s]\n", user_command.cmd, user_command.help_msg);

    while (user_command.name)
    {
        if (strcmp(user_command.name, command) == 0)
        {
            // dump_args(argc, argv);

            bool success = user_command.func(argc, argv);

            if (success)
            {
                fprintf_P(_vsf, PSTR(">OK\n"));
            }
            else
            {
                fprintf_P(_vsf, PSTR(">ERR\n"));
            }

            break;
        }

        memcpy_P(&user_command, &shell_cmd[pos++], sizeof(struct _s_shell_cmd));

        // dprintf("uc: [%s] [%s]\n", user_command.cmd, user_command.help_msg);
    }

    if (!user_command.func)
    {
        fprintf_P(_vsf, PSTR(">NC ':?' for help\n"));
        fprintf_P(_vsf, PSTR(">NC\n"));
    }
}

void virtser_recv(uint8_t ucData)
{
    static uint8_t buffer_pos = 0;

#ifdef VIRTSER_ENABLE_ECHO
    fputc(ucData, _vsf);
#endif

    // dprintf("recv: [%02X] <%c> s:%u\n", ucData, (char)ucData, recv_status);

    if (datagram_is_userdata_start(ucData))
    {
        // dprintf("recv: user start\n");

        buffer_pos = 0;

        // do not store the user command start byte
        // recv_buffer[buffer_pos] = ucData;
        // buffer_pos++;

        recv_status = recvStatusFoundUserStart;
    }
    else if (recv_status == recvStatusFoundUserStart)
    {
        if (ucData == DATAGRAM_USER_STOP)
        {
            // dprintf("recv: user stop\n");
            recv_buffer[buffer_pos] = '\0';
            shell_command(recv_buffer, buffer_pos);

            buffer_pos = 0;
            recv_status = recvStatusIdle;
        }
        else if (buffer_pos + 1 >= MAX_MSG_LENGTH)
        {
            // bail out
            buffer_pos = 0;
            recv_status = recvStatusIdle;
            // print("recv: payload too long\n");
            fprintf_P(_vsf, PSTR(">ERR payload\n"));
            fprintf_P(_vsf, PSTR(">ERR\n"));
        }

        else
        {
            recv_buffer[buffer_pos] = ucData;
            buffer_pos++;
        }
    }
}

#if 0
#define LOW_BYTE(x)        	(x & 0xff)					// 16Bit 	--> 8Bit
#define HIGH_BYTE(x)       	((x >> 8) & 0xff)			// 16Bit 	--> 8Bit

void uart_recv_task(void)
{
    uint8_t ucData;
    unsigned int rd = uart_getc();

    // wenn Zeichen empfangen
    if (HIGH_BYTE(rd) == 0)
    {
        ucData = LOW_BYTE(rd);

        virtser_recv(ucData);
    }
}
#endif
