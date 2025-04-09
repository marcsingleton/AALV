#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "array.h"
#include "terminal.h"

int terminal_get_termios(struct termios *termios_p)
{
    return tcgetattr(STDIN_FILENO, termios_p);
}

int terminal_enable_raw_mode(struct termios *old_termios, struct termios *raw_termios)
{
    *raw_termios = *old_termios;
    cfmakeraw(raw_termios);
    return tcsetattr(STDIN_FILENO, TCSAFLUSH, raw_termios);
}

void terminal_disable_raw_mode(struct termios *old_termios)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, old_termios);
}

void terminal_use_alternate_buffer(void)
{
    char s[] = "\x1b[?1049h\x1b[H"; // Use alternate buffer and move cursor to top left
    write(STDOUT_FILENO, s, sizeof(s) - 1);
}

void terminal_use_normal_buffer(void)
{
    char s[] = "\x1b[?1049l";
    write(STDOUT_FILENO, s, sizeof(s) - 1);
}

void terminal_cursor_up(Array *buffer)
{
    char s[] = "\x1b[A";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_cursor_down(Array *buffer)
{
    char s[] = "\x1b[B";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_cursor_right(Array *buffer)
{
    char s[] = "\x1b[C";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_cursor_left(Array *buffer)
{
    char s[] = "\x1b[D";
    array_extend(buffer, s, sizeof(s) - 1);
}

int terminal_get_window_size(int *rows, int *cols)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
        return -1;
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
