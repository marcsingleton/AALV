#ifndef SEQRECORD_H
#define SEQRECORD_H

typedef struct
{
    char *seq;
    char *id;
    char *header;
    int len;
} SeqRecord;

typedef struct
{
    char *SeqRecord;
    int len;
} SeqRecordArray;

#endif // SEQRECORD_H