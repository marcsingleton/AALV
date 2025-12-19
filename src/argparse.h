#ifndef ARGPARSE_H
#define ARGPARSE_H

/*
 * Arguments and parsing
 */

#include <stdio.h>

#include "cli.h"
#include "sequences.h"

typedef struct
{
    const char *name;
    const char *exts;
    int (*reader)(FILE *, SeqRecord **);
} FormatOption;

typedef struct
{
    const char *name;
    const char *identifiers;
    const SeqType type;
    const Alphabet *alphabet;
} SeqTypeOption;

// Option parsing
int parse_options(int argc, char *argv[],
                  unsigned int noptions, Option *options,
                  unsigned int n_format_options, FormatOption *format_options,
                  unsigned int n_type_options, SeqTypeOption *type_options,
                  const char *short_options, const struct option *long_options,
                  unsigned int *n_format_args, char ***format_args_ptr,
                  unsigned int *n_type_args, char ***type_args_ptr,
                  char *program_name, char *positional_usage, char *synopsis);

#endif // ARGPARSE_H
