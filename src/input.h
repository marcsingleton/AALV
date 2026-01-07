#ifndef INPUT_H
#define INPUT_H

#include "array.h"
#include "action.h"

enum ParseResult
{
    PARSE_SUCCESS = 0,
    PARSE_INCOMPLETE,
    PARSE_FAIL
};

int input_read_key(Array *buffer, int fd);
int input_parse_keys(Array *buffer, Action *action, size_t *count);
int input_execute_action(Action *action, size_t count);
void input_buffer_flush(Array *buffer);

#endif // INPUT_H
