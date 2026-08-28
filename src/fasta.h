#ifndef FASTA_H
#define FASTA_H

/*
 * FASTA format IO
 */

#include <stdio.h>

#include "sequences.h"

int fasta_fread(FILE *fp, SeqRecordArray *record_array);
int fasta_read(const char *path, SeqRecordArray *record_array);
int fasta_fwrite(FILE *fp, SeqRecordArray *record_array, const int max_len);
int fasta_write(const char *path, SeqRecordArray *record_array, const int max_len);
void fasta_wrap_string(FILE *fp, const char *s, const size_t len, const int max_len);
char *fasta_get_id(const char *header);

#endif // FASTA_H
