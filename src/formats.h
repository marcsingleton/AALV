#ifndef FORMATS_H
#define FORMATS_H

/*
 * Functions for reading sequence formats and structures for holding them
 */

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

int parse_fasta(FILE *fp, SeqRecord **records_ptr);

#endif // FORMATS_H
