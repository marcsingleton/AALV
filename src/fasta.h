#ifndef FASTA_H
#define FASTA_H

#include "seqrecord.h"

int FASTA_ERROR_INVALID_FORMAT = -1;
int FASTA_ERROR_RECORD_OVERFLOW = -2;
int FASTA_ERROR_SEQUENCE_OVERFLOW = -3;
int FASTA_ERROR_FILE_IO = -4;
int FASTA_ERROR_MEMORY_ALLOCATION = -5;

int fasta_parse(FILE *fp, SeqRecord **records_ptr);

#endif // FASTA_H
