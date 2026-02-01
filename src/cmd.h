#ifndef CMD_H
#define CMD_H

/*
 * Command parsing
 */

char *cmd_read_command(char *prompt);
int cmd_parse_and_execute_command(char *command);

#endif // CMD_H
