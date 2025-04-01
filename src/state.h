#ifndef STATE_H
#define STATE_H

/*
 * Program state
 */

#include <termios.h>

typedef struct
{
    struct termios old_termios;
    struct termios raw_termios;
} State;

#endif // STATE_H
