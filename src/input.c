#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#include "array.h"
#include "input.h"
#include "terminal.h"

#include "action.h"

int input_read_key(Array *buffer, int fd)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 2500;

    char c;
    int n = 0;
    while (select(fd + 1, &readfds, NULL, NULL, &tv) > 0 && FD_ISSET(fd, &readfds))
    {
        read(fd, &c, 1);
        array_append(buffer, &c);
        n++;
    }

    return n;
}

int input_parse_keys(Array *buffer, int *count, Command *cmd)
{
    /* Return codes
        0: success
        1: incomplete
        2: failure
    */

    size_t index = 0;
    char *ptr, c;

    // Parse digits
    size_t a = 0;
    int default_count = 0;
    while (index < buffer->len)
    {
        ptr = array_get(buffer, index);
        c = *ptr;
        if (!isdigit(c))
            break;
        if (a == 0 && c == '0') // 0 is line start, so ignore at start of digits
            break;
        if (a > (SIZE_MAX - (c - '0')) / 10)
            return 2;
        a = 10 * a + (c - '0');
        index++;
    }
    if (index >= buffer->len)
        return 1;
    if (a == 0)
    {
        a = 1;
        default_count = 1;
    }

    // Parse command
    switch (c)
    {
    case 27: // ESC
        if (index + 2 >= buffer->len)
            return 2;
        ptr = array_get(buffer, index + 1);
        c = *ptr;
        if (c != '[')
            return 2;
        ptr = array_get(buffer, index + 2);
        c = *ptr;
        switch (c)
        {
        case 'A':
            *cmd = CMD_MOVE_UP;
            break;
        case 'B':
            *cmd = CMD_MOVE_DOWN;
            break;
        case 'C':
            *cmd = CMD_MOVE_RIGHT;
            break;
        case 'D':
            *cmd = CMD_MOVE_LEFT;
            break;
        default:
            return 2;
        }
        break;
    case 'q':
        *cmd = CMD_QUIT;
        break;
    case '>':
        *cmd = CMD_NEXT_FILE;
        break;
    case '<':
        *cmd = CMD_PREVIOUS_FILE;
        break;
    case 'k':
        *cmd = CMD_MOVE_UP;
        break;
    case 'j':
        *cmd = CMD_MOVE_DOWN;
        break;
    case 'l':
        *cmd = CMD_MOVE_RIGHT;
        break;
    case 'h':
        *cmd = CMD_MOVE_LEFT;
        break;
    case CTRL('b'):
        *cmd = CMD_MOVE_FULL_PAGE_UP;
        break;
    case CTRL('f'):
        *cmd = CMD_MOVE_FULL_PAGE_DOWN;
        break;
    case CTRL('n'):
        *cmd = CMD_MOVE_FULL_PAGE_RIGHT;
        break;
    case CTRL('p'):
        *cmd = CMD_MOVE_FULL_PAGE_LEFT;
        break;
    case CTRL('u'):
        *cmd = CMD_MOVE_HALF_PAGE_UP;
        break;
    case CTRL('d'):
        *cmd = CMD_MOVE_HALF_PAGE_DOWN;
        break;
    case CTRL('r'):
        *cmd = CMD_MOVE_HALF_PAGE_RIGHT;
        break;
    case CTRL('l'):
        *cmd = CMD_MOVE_HALF_PAGE_LEFT;
        break;
    case '$':
        *cmd = CMD_MOVE_LINE_END;
        break;
    case '0':
    case '^':
        *cmd = CMD_MOVE_LINE_START;
        break;
    case 'g':
        if (index + 1 >= buffer->len)
            return 1;
        ptr = array_get(buffer, index + 1);
        c = *ptr;
        if (c != 'g')
            return 2;
        *cmd = CMD_MOVE_FIRST_RECORD;
        break;
    case 'G':
        if (default_count == 1)
            *cmd = CMD_MOVE_LAST_RECORD;
        else
        {
            *cmd = CMD_MOVE_TO_RECORD;
            a--;
        }
        break;
    case 'H':
        *cmd = CMD_MOVE_TOP_EDGE;
        break;
    case 'M':
        *cmd = CMD_MOVE_VERTICAL_MIDDLE;
        break;
    case 'L':
        *cmd = CMD_MOVE_BOTTOM_EDGE;
        break;
    case 'S':
        *cmd = CMD_MOVE_LEFT_EDGE;
        break;
    case 'C':
        *cmd = CMD_MOVE_HORIZONTAL_MIDDLE;
        break;
    case 'E':
        *cmd = CMD_MOVE_RIGHT_EDGE;
        break;
    case ']':
        *cmd = CMD_INCREASE_HEADER_PANE_WIDTH;
        break;
    case '[':
        *cmd = CMD_DECREASE_HEADER_PANE_WIDTH;
        break;
    case '}':
        *cmd = CMD_INCREASE_RULER_PANE_HEIGHT;
        break;
    case '{':
        *cmd = CMD_DECREASE_RULER_PANE_HEIGHT;
        break;
    case '+':
        *cmd = CMD_INCREASE_TICK_SPACING;
        break;
    case '-':
        *cmd = CMD_DECREASE_TICK_SPACING;
        break;
    default:
        return 2;
    }

    *count = a;

    return 0;
}

int input_execute_command(int count, Command cmd)
{
    switch (cmd)
    {
    case CMD_QUIT:
        exit(0);
        break;
    case CMD_NEXT_FILE:
        action_next_file();
        break;
    case CMD_PREVIOUS_FILE:
        action_previous_file();
        break;
    case CMD_MOVE_DOWN:
        action_move_down(count);
        break;
    case CMD_MOVE_UP:
        action_move_up(count);
        break;
    case CMD_MOVE_RIGHT:
        action_move_right(count);
        break;
    case CMD_MOVE_LEFT:
        action_move_left(count);
        break;
    case CMD_MOVE_FULL_PAGE_UP:
        action_move_page_up(PAGE_SIZE_FULL);
        break;
    case CMD_MOVE_FULL_PAGE_DOWN:
        action_move_page_down(PAGE_SIZE_FULL);
        break;
    case CMD_MOVE_FULL_PAGE_RIGHT:
        action_move_page_right(PAGE_SIZE_FULL);
        break;
    case CMD_MOVE_FULL_PAGE_LEFT:
        action_move_page_left(PAGE_SIZE_FULL);
        break;
    case CMD_MOVE_HALF_PAGE_UP:
        action_move_page_up(PAGE_SIZE_HALF);
        break;
    case CMD_MOVE_HALF_PAGE_DOWN:
        action_move_page_down(PAGE_SIZE_HALF);
        break;
    case CMD_MOVE_HALF_PAGE_RIGHT:
        action_move_page_right(PAGE_SIZE_HALF);
        break;
    case CMD_MOVE_HALF_PAGE_LEFT:
        action_move_page_left(PAGE_SIZE_HALF);
        break;
    case CMD_MOVE_LINE_END:
        action_move_line_end();
        break;
    case CMD_MOVE_LINE_START:
        action_move_line_start();
        break;
    case CMD_MOVE_FIRST_RECORD:
        action_move_first_record();
        break;
    case CMD_MOVE_LAST_RECORD:
        action_move_last_record();
        break;
    case CMD_MOVE_TO_RECORD:
        action_move_to_record(count);
        break;
    case CMD_MOVE_TOP_EDGE:
        action_move_top_edge();
        break;
    case CMD_MOVE_VERTICAL_MIDDLE:
        action_move_vertical_middle();
        break;
    case CMD_MOVE_BOTTOM_EDGE:
        action_move_bottom_edge();
        break;
    case CMD_MOVE_LEFT_EDGE:
        action_move_left_edge();
        break;
    case CMD_MOVE_HORIZONTAL_MIDDLE:
        action_move_horizontal_middle();
        break;
    case CMD_MOVE_RIGHT_EDGE:
        action_move_right_edge();
        break;
    case CMD_INCREASE_HEADER_PANE_WIDTH:
        action_increase_header_pane_width();
        break;
    case CMD_DECREASE_HEADER_PANE_WIDTH:
        action_decrease_header_pane_width();
        break;
    case CMD_INCREASE_RULER_PANE_HEIGHT:
        action_increase_ruler_pane_height();
        break;
    case CMD_DECREASE_RULER_PANE_HEIGHT:
        action_decrease_ruler_pane_height();
        break;
    case CMD_INCREASE_TICK_SPACING:
        action_increase_tick_spacing();
        break;
    case CMD_DECREASE_TICK_SPACING:
        action_decrease_tick_spacing();
        break;
    }

    return 0;
}

void input_buffer_flush(Array *buffer)
{
    write(STDOUT_FILENO, buffer->data, buffer->len);
    buffer->len = 0;
}
