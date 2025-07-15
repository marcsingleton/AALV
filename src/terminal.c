#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "array.h"
#include "terminal.h"

#define UINT_STR_MAX 3 * sizeof(UINT_MAX)

int TERMINAL_FILENO = STDIN_FILENO;

int terminal_get_termios(struct termios *termios_p)
{
    return tcgetattr(TERMINAL_FILENO, termios_p);
}

int terminal_get_window_size(unsigned int *rows, unsigned int *cols)
{
    struct winsize ws;
    if (ioctl(TERMINAL_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
        return -1;
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

int terminal_enable_raw_mode(struct termios *old_termios, struct termios *raw_termios)
{
    *raw_termios = *old_termios;
    cfmakeraw(raw_termios);
    return tcsetattr(TERMINAL_FILENO, TCSAFLUSH, raw_termios);
}

int terminal_disable_raw_mode(struct termios *old_termios)
{
    return tcsetattr(TERMINAL_FILENO, TCSAFLUSH, old_termios);
}

void terminal_use_alternate_buffer(void)
{
    char s[] = "\x1b[?1049h";
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

void terminal_cursor_origin(Array *buffer)
{
    char s[] = "\x1b[1;1H";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_cursor_ij(Array *buffer, const unsigned int i, const unsigned int j)
{
    char s[4 + 2 * UINT_STR_MAX + 1]; // 4 template, 2 UINTS, and 1 null
    int len = sprintf(s, "\x1b[%d;%dH", i, j);
    array_extend(buffer, s, len + 1);
}

void terminal_cursor_hide(Array *buffer)
{
    char s[] = "\x1b[?25l";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_cursor_show(Array *buffer)
{
    char s[] = "\x1b[?25h";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_clear_screen(Array *buffer)
{
    char s[] = "\x1b[2J";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_clear_line(Array *buffer)
{
    char s[] = "\x1b[2K";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_clear_line_left(Array *buffer)
{
    char s[] = "\x1b[1K";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_clear_line_right(Array *buffer)
{
    char s[] = "\x1b[0K";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_set_foreground_color_4bit(Array *buffer, ForegroundColor4Bit color)
{
    char s[7];
    int len = sprintf(s, "\x1b[%dm", color);
    array_extend(buffer, s, len);
}

void terminal_set_background_color_4bit(Array *buffer, BackgroundColor4Bit color)
{
    char s[7];
    int len = sprintf(s, "\x1b[%dm", color);
    array_extend(buffer, s, len);
}

void terminal_set_color_4bit(Array *buffer, ForegroundColor4Bit fg_color, BackgroundColor4Bit bg_color)

{
    char s[11];
    int len = sprintf(s, "\x1b[%d;%dm", fg_color, bg_color);
    array_extend(buffer, s, len);
}

void terminal_set_foreground_color_8bit(Array *buffer, uint8_t n)
{

    if (n > 255)
        return;
    char s[12];
    int len = sprintf(s, "\x1b[38;5;%dm", n);
    array_extend(buffer, s, len);
}

void terminal_set_background_color_8bit(Array *buffer, uint8_t n)
{

    if (n > 255)
        return;
    char s[12];
    int len = sprintf(s, "\x1b[48;5;%dm", n);
    array_extend(buffer, s, len);
}

void terminal_set_color_8bit(Array *buffer, uint8_t fg_n, uint8_t bg_n)
{
    char s[21];
    int len = sprintf(s, "\x1b[38;5;%d;48;5;%dm", fg_n, bg_n);
    array_extend(buffer, s, len);
}

void terminal_set_foreground_color_default(Array *buffer)
{
    char s[] = "\x1b[39m";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_set_background_color_default(Array *buffer)
{
    char s[] = "\x1b[49m";
    array_extend(buffer, s, sizeof(s) - 1);
}

void terminal_set_color_default(Array *buffer)
{
    char s[] = "\x1b[39;49m";
    array_extend(buffer, s, sizeof(s) - 1);
}
