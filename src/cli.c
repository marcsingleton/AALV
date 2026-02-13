#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "cli.h"
#include "error.h"

int cli_prepare_options(unsigned int noptions, Option *options,
                        char **short_options_ptr, struct option *long_options,
                        const char *invocation_name)
{
    int retcode = 0;
    Array short_options_array = {.data = NULL};
    if (array_init(&short_options_array, sizeof(char)) != 0)
    {
        error_printf("%s: Failed to allocate memory to create options array\n", invocation_name);
        goto cleanup;
    }
    for (unsigned int i = 0; i < noptions; i++)
    {
        Option *argument = options + i;

        // Fill struct for long options
        struct option *long_option = long_options + i;
        long_option->name = argument->long_name;
        long_option->has_arg = argument->has_arg;
        long_option->flag = 0;
        long_option->val = 0;

        // Fill array for short options syntax
        if (!argument->short_name)
            continue;
        char s = argument->short_name;
        array_append(&short_options_array, &s);
        switch (argument->has_arg)
        {
        case required_argument:
        {
            char s[] = ":";
            array_extend(&short_options_array, &s, sizeof(s) - 1);
            break;
        }
        case optional_argument:
        {
            char s[] = "::";
            array_extend(&short_options_array, &s, sizeof(s) - 1);
            break;
        }
        }
    }
    char s[] = "\0"; // Null terminate
    array_extend(&short_options_array, &s, sizeof(s) - 1);

    // Convert array of short options to string
    char *short_options = malloc(short_options_array.len * short_options_array.size);
    if (!short_options)
    {
        retcode = -1;
        error_printf("%s: Failed to allocate memory to create options string\n", invocation_name);
        goto cleanup;
    }
    memcpy(short_options, short_options_array.data, short_options_array.len * short_options_array.size);
    *short_options_ptr = short_options;
cleanup:
    array_deinit(&short_options_array);
    return retcode;
}

void cli_print_long_help(unsigned int noptions, Option *options,
                         char *program_name, char *positional_usage, char *synopsis)
{
    unsigned int nmax;
    unsigned int nchars;

    // Usage section
    int prefix_size = printf("usage: %s", program_name) + 1; // +1 account for space added by first option
    nmax = 80;
    nchars = 0;

    nchars += prefix_size;
    for (unsigned int i = 0; i < noptions; i++)
    {
        Option *argument = options + i;
        UsageStyle usage_style = OMIT;
        if (argument->long_name && argument->short_name)
            usage_style = ALL_NAMES;
        else if (argument->long_name)
            usage_style = LONG_NAME;
        else if (argument->short_name)
            usage_style = SHORT_NAME;
        if (nchars > nmax)
        {
            putchar('\n');
            nchars = printf("%*s", prefix_size, "");
        }
        else
        {
            putchar(' ');
            nchars++;
        }
        nchars += cli_print_option_usage(argument, usage_style, true, " |");
    }
    printf("\n%*s%s\n\n", prefix_size, "", positional_usage);

    // Synopsis section
    printf("%s\n", synopsis);

    // Options section
    printf("options:\n");

    nmax = 24;
    for (unsigned int i = 0; i < noptions; i++)
    {
        Option *argument = options + i;
        UsageStyle usage_style = OMIT;
        if (argument->long_name && argument->short_name)
            usage_style = ALL_NAMES;
        else if (argument->long_name)
            usage_style = LONG_NAME;
        else if (argument->short_name)
            usage_style = SHORT_NAME;
        nchars = printf("    ");
        nchars += cli_print_option_usage(argument, usage_style, false, ",");
        if (nchars > nmax - 2) // Two spaces of buffer
            printf("\n%*s", nmax, "");
        else
            printf("%*s", nmax - nchars, "");
        printf("%s\n", argument->description);
    }
}

void cli_print_short_help(unsigned int noptions, Option *options, char *program_name, char *positional_usage)
{
    printf("usage: %s", program_name);
    for (unsigned int i = 0; i < noptions; i++)
    {
        Option *argument = options + i;
        if (argument->usage_style == OMIT)
            continue;
        putchar(' ');
        cli_print_option_usage(argument, argument->usage_style, true, " |");
    }
    printf(" %s\n", positional_usage);
}

int cli_print_option_usage(Option *argument, UsageStyle usage_style, const bool brackets, const char *style_sep)
{
    const char *arg_sep = "";
    const char *arg = "";
    if (argument->has_arg)
    {
        arg_sep = " ";
        arg = argument->usage_arg;
    }
    const char *lbracket = "";
    const char *rbracket = "";
    if (brackets)
    {
        lbracket = "[";
        rbracket = "]";
    }
    switch (usage_style) // This can be poorly formatted if the usage_style does not match the option members
    {
    case OMIT:
        return 0;
    case SHORT_NAME:
        return printf("%s"
                      "-%c"
                      "%s%s%s",
                      lbracket,
                      argument->short_name, arg_sep, arg,
                      rbracket);
    case LONG_NAME:
        return printf("%s"
                      "--%s%s%s"
                      "%s",
                      lbracket,
                      argument->long_name, arg_sep, arg,
                      rbracket);
    case ALL_NAMES:
        return printf("%s"
                      "--%s%s%s"
                      "%s "
                      "-%c%s%s"
                      "%s",
                      lbracket,
                      argument->long_name, arg_sep, arg,
                      style_sep,
                      argument->short_name, arg_sep, arg,
                      rbracket);
    default:
        return 0;
    }
}
