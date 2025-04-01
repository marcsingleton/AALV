#ifndef STATE_H
#define STATE_H

/*
 * Program state
 */

#include <termios.h>

#include "sequences.h"

typedef struct
{
    struct termios old_termios;
    struct termios raw_termios;
    SeqRecordArray record_array;
} State;

#endif // STATE_H
