#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "array.h"
#include "input.h"
#include "terminal.h"

int input_get_action(void)
{
    char c;
    read(STDIN_FILENO, &c, 1);
    return (int)c;
}

int input_process_action(int action, Array *buffer)
{
    switch (action)
    {
    case 'j':
        terminal_cursor_down(buffer);
        break;
    case 'k':
        terminal_cursor_up(buffer);
        break;
    case 'h':
        terminal_cursor_left(buffer);
        break;
    case 'l':
        terminal_cursor_right(buffer);
        break;
    case 'q':
        exit(0);
        break;
    default:
        if (iscntrl(action))
            printf("%d\r\n", action);
        else
            printf("%d ('%c')\r\n", action, action);
    }
    return 0;
}

void input_buffer_flush(Array *buffer)
{
    write(STDOUT_FILENO, buffer->data, buffer->len);
    buffer->len = 0;
}
