#ifndef FASTA_H
#define FASTA_H

#include "seqrecord.h"

extern const int FASTA_ERROR_INVALID_FORMAT;
extern const int FASTA_ERROR_RECORD_OVERFLOW;
extern const int FASTA_ERROR_SEQUENCE_OVERFLOW;
extern const int FASTA_ERROR_FILE_IO;
extern const int FASTA_ERROR_MEMORY_ALLOCATION;

int fasta_fread(FILE *fp, SeqRecord **records_ptr);

int fasta_read(const char *path, SeqRecord **records_ptr);

int fasta_fwrite(FILE *fp, const SeqRecord *records, const int nrecords, const int maxlen);

int fasta_write(const char *path, const SeqRecord *records, const int nrecords, const int maxlen);

#endif // FASTA_H
