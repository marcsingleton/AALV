#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "input.h"
#include "terminal.h"

#define ESC 27

int input_read_key(Array *buffer, int fd)
{
    char c;
    int n = 0;
    if (read(fd, &c, 1) > 0)
    {
        array_append(buffer, &c);
        n++;
    }

    return n;
}

int input_parse_digits(Array *buffer, size_t *index, int *count)
{
    size_t new_index = *index;
    if (new_index >= buffer->len)
        return PARSE_INCOMPLETE;

    int new_count = 0;
    char c = *(char *)array_get(buffer, new_index);
    if (!isdigit(c))
        return PARSE_FAIL;
    else
    {
        for (; new_index < buffer->len; new_index++)
        {
            c = *(char *)array_get(buffer, new_index);
            if (!isdigit(c))
                break;
            if (new_count > (INT_MAX - (c - '0')) / 10)
                return PARSE_FAIL;
            new_count = 10 * new_count + (c - '0');
        }
    }

    *index = new_index;
    *count = new_count;

    return PARSE_SUCCESS;
}

int input_parse_count(Array *buffer, size_t *index, int *count)
{
    size_t new_index = *index;
    if (new_index >= buffer->len)
        return PARSE_INCOMPLETE;

    int sign;
    char c = *(char *)array_get(buffer, new_index);
    if (c == '+' || c == '-')
    {
        sign = (c == '+') ? 1 : -1;
        new_index++;
    }
    else
        sign = 1;

    int digits;
    int retcode = input_parse_digits(buffer, &new_index, &digits);
    switch (retcode)
    {
    case PARSE_SUCCESS:
        break;
    case PARSE_INCOMPLETE:
    case PARSE_FAIL:
        return retcode;
    }

    *index = new_index;
    *count = sign * digits;

    return PARSE_SUCCESS;
}

int input_parse_action(Array *buffer, size_t *index, Action *action, int default_count_branch)
{
    size_t new_index = *index;
    if (new_index >= buffer->len)
        return PARSE_INCOMPLETE;

    Action new_action;
    char c = *(char *)array_get(buffer, new_index);
    switch (c)
    {
    case ESC:
        if (new_index + 1 < buffer->len && *(char *)array_get(buffer, new_index + 1) == ESC) // Special case: Error on two escapes
            return PARSE_FAIL;
        if (new_index + 2 >= buffer->len) // Otherwise wait for complete escape sequence
            return PARSE_INCOMPLETE;

        c = *(char *)array_get(buffer, new_index + 1);
        if (c != '[')
            return PARSE_FAIL;

        c = *(char *)array_get(buffer, new_index + 2);
        switch (c)
        {
        case 'A':
            new_action.fn.int_arg = action_move_up;
            new_action.args = INT_ARG;
            break;
        case 'B':
            new_action.fn.int_arg = action_move_down;
            new_action.args = INT_ARG;
            break;
        case 'C':
            new_action.fn.int_arg = action_move_right;
            new_action.args = INT_ARG;
            break;
        case 'D':
            new_action.fn.int_arg = action_move_left;
            new_action.args = INT_ARG;
            break;
        default:
            return PARSE_FAIL;
        }
        break;
    case 'k':
        new_action.fn.int_arg = action_move_up;
        new_action.args = INT_ARG;
        break;
    case 'j':
        new_action.fn.int_arg = action_move_down;
        new_action.args = INT_ARG;
        break;
    case 'l':
        new_action.fn.int_arg = action_move_right;
        new_action.args = INT_ARG;
        break;
    case 'h':
        new_action.fn.int_arg = action_move_left;
        new_action.args = INT_ARG;
        break;
    case CTRL('b'):
        new_action.fn.void_arg = action_move_page_up;
        new_action.args = VOID_ARG;
        break;
    case CTRL('f'):
        new_action.fn.void_arg = action_move_page_down;
        new_action.args = VOID_ARG;
        break;
    case CTRL('n'):
        new_action.fn.void_arg = action_move_page_right;
        new_action.args = VOID_ARG;
        break;
    case CTRL('p'):
        new_action.fn.void_arg = action_move_page_left;
        new_action.args = VOID_ARG;
        break;
    case CTRL('u'):
        new_action.fn.void_arg = action_move_half_page_up;
        new_action.args = VOID_ARG;
        break;
    case CTRL('d'):
        new_action.fn.void_arg = action_move_half_page_down;
        new_action.args = VOID_ARG;
        break;
    case CTRL('r'):
        new_action.fn.void_arg = action_move_half_page_right;
        new_action.args = VOID_ARG;
        break;
    case CTRL('l'):
        new_action.fn.void_arg = action_move_half_page_left;
        new_action.args = VOID_ARG;
        break;
    case '$':
        new_action.fn.void_arg = action_move_last_column;
        new_action.args = VOID_ARG;
        break;
    case '0':
        new_action.fn.void_arg = action_move_first_column;
        new_action.args = VOID_ARG;
        break;
    case '|':
        new_action.fn.int_arg = action_move_to_column;
        new_action.args = INT_ARG;
        break;
    case '^':
        new_action.fn.void_arg = action_move_first_non_gap_or_non_whitespace;
        new_action.args = VOID_ARG;
        break;
    case 'g':
        if (new_index + 1 >= buffer->len)
            return PARSE_INCOMPLETE;
        c = *(char *)array_get(buffer, new_index);
        switch (c)
        {
        case 'g':
            new_action.fn.void_arg = action_move_first_row;
            new_action.args = VOID_ARG;
            break;
        case '_':
            new_action.fn.void_arg = action_move_last_non_gap_or_non_whitespace;
            new_action.args = VOID_ARG;
            break;
        case '0':
            new_action.fn.void_arg = action_move_left_edge;
            new_action.args = VOID_ARG;
            break;
        case 'm':
            new_action.fn.void_arg = action_move_horizontal_middle;
            new_action.args = VOID_ARG;
            break;
        case 'M':
            new_action.fn.void_arg = action_move_middle_column;
            new_action.args = VOID_ARG;
            break;
        case '$':
            new_action.fn.void_arg = action_move_right_edge;
            new_action.args = VOID_ARG;
            break;
        default:
            return PARSE_FAIL;
        }
        break;
    case 'G':
        if (default_count_branch == 1)
        {
            new_action.fn.void_arg = action_move_last_row;
            new_action.args = VOID_ARG;
        }
        else
        {
            new_action.fn.int_arg = action_move_to_row;
            new_action.args = INT_ARG;
        }
        break;
    case 'H':
        new_action.fn.void_arg = action_move_top_edge;
        new_action.args = VOID_ARG;
        break;
    case 'M':
        new_action.fn.void_arg = action_move_vertical_middle;
        new_action.args = VOID_ARG;
        break;
    case 'L':
        new_action.fn.void_arg = action_move_bottom_edge;
        new_action.args = VOID_ARG;
        break;
    case 'z':
        if (new_index + 1 >= buffer->len)
            return PARSE_INCOMPLETE;
        c = *(char *)array_get(buffer, new_index);
        switch (c)
        {
        case 'z':
            new_action.fn.void_arg = action_move_row_to_center;
            new_action.args = VOID_ARG;
            break;
        default:
            return PARSE_FAIL;
        }
        break;
    case ']':
        new_action.fn.void_arg = action_increase_header_sequence_divider;
        new_action.args = VOID_ARG;
        break;
    case '[':
        new_action.fn.void_arg = action_decrease_header_sequence_divider;
        new_action.args = VOID_ARG;
        break;
    case '}':
        new_action.fn.void_arg = action_increase_ruler_records_divider;
        new_action.args = VOID_ARG;
        break;
    case '{':
        new_action.fn.void_arg = action_decrease_ruler_records_divider;
        new_action.args = VOID_ARG;
        break;
    case ')':
        new_action.fn.void_arg = action_increase_tick_spacing;
        new_action.args = VOID_ARG;
        break;
    case '(':
        new_action.fn.void_arg = action_decrease_tick_spacing;
        new_action.args = VOID_ARG;
        break;
    case ':':
        new_action.fn.void_arg = action_enter_command_mode;
        new_action.args = VOID_ARG;
        break;
    case CTRL('w'):
        // A little inelegant, but it's only twice
        if (new_index + 1 >= buffer->len)
            return PARSE_INCOMPLETE;
        c = *(char *)array_get(buffer, new_index + 1);
        if (c != ESC)
            return PARSE_FAIL;

        if (new_index + 2 >= buffer->len)
            return PARSE_INCOMPLETE;
        c = *(char *)array_get(buffer, new_index + 2);
        if (c != '[')
            return PARSE_FAIL;

        if (new_index + 3 >= buffer->len)
            return PARSE_INCOMPLETE;
        c = *(char *)array_get(buffer, new_index + 3);
        switch (c)
        {
        case 'C':
            new_action.fn.void_arg = action_set_sequence_pane_active;
            new_action.args = VOID_ARG;
            break;
        case 'D':
            new_action.fn.void_arg = action_set_header_pane_active;
            new_action.args = VOID_ARG;
            break;
        default:
            return PARSE_FAIL;
        }
        break;
    case CTRL('Z'):
        new_action.fn.void_arg = action_raise_sigtstp;
        new_action.args = VOID_ARG;
        break;
    default:
        return PARSE_FAIL;
    }

    *index = new_index;
    *action = new_action;

    return PARSE_SUCCESS;
}

int input_parse_count_action(Array *buffer, size_t *index, Action *action, int *count)
{
    size_t new_index = *index;
    if (new_index >= buffer->len)
        return PARSE_INCOMPLETE;

    int retcode;

    retcode = input_parse_count(buffer, index, count);
    switch (retcode)
    {
    case PARSE_SUCCESS:
        break;
    case PARSE_INCOMPLETE:
    case PARSE_FAIL:
        return retcode;
    }

    retcode = input_parse_action(buffer, index, action, 0);

    return retcode;
}

int input_parse_buffer(Array *buffer, Action *action, int *count)
{
    size_t index = 0;

    int retcode1 = input_parse_action(buffer, &index, action, 1);
    switch (retcode1)
    {
    case PARSE_SUCCESS:
        *count = 1;
        return PARSE_SUCCESS;
    case PARSE_INCOMPLETE:
    case PARSE_FAIL:
        break;
    }

    int retcode2 = input_parse_count_action(buffer, &index, action, count);
    switch (retcode2)
    {
    case PARSE_SUCCESS:
        return PARSE_SUCCESS;
    case PARSE_INCOMPLETE:
    case PARSE_FAIL:
        break;
    }

    if (retcode1 == PARSE_FAIL && retcode2 == PARSE_FAIL)
        return PARSE_FAIL;
    else
        return PARSE_INCOMPLETE;
}

int input_execute_action(State *state, Action *action, int count)
{
    switch (action->args)
    {
    case VOID_ARG:
    {
        void (*fn)(State *) = action->fn.void_arg;
        fn(state);
        break;
    }
    case INT_ARG:
    {
        void (*fn)(State *, int) = action->fn.int_arg;
        int arg = count;
        fn(state, arg);
        break;
    }
    }

    return 0;
}

void input_write_buffer_flush(Array *write_buffer)
{
    write(STDOUT_FILENO, write_buffer->data, write_buffer->len);
    write_buffer->len = 0;
}

void input_read_buffer_flush(Array *read_buffer)
{
    char c;
    terminal_set_nonblocking_read();
    while (read(TERMINAL_FILENO, &c, 1) > 0)
        ;
    terminal_set_blocking_read();
    read_buffer->len = 0;
}
