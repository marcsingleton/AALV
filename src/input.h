#ifndef INPUT_H
#define INPUT_H

#include "array.h"
#include "action.h"
#include "state.h"

/*
 * Reading and parsing normal mode input
 *
 * The parser follows the following grammar:
 *   BUFFER = ACTION | COUNT_ACTION
 *   COUNT_ACTION = COUNT ACTION
 *   COUNT = SIGN DIGITS | DIGITS
 *   SIGN = +-
 *   DIGITS = 0123456789
 */

enum ParseResult
{
    PARSE_SUCCESS = 0,
    PARSE_INCOMPLETE,
    PARSE_FAIL
};

int input_read_key(Array *buffer, int fd);
int input_parse_digits(Array *buffer, size_t *index, int *count);
int input_parse_count(Array *buffer, size_t *index, int *count);
int input_parse_action(Array *buffer, size_t *index, Action *action, int default_count_branch);
int input_parse_count_action(Array *buffer, size_t *index, Action *action, int *count);
int input_parse_buffer(Array *buffer, Action *action, int *count);
int input_execute_action(State *state, Action *action, int count);
void input_write_buffer_flush(Array *write_buffer);
void input_read_buffer_flush(Array *read_buffer);

#endif // INPUT_H
