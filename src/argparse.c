#include <getopt.h>
#include <string.h>

#include "array.h"
#include "argparse.h"
#include "error.h"
#include "macros.h"
#include "str.h"

extern char *INVOCATION_NAME;

// --help option shows in given order (alphabetical except help and version)
Option options[] = {
    {"help",
     'h',
     "print usage and options then exit",
     "",
     SHORT_NAME,
     no_argument},
    {"version",
     'v',
     "print version then exit",
     "",
     OMIT,
     no_argument},
    {"format",
     'f',
     "comma-separated list of format extensions for input files",
     "<fmt,...,fmt>",
     SHORT_NAME,
     required_argument},
    {"list-formats",
     0,
     "list allowable formats and their recognized extensions then exit",
     "",
     OMIT,
     no_argument},
    {"list-types",
     0,
     "list allowable types and their recognized identifiers then exit",
     "",
     OMIT,
     no_argument},
    {"type",
     't',
     "comma-separated list of sequence types for input files",
     "<type,...,type>",
     SHORT_NAME,
     required_argument},
};
unsigned int noptions = sizeof(options) / sizeof(Option);

FormatOption format_options[] = {
    {"FASTA", "fasta,fa,faa,fna,afa", &fasta_fread},
    {"A2M/A3M", "a2m,a3m", &fasta_fread},
    // CLUSTAL
    // PHYLIP
    // STOCKHOLM
};
unsigned int n_format_options = sizeof(format_options) / sizeof(FormatOption);

SeqTypeOption seq_type_options[] = {
    {"nucleic", "nucleic,nt", SEQ_TYPE_NUCLEIC, &NUCLEIC_ALPHABET},
    {"protein", "protein,aa", SEQ_TYPE_PROTEIN, &PROTEIN_ALPHABET},
};
unsigned int n_seq_type_options = sizeof(seq_type_options) / sizeof(SeqTypeOption);

char *option_delim = ",";

char synopsis[] = PROGRAM_NAME " is a vim-inspired alignment viewer\n";
char positional_usage[] = "[<file> ...]";

int argparse_options(int argc, char *argv[],
                     unsigned int noptions, Option *options,
                     unsigned int n_format_options, FormatOption *format_options,
                     unsigned int n_seq_type_options, SeqTypeOption *seq_type_options,
                     const char *short_options, const struct option *long_options,
                     unsigned int *n_format_args, char ***format_args_ptr,
                     unsigned int *n_seq_type_args, char ***seq_type_args_ptr,
                     char *program_name, char *positional_usage, char *synopsis)
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
            cli_print_short_help(noptions, options, program_name, positional_usage);
            return -2;
        }

        // Process option
        const char *name = "";
        if (option_index != -1)
            name = options[option_index].long_name;
        if (c == 'f' || strcmp(name, "format") == 0)
        {
            ssize_t nwords = str_split(format_args_ptr, argv[optind - 1], option_delim);
            if (nwords < 0)
            {
                error_printf("%s: Failed to parse formats\n", INVOCATION_NAME);
                return -2;
            }
            *n_format_args = nwords;
        }
        else if (c == 'h')
        {
            cli_print_short_help(noptions, options, program_name, positional_usage);
            return -1;
        }
        else if (strcmp(name, "help") == 0)
        {
            cli_print_long_help(noptions, options, program_name, positional_usage, synopsis);
            return -1;
        }
        else if (strcmp(name, "list-formats") == 0)
        {
            printf("Format\tExtensions\n");
            for (unsigned int i = 0; i < n_format_options; i++)
            {
                FormatOption *format_option = format_options + i;
                printf("%s\t%s\n", format_option->name, format_option->exts);
            }
            return -1;
        }
        else if (strcmp(name, "list-types") == 0)
        {
            printf("Type\tIdentifiers\n");
            for (unsigned int i = 0; i < n_seq_type_options; i++)
            {
                SeqTypeOption *seq_type_option = seq_type_options + i;
                printf("%s\t%s\n", seq_type_option->name, seq_type_option->identifiers);
            }
            return -1;
        }
        else if (c == 't' || strcmp(name, "type") == 0)
        {
            ssize_t nwords = str_split(seq_type_args_ptr, argv[optind - 1], option_delim);
            if (nwords < 0)
            {
                error_printf("%s: Failed to parse types\n", INVOCATION_NAME);
                return -2;
            }
            *n_seq_type_args = nwords;
        }
        else if (c == 'v' || strcmp(name, "version") == 0)
        {
            printf(PROGRAM_NAME " " VERSION "\n");
            return -1;
        }
    }
    return 0;
}

FileReader argparse_reader(const char *format_arg,
                           unsigned int n_format_options,
                           FormatOption *format_options)
{
    FileReader reader = NULL;
    for (unsigned int i = 0; i < n_format_options; i++)
    {
        FormatOption *format_option = format_options + i;
        const char *exts = format_option->exts;
        if (str_is_in_strsep(exts, option_delim, format_arg))
            return format_option->reader;
    }
    return reader;
}

SeqType argparse_seq_type(const char *seq_type_arg,
                          unsigned int n_seq_type_options,
                          SeqTypeOption *seq_type_options)
{
    SeqType seq_type = SEQ_TYPE_UNSPECIFIED;
    for (unsigned int i = 0; i < n_seq_type_options; i++)
    {
        SeqTypeOption *seq_type_option = seq_type_options + i;
        const char *identifiers = seq_type_option->identifiers;
        if (str_is_in_strsep(identifiers, option_delim, seq_type_arg))
            return seq_type_option->type;
    }
    return seq_type;
}
