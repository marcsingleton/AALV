#ifndef ARGPARSE_H
#define ARGPARSE_H

/*
 * Arguments and parsing
 */

#include <getopt.h>
#include <stdio.h>

#include "sequences.h"

typedef enum
{
    OMIT,
    SHORT_NAME,
    LONG_NAME,
    ALL_NAMES,
} UsageStyle;

typedef struct
{
    const char *long_name;
    const char short_name;
    const char *description;
    const char *usage_arg;
    const UsageStyle usage_style;
    const int has_arg; // getopt_long field
} Option;

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
} SeqTypeOption;

// Option parsing
int parse_options(int argc, char *argv[],
                  unsigned int noptions, Option *options,
                  unsigned int n_format_options, FormatOption *format_options,
                  unsigned int n_type_options, SeqTypeOption *type_options,
                  const char *short_options, const struct option *long_options,
                  unsigned int *n_format_args, char ***format_args_ptr,
                  unsigned int *n_type_args, char ***type_args_ptr);
int prepare_options(unsigned int noptions, Option *options,
                    char **short_options_ptr, struct option *long_options);

// CLI output
void print_long_help(unsigned int noptions, Option *options);
void print_short_help(unsigned int noptions, Option *options);
int print_option_usage(Option *option, UsageStyle usage_style, const bool brackets, const char *style_sep);

#endif // ARGPARSE_H
