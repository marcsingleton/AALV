#ifndef ARGPARSE_H
#define ARGPARSE_H

/*
 * Arguments and parsing
 */

#include <stdio.h>

#include "cli.h"
#include "fasta.h"
#include "sequences.h"

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

extern Option options[];
extern unsigned int noptions;

extern FormatOption format_options[];
extern unsigned int n_format_options;

extern SeqTypeOption seq_type_options[];
extern unsigned int n_seq_type_options;

extern char *option_delim;

extern char synopsis[];
extern char positional_usage[];

// Option parsing
int argparse_options(int argc, char *argv[],
                     unsigned int noptions, Option *options,
                     unsigned int n_format_options, FormatOption *format_options,
                     unsigned int n_seq_type_options, SeqTypeOption *seq_type_options,
                     const char *short_options, const struct option *long_options,
                     unsigned int *n_format_args, char ***format_args_ptr,
                     unsigned int *n_type_args, char ***type_args_ptr,
                     char *program_name, char *positional_usage, char *synopsis);
FileReader argparse_reader(const char *format_arg,
                           unsigned int n_format_options,
                           FormatOption *format_options);
SeqType argparse_seq_type(const char *seq_type_arg,
                          unsigned int n_seq_type_options,
                          SeqTypeOption *seq_type_options);

#endif // ARGPARSE_H
