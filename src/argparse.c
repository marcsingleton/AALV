#include <string.h>

#include "array.h"
#include "argparse.h"
#include "error.h"
#include "str.h"

#define PROGRAM_NAME "aalv" // TODO: Change to const

int parse_options(int argc, char *argv[],
                  unsigned int narguments, Argument *arguments,
                  unsigned int n_format_options, FormatOption *format_options,
                  unsigned int n_type_options, SeqTypeOption *type_options,
                  const char *short_options, const struct option *long_options,
                  unsigned int *n_format_args, char ***format_args_ptr,
                  unsigned int *n_type_args, char ***type_args_ptr)
{
    while (1)
    {
        // Check if next argument is a known option
        int option_index = -1;
        int c = getopt_long(argc, argv, short_options, long_options, &option_index);
        if (c == -1)
            break;
        else if (c == '?')
        {
            print_short_help(narguments, arguments);
            return 2;
        }

        // Process option
        const char *name = "";
        if (option_index != -1)
            name = arguments[option_index].long_name;
        if (c == 'f' || strcmp(name, "format") == 0)
        {
            int code = str_split(format_args_ptr, argv[optind - 1], ',');
            if (code < 0)
            {
                snprintf(error_message, ERROR_MESSAGE_LEN, "%s: Failed to parse formats\n", PROGRAM_NAME);
                return 2;
            }
            *n_format_args = code;
            printf("Identified the following formats:\n");
            for (unsigned int i = 0; i < *n_format_args; i++)
                printf("    %s\n", (*format_args_ptr)[i]);
        }
        else if (c == 'h')
        {
            print_short_help(narguments, arguments);
            return 1;
        }
        else if (strcmp(name, "help") == 0)
        {
            print_long_help(narguments, arguments);
            return 1;
        }
        else if (strcmp(name, "list-formats") == 0)
        {
            printf("Format\tExtensions\n");
            for (unsigned int i = 0; i < n_format_options; i++)
            {
                FormatOption *format_option = format_options + i;
                printf("%s\t%s\n", format_option->name, format_option->exts);
            }
            return 1;
        }
        else if (strcmp(name, "list-types") == 0)
        {
            printf("Type\tIdentifiers\n");
            for (unsigned int i = 0; i < n_type_options; i++)
            {
                SeqTypeOption *type_option = type_options + i;
                printf("%s\t%s\n", type_option->name, type_option->identifiers);
            }
            return 1;
        }
        else if (c == 't' || strcmp(name, "type") == 0)
        {
            int code = str_split(type_args_ptr, argv[optind - 1], ',');
            if (code < 0)
            {
                snprintf(error_message, ERROR_MESSAGE_LEN, "%s: Failed to parse types\n", PROGRAM_NAME);
                return 2;
            }
            *n_type_args = code;
            printf("Identified the following types:\n");
            for (unsigned int i = 0; i < *n_type_args; i++)
                printf("    %s\n", (*type_args_ptr)[i]);
        }
        else if (c == 'v' || strcmp(name, "version") == 0)
        {
            printf("aalv 0.1.0 dev\n");
            return 1;
        }
    }
    return 0;
}

int prepare_options(unsigned int narguments, Argument *arguments,
                    char **short_options_ptr, struct option *long_options)
{
    int code = 0;
    Array short_options_array;
    array_init(&short_options_array, sizeof(char));
    for (unsigned int i = 0; i < narguments; i++)
    {
        Argument *argument = arguments + i;

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
    if (short_options == NULL)
    {
        code = 1;
        snprintf(error_message, ERROR_MESSAGE_LEN,
                 "%s: Failed to allocate memory to create options string\n", PROGRAM_NAME);
        goto cleanup;
    }
    memcpy(short_options, short_options_array.data, short_options_array.len * short_options_array.size);
    *short_options_ptr = short_options;
cleanup:
    array_free(&short_options_array);
    return code;
}

void print_long_help(unsigned int narguments, Argument *arguments)
{
    unsigned int nmax;
    unsigned int nchars;

    // Usage section
    int prefix_size = printf("usage: %s", PROGRAM_NAME) + 1; // +1 account for space added by first option
    nmax = 80;
    nchars = 0;

    nchars += prefix_size;
    for (unsigned int i = 0; i < narguments; i++)
    {
        Argument *argument = arguments + i;
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
        nchars += print_option_usage(argument, usage_style, true, " |");
    }
    printf("\n%*s[<file> ...]\n\n", prefix_size, "");

    // Synopsis section
    printf("%s is a vim-inspired alignment viewer\n\n", PROGRAM_NAME);

    // Options section
    printf("options:\n");

    nmax = 24;
    for (unsigned int i = 0; i < narguments; i++)
    {
        Argument *argument = arguments + i;
        UsageStyle usage_style = OMIT;
        if (argument->long_name && argument->short_name)
            usage_style = ALL_NAMES;
        else if (argument->long_name)
            usage_style = LONG_NAME;
        else if (argument->short_name)
            usage_style = SHORT_NAME;
        nchars = printf("    ");
        nchars += print_option_usage(argument, usage_style, false, ",");
        if (nchars > nmax - 2) // Two spaces of buffer
            printf("\n%*s", nmax, "");
        else
            printf("%*s", nmax - nchars, "");
        printf("%s\n", argument->description);
    }
}

void print_short_help(unsigned int narguments, Argument *arguments)
{
    printf("usage: %s", PROGRAM_NAME);
    for (unsigned int i = 0; i < narguments; i++)
    {
        Argument *argument = arguments + i;
        if (argument->usage_style == OMIT)
            continue;
        putchar(' ');
        print_option_usage(argument, argument->usage_style, true, " |");
    }
    printf(" [<file> ...]\n");
}

int print_option_usage(Argument *argument, UsageStyle usage_style, const bool brackets, const char *style_sep)
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
    switch (usage_style) // This can be poorly formatted if the usage_style does not match the arguments
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
    }
}
