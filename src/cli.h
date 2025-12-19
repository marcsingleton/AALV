#ifndef CLI_H
#define CLI_H

/*
 * Generic command-line interface functions
 */

#include <stdbool.h>

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

int prepare_options(unsigned int noptions, Option *options,
                    char **short_options_ptr, struct option *long_options,
                    const char *invocation_name);

// CLI output
void print_long_help(unsigned int noptions, Option *options, char *program_name, char *positional_usage, char *synopsis);
void print_short_help(unsigned int noptions, Option *options, char *program_name, char *positional_usage);
int print_option_usage(Option *argument, UsageStyle usage_style, const bool brackets, const char *style_sep);

#endif // CLI_H
