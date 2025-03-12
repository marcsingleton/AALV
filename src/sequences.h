#ifndef SEQUENCES_H
#define SEQUENCES_H

/*
 * Sequence data structures
 */

#include <stddef.h>

typedef struct
{
    char *header;
    char *seq;
    char *id;
    size_t len;
} SeqRecord;

typedef struct
{
    char *SeqRecord;
    size_t len;
} SeqRecordArray;

#endif // SEQUENCES_H
