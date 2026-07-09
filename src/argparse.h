#ifndef ARGPARSE_H
#define ARGPARSE_H

/*
 * Arguments and parsing
 */

#include <stdio.h>

#include "cli.h"
#include "io.h"

extern Option options[];
extern unsigned int noptions;

extern char synopsis[];
extern char positional_usage[];

int argparse_options(int argc, char *argv[],
                     unsigned int noptions, Option *options,
                     unsigned int n_format_options, FormatOption *format_options,
                     unsigned int n_seq_type_options, SeqTypeOption *seq_type_options,
                     const char *short_options, const struct option *long_options,
                     unsigned int *n_format_args, char ***format_args_ptr,
                     unsigned int *n_type_args, char ***type_args_ptr,
                     char *program_name, char *positional_usage, char *synopsis);

#endif // ARGPARSE_H
