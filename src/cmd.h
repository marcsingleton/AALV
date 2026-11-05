#ifndef CMD_H
#define CMD_H

/*
 * Command parsing
 */

#include "state.h"

#define CMD_ARG_MAX 32
#define CMD_ALPHABET "abcdefghijklmnopqrstuvwxyz"

typedef struct
{
    int (*fn_ptr)(State *, int, char **);
    char *name;
} Command;

typedef enum
{
    CMD_ESUCCESS = 0,
    CMD_EARGNUM,   // Incorrect number of arguments
    CMD_EINVAL,    // Invalid argument(s)
    CMD_EINTPARSE, // Argument not valid integer
    CMD_ERANGE,    // Value out of range
    CMD_ENOMEM,    // Memory allocation failed
    CMD_EARGMAX,   // Exceeded maximum number of arguments
    CMD_EUNKCMD,   // Unknown command
    CMD_EUNKARG,   // Unknown option or subcommand
    CMD_ESEQTYPE,  // Unknown sequence type
    CMD_ESCHEME,   // Unknown color scheme
    CMD_EFORMAT,   // Unknown file format
    CMD_EOPEN,     // Failed to open file
    CMD_ENOPATH,   // No file path available
    CMD_EFILEOP,   // Failure in internal file operation
} CommandError;

char *cmd_read_command_line(int input_fd, char *prompt);
int cmd_parse_command_line(char *line, int *argc, char *argv[CMD_ARG_MAX]);
int cmd_execute_command(State *state, int argc, char *argv[CMD_ARG_MAX]);
int cmd_parse_execute_command_line(State *state, char *line);

int cmd_init_command_map(void);
void cmd_deinit_command_map(void);

char *cmd_strerror(int errnum);

int cmd_quit(State *state, int argc, char **argv);
int cmd_quitall(State *state, int argc, char **argv);
int cmd_next_file(State *state, int argc, char **argv);
int cmd_previous_file(State *state, int argc, char **argv);
int cmd_set(State *state, int argc, char **argv);
int cmd_type(State *state, int argc, char **argv);
int cmd_scheme(State *state, int argc, char **argv);
int cmd_config(State *state, int argc, char **argv);
int cmd_edit(State *state, int argc, char **argv);
int cmd_write(State *state, int argc, char **argv);

#endif // CMD_H
