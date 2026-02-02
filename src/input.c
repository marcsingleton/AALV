#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

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

int input_parse_keys(Array *buffer, Action *action, size_t *count)
{
    size_t index = 0;
    char *ptr, c;

    // Parse digits
    size_t accum = 0;
    int accum_default_set = 0;
    while (index < buffer->len)
    {
        ptr = array_get(buffer, index);
        c = *ptr;
        if (!isdigit(c))
            break;
        if (accum == 0 && c == '0') // 0 is line start, so ignore at start of digits
            break;
        if (accum > (SIZE_MAX - (c - '0')) / 10)
            return PARSE_FAIL;
        accum = 10 * accum + (c - '0');
        index++;
    }
    if (index >= buffer->len)
        return PARSE_INCOMPLETE;
    if (accum == 0)
    {
        accum = 1;
        accum_default_set = 1;
    }

    // Parse command
    switch (c)
    {
    case ESC:
        if (index + 1 < buffer->len && *(char *)array_get(buffer, index + 1) == ESC) // Special case: Error on two escapes
            return PARSE_FAIL;
        if (index + 2 >= buffer->len) // Otherwise wait for complete escape sequence
            return PARSE_INCOMPLETE;

        c = *(char *)array_get(buffer, index + 1);
        if (c != '[')
            return PARSE_FAIL;

        c = *(char *)array_get(buffer, index + 2);
        switch (c)
        {
        case 'A':
            action->fn.size_t_arg = action_move_up;
            action->args = SIZE_T_ARG;
            break;
        case 'B':
            action->fn.size_t_arg = action_move_down;
            action->args = SIZE_T_ARG;
            break;
        case 'C':
            action->fn.size_t_arg = action_move_right;
            action->args = SIZE_T_ARG;
            break;
        case 'D':
            action->fn.size_t_arg = action_move_left;
            action->args = SIZE_T_ARG;
            break;
        default:
            return PARSE_FAIL;
        }
        break;
    case 'k':
        action->fn.size_t_arg = action_move_up;
        action->args = SIZE_T_ARG;
        break;
    case 'j':
        action->fn.size_t_arg = action_move_down;
        action->args = SIZE_T_ARG;
        break;
    case 'l':
        action->fn.size_t_arg = action_move_right;
        action->args = SIZE_T_ARG;
        break;
    case 'h':
        action->fn.size_t_arg = action_move_left;
        action->args = SIZE_T_ARG;
        break;
    case CTRL('b'):
        action->fn.page_size_arg = action_move_page_up;
        action->args = PAGE_SIZE_ARG;
        accum = PAGE_SIZE_FULL;
        break;
    case CTRL('f'):
        action->fn.page_size_arg = action_move_page_down;
        action->args = PAGE_SIZE_ARG;
        accum = PAGE_SIZE_FULL;
        break;
    case CTRL('n'):
        action->fn.page_size_arg = action_move_page_right;
        action->args = PAGE_SIZE_ARG;
        accum = PAGE_SIZE_FULL;
        break;
    case CTRL('p'):
        action->fn.page_size_arg = action_move_page_left;
        action->args = PAGE_SIZE_ARG;
        accum = PAGE_SIZE_FULL;
        break;
    case CTRL('u'):
        action->fn.page_size_arg = action_move_page_up;
        action->args = PAGE_SIZE_ARG;
        accum = PAGE_SIZE_HALF;
        break;
    case CTRL('d'):
        action->fn.page_size_arg = action_move_page_down;
        action->args = PAGE_SIZE_ARG;
        accum = PAGE_SIZE_HALF;
        break;
    case CTRL('r'):
        action->fn.page_size_arg = action_move_page_right;
        action->args = PAGE_SIZE_ARG;
        accum = PAGE_SIZE_HALF;
        break;
    case CTRL('l'):
        action->fn.page_size_arg = action_move_page_left;
        action->args = PAGE_SIZE_ARG;
        accum = PAGE_SIZE_HALF;
        break;
    case '$':
        action->fn.void_arg = action_move_line_end;
        action->args = VOID_ARG;
        break;
    case '0':
        action->fn.void_arg = action_move_line_start;
        action->args = VOID_ARG;
        break;
    case '^':
        action->fn.void_arg = action_move_first_non_gap_or_non_whitespace;
        action->args = VOID_ARG;
        break;
    case 'g':
        if (index + 1 >= buffer->len)
            return PARSE_INCOMPLETE;
        ptr = array_get(buffer, index + 1);
        c = *ptr;
        switch (c)
        {
        case 'g':
            action->fn.void_arg = action_move_first_record;
            action->args = VOID_ARG;
            break;
        case '_':
            action->fn.void_arg = action_move_last_non_gap_or_non_whitespace;
            action->args = VOID_ARG;
            break;
        default:
            return PARSE_FAIL;
        }
        break;
    case 'G':
        if (accum_default_set == 1)
        {
            action->fn.void_arg = action_move_last_record;
            action->args = VOID_ARG;
        }
        else
        {
            action->fn.size_t_arg = action_move_to_record;
            action->args = SIZE_T_ARG;
            accum--;
        }
        break;
    case 'H':
        action->fn.void_arg = action_move_top_edge;
        action->args = VOID_ARG;
        break;
    case 'M':
        action->fn.void_arg = action_move_vertical_middle;
        action->args = VOID_ARG;
        break;
    case 'L':
        action->fn.void_arg = action_move_bottom_edge;
        action->args = VOID_ARG;
        break;
    case 'S':
        action->fn.void_arg = action_move_left_edge;
        action->args = VOID_ARG;
        break;
    case 'C':
        action->fn.void_arg = action_move_horizontal_middle;
        action->args = VOID_ARG;
        break;
    case 'E':
        action->fn.void_arg = action_move_right_edge;
        action->args = VOID_ARG;
        break;
    case ']':
        action->fn.void_arg = action_increase_header_sequence_divider;
        action->args = VOID_ARG;
        break;
    case '[':
        action->fn.void_arg = action_decrease_header_sequence_divider;
        action->args = VOID_ARG;
        break;
    case '}':
        action->fn.void_arg = action_increase_ruler_records_divider;
        action->args = VOID_ARG;
        break;
    case '{':
        action->fn.void_arg = action_decrease_ruler_records_divider;
        action->args = VOID_ARG;
        break;
    case '+':
        action->fn.void_arg = action_increase_tick_spacing;
        action->args = VOID_ARG;
        break;
    case '-':
        action->fn.void_arg = action_decrease_tick_spacing;
        action->args = VOID_ARG;
        break;
    case ':':
        action->fn.void_arg = action_enter_command_mode;
        action->args = VOID_ARG;
        break;
    case CTRL('w'):
        // A little inelegant, but it's only twice
        if (index + 1 >= buffer->len)
            return PARSE_INCOMPLETE;
        c = *(char *)array_get(buffer, index + 1);
        if (c != ESC)
            return PARSE_FAIL;

        if (index + 2 >= buffer->len)
            return PARSE_INCOMPLETE;
        c = *(char *)array_get(buffer, index + 2);
        if (c != '[')
            return PARSE_FAIL;

        if (index + 3 >= buffer->len)
            return PARSE_INCOMPLETE;
        c = *(char *)array_get(buffer, index + 3);
        switch (c)
        {
        case 'C':
            action->fn.void_arg = action_set_sequence_pane_active;
            action->args = VOID_ARG;
            break;
        case 'D':
            action->fn.void_arg = action_set_header_pane_active;
            action->args = VOID_ARG;
            break;
        default:
            return PARSE_FAIL;
        }
        break;
    default:
        return PARSE_FAIL;
    }

    *count = accum;

    return PARSE_SUCCESS;
}

int input_execute_action(Action *action, size_t count)
{
    switch (action->args)
    {
    case VOID_ARG:
    {
        void (*fn)(void) = action->fn.void_arg;
        fn();
        break;
    }
    case SIZE_T_ARG:
    {
        void (*fn)(size_t) = action->fn.size_t_arg;
        size_t arg = count;
        fn(arg);
        break;
    }
    case PAGE_SIZE_ARG:
    {
        void (*fn)(PageSize) = action->fn.page_size_arg;
        PageSize arg = count;
        fn(arg);
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
