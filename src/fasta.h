#ifndef FASTA_H
#define FASTA_H

/*
 * FASTA format IO
 */

#include <stdio.h>

#include "sequences.h"

int fasta_fread(FILE *fp, SeqRecordArray *record_array);
int fasta_read(const char *path, SeqRecordArray *record_array);
int fasta_fwrite(FILE *fp, SeqRecordArray *record_array, const unsigned int max_len);
int fasta_write(const char *path, SeqRecordArray *record_array, const int unsigned max_len);

void fasta_write_header(FILE *fp, const char *s);
void fasta_write_sequence(FILE *fp, const char *s, const int unsigned max_len);
char *fasta_get_id(const char *header);

#endif // FASTA_H
