#ifndef IO_H
#define IO_H

/*
 * IO
 */

#include <stdio.h>

#include "sequences.h"
#include "state.h"

typedef int (*FileReader)(FILE *, SeqRecordArray *);

typedef struct
{
    const char *name;
    const char *exts;
    FileReader reader;
} FormatOption;

typedef struct
{
    const char *name;
    const char *identifiers;
    const SeqType type;
    const Alphabet *alphabet;
} SeqTypeOption;

extern FormatOption format_options[];
extern unsigned int n_format_options;

extern SeqTypeOption seq_type_options[];
extern unsigned int n_seq_type_options;

FileReader io_get_reader(const char *format_arg,
                         unsigned int n_format_options,
                         FormatOption *format_options);
SeqType io_get_seq_type(const char *seq_type_arg,
                        unsigned int n_seq_type_options,
                        SeqTypeOption *seq_type_options);
int io_load_seqs(FileState *file, FILE *fp, FileReader reader);
int io_set_seq_types(FileState *file, const char *seq_type_arg);
int io_set_unaligned_indices(FileState *file);

#endif // IO_H
