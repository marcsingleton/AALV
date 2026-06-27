#ifndef CMD_H
#define CMD_H

/*
 * Command parsing
 */

#define CMD_ARG_MAX 8
#define CMD_ALPHABET "abcdefghijklmnopqrstuvwxyz"

typedef struct
{
    void (*fn_ptr)(int, char **);
    char *name;
} Command;

char *cmd_read_command_line(int input_fd, char *prompt);
int cmd_parse_and_execute_command_line(char *cmd_line);
int cmd_init_command_map(void);
void cmd_deinit_command_map(void);

void cmd_quit(int argc, char **argv);
void cmd_quitall(int argc, char **argv);
void cmd_next_file(int argc, char **argv);
void cmd_previous_file(int argc, char **argv);
void cmd_set(int argc, char **argv);
void cmd_type(int argc, char **argv);
void cmd_scheme(int argc, char **argv);
void cmd_config(int argc, char **argv);

#endif // CMD_H
