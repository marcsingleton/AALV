#ifndef LINEFMT_H
#define LINEFMT_H

/*
 * Line-based sequence IO
 * Each line contains a complete sequence, no headers
 */

#include <stdio.h>

#include "sequences.h"

int linefmt_fread(FILE *fp, SeqRecordArray *record_array);
int linefmt_read(const char *path, SeqRecordArray *record_array);
int linefmt_fwrite(FILE *fp, SeqRecordArray *record_array);
int linefmt_write(const char *path, SeqRecordArray *record_array);

#endif // LINEFMT_H
