#ifndef SEQUENCES_H
#define SEQUENCES_H

/*
 * Sequence data structures
 */

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