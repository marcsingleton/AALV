#ifndef SCHEMES_H
#define SCHEMES_H

/*
 * Color schemes
 */

#include "terminal.h"

typedef struct
{
    char sym;
    Color8Bit color;
} ForegroundColor4BitTuple;

ForegroundColor4BitTuple default_nucleic_4_bit[] = {
    {'A', FG_GREEN},
    {'C', FG_BLUE},
    {'G', FG_YELLOW},
    {'T', FG_RED},
    {'N', FG_WHITE},
    {0, 0},
};

ForegroundColor4BitTuple default_protein_4_bit[] = {
    {'A', FG_BRIGHT_GREEN},
    {'C', FG_YELLOW},
    {'D', FG_RED},
    {'E', FG_RED},
    {'F', FG_MAGENTA},
    {'G', FG_BRIGHT_BLACK},
    {'H', FG_BRIGHT_BLUE},
    {'I', FG_GREEN},
    {'K', FG_BLUE},
    {'L', FG_GREEN},
    {'M', FG_GREEN},
    {'N', FG_BRIGHT_CYAN},
    {'P', FG_BRIGHT_MAGENTA},
    {'Q', FG_BRIGHT_CYAN},
    {'R', FG_BLUE},
    {'S', FG_BRIGHT_RED},
    {'T', FG_BRIGHT_RED},
    {'V', FG_GREEN},
    {'W', FG_MAGENTA},
    {'Y', FG_MAGENTA},
    {'X', FG_WHITE},
    {0, 0},
};

#endif // SCHEMES_H
