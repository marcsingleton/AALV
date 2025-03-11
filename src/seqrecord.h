#ifndef SEQRECORD_H
#define SEQRECORD_H

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

#endif // SEQRECORD_H