#ifndef SEQUENCES_H
#define SEQUENCES_H

/*
 * Sequence data structures
 */

#include <stddef.h>
#include <stdlib.h>

typedef struct
{
    char *header;
    char *seq;
    char *id;
    size_t len;
} SeqRecord;

typedef struct
{
    SeqRecord *records;
    unsigned int len;
    unsigned int offset;
} SeqRecordArray;

void sequences_free_seq_records(SeqRecord *records, int nrecords);
void sequences_free_seq_record_array(SeqRecordArray *record_array);

#endif // SEQUENCES_H
