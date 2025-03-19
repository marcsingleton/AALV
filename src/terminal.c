#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "terminal.h"

void terminal_enable_raw_mode(struct termios *old_termios, struct termios *raw_termios)
{
    tcgetattr(STDIN_FILENO, old_termios);
    *raw_termios = *old_termios;
    cfmakeraw(raw_termios);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, raw_termios);

    // printf("\033[s"); // Push cursor
    printf("\033[?1049h");   // Enter alternate buffer
    printf("\033[H\033[2J"); // Move cursor to top; erase display
}

void terminal_disable_raw_mode(struct termios *old_termios)
{
    printf("\033[?1049l"); // Exit alternate buffer
    // printf("\033[u");  // Pop cursor
    tcsetattr(STDIN_FILENO, TCSAFLUSH, old_termios);
}
