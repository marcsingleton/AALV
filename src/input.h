#ifndef INPUT_H
#define INPUT_H

#include "array.h"
#include "commands.h"

int input_read_key(Array *buffer, int fd);
int input_parse_keys(Array *buffer, int *count, Command *cmd);
int input_execute_command(int count, Command cmd);
void input_buffer_flush(Array *buffer);

#endif // INPUT_H
