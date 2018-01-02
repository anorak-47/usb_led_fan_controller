#ifndef __SHELL_H_INCLUDED__
#define __SHELL_H_INCLUDED__

#include "config.h"
#include <inttypes.h>
#include <stdbool.h>

//#define SHELL_ARGS_HELP_SUPPORTED
//#define SHELL_HELP_SUPPORTED

struct _s_shell_cmd
{
    char *name;
    bool (*func)(uint8_t argc, char **argv);
#ifdef SHELL_ARGS_HELP_SUPPORTED
    char *args_help;
#endif
#ifdef SHELL_HELP_SUPPORTED
    char *cmd_help;
#endif
};

#if defined(SHELL_ARGS_HELP_SUPPORTED) && defined(SHELL_HELP_SUPPORTED)
#define SHELLCMD(arg_name, arg_func, arg_args, arg_help) \
		{ arg_name, arg_func, arg_args, arg_help }
#else
#if defined(SHELL_ARGS_HELP_SUPPORTED)
#define SHELLCMD(arg_name, arg_func, arg_args, arg_help) \
		{ arg_name, arg_func, arg_args }
#else
#if defined(SHELL_HELP_SUPPORTED)
#define SHELLCMD(arg_name, arg_func, arg_args, arg_help) \
		{ arg_name, arg_func, arg_help }
#else
#define SHELLCMD(arg_name, arg_func, arg_args, arg_help) \
		{ arg_name, arg_func }
#endif
#endif
#endif

bool sub_shell_command(const struct _s_shell_cmd *sub_cmd, uint8_t argc, char **argv);

void uart_recv_task(void);

bool set_if_match_8(const char *arg, const char *cmd, uint8_t value, uint8_t *variable);
bool set_if_match_i8(const char *arg, const char *cmd, int8_t value, int8_t *variable);
bool set_if_match_i16(const char *arg, const char *cmd, int16_t value, int16_t *variable);

#endif
