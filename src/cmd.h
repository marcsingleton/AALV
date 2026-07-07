#ifndef CMD_H
#define CMD_H

/*
 * Command parsing
 */

#include "state.h"

#define CMD_ARG_MAX 8
#define CMD_ALPHABET "abcdefghijklmnopqrstuvwxyz"

typedef struct
{
    void (*fn_ptr)(State *, int, char **);
    char *name;
} Command;

char *cmd_read_command_line(int input_fd, char *prompt);
int cmd_parse_and_execute_command_line(State *state, char *cmd_line);

int cmd_init_command_map(void);
void cmd_deinit_command_map(void);

void cmd_quit(State *state, int argc, char **argv);
void cmd_quitall(State *state, int argc, char **argv);
void cmd_next_file(State *state, int argc, char **argv);
void cmd_previous_file(State *state, int argc, char **argv);
void cmd_set(State *state, int argc, char **argv);
void cmd_type(State *state, int argc, char **argv);
void cmd_scheme(State *state, int argc, char **argv);
void cmd_config(State *state, int argc, char **argv);
void cmd_edit(State *state, int argc, char **argv);

#endif // CMD_H
