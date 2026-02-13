#ifndef FASTA_H
#define FASTA_H

/*
 * FASTA format IO
 */

#include <stdio.h>

#include "sequences.h"

typedef enum
{
    FASTA_ERROR_SUCCESS = 0,
    FASTA_ERROR_INVALID_FORMAT = -1,
    FASTA_ERROR_RECORD_OVERFLOW = -2,
    FASTA_ERROR_SEQUENCE_OVERFLOW = -3,
    FASTA_ERROR_FILE_IO = -4,
    FASTA_ERROR_MEMORY_ALLOCATION = -5,
} FastaError;

int fasta_fread(FILE *fp, SeqRecordArray *record_array);
int fasta_read(const char *path, SeqRecordArray *record_array);
int fasta_fwrite(FILE *fp, SeqRecordArray *record_array, const int max_len);
int fasta_write(const char *path, SeqRecordArray *record_array, const int max_len);
void fasta_wrap_string(FILE *fp, const char *s, const size_t len, const int max_len);
char *fasta_get_id(const char *header);

#endif // FASTA_H
