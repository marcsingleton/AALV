#ifndef SEQRECORD_H
#define SEQRECORD_H

typedef struct
{
    char *header;
    char *seq;
    char *id;
    int len;
} SeqRecord;

typedef struct
{
    char *SeqRecord;
    int len;
} SeqRecordArray;

#endif // SEQRECORD_H