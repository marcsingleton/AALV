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

char *cmd_read_command_line(int input_fd, char *prompt);
int cmd_parse_command_line(char *line, int *argc, char *argv[CMD_ARG_MAX]);
int cmd_execute_command(State *state, int argc, char *argv[CMD_ARG_MAX]);
int cmd_parse_execute_command_line(State *state, char *line);

int cmd_init_command_map(void);
void cmd_deinit_command_map(void);

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
