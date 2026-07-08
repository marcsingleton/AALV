#ifndef SCHEMES_H
#define SCHEMES_H

/*
 * Color schemes
 *
 * In case-insensitive alphabets, lowercase symbols are internally mapped to uppercase. As a result, color schemes given
 * in uppercase are case-insensitive. However, "lowercase" color schemes are not.
 *
 */

#include <stdbool.h>

#include "color.h"
#include "sequences.h"
#include "terminal.h"

typedef struct
{
    SeqType type;
    ColorScheme scheme;
} SeqColorScheme;

extern SeqColorScheme schemes_default_nucleic_4_bit;
extern SeqColorScheme schemes_default_protein_4_bit;
extern SeqColorScheme schemes_default_nucleic_8_bit;
extern SeqColorScheme schemes_default_protein_8_bit;

extern SeqColorScheme schemes_base[];
extern unsigned int n_base_schemes;

int schemes_init_base(void);
void schemes_deinit_base(void);

#endif // SCHEMES_H
