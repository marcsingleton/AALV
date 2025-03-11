#ifndef FASTA_H
#define FASTA_H

/*
 * FASTA format IO
 */

#include "sequences.h"

extern const int FASTA_ERROR_INVALID_FORMAT;
extern const int FASTA_ERROR_RECORD_OVERFLOW;
extern const int FASTA_ERROR_SEQUENCE_OVERFLOW;
extern const int FASTA_ERROR_FILE_IO;
extern const int FASTA_ERROR_MEMORY_ALLOCATION;

int fasta_fread(FILE *fp, SeqRecord **records_ptr);

int fasta_read(const char *path, SeqRecord **records_ptr);

int fasta_fwrite(FILE *fp, SeqRecord *records, const int nrecords, const int maxlen);

int fasta_write(const char *path, SeqRecord *records, const int nrecords, const int maxlen);

void fasta_wrap_string(FILE *fp, const char *s, const int len, const int maxlen);

#endif // FASTA_H
