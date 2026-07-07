#include <string.h>
#include <sys/errno.h>

#include "argparse.h"
#include "config.h"
#include "error.h"
#include "io.h"

extern char *INVOCATION_NAME;

int io_load_seqs(FileState *file, const char *format_arg)
{
    // Infer reader
    FileReader reader;
    if (format_arg[0] != '\0') // From format argument
    {
        reader = argparse_reader(format_arg, n_format_options, format_options);
        if (!reader)
        {
            error_printf("%s: %s: Error identifying format\n", INVOCATION_NAME, format_arg);
            return -1;
        }
    }
    else if ((format_arg = strrchr(file->file_path, '.'))) // From path extension
    {
        format_arg++; // Exclude dot from comparison
        reader = argparse_reader(format_arg, n_format_options, format_options);
        if (!reader)
        {
            error_printf("%s: %s: Unknown extension\n", INVOCATION_NAME, file->file_path);
            return -1;
        }
    }
    else
    {
        error_printf("%s: %s: No format or known extension\n", INVOCATION_NAME, file->file_path);
        return -1;
    }

    // Read file
    FILE *fp;
    if (strcmp(file->file_path, "-") == 0)
        fp = stdin;
    else if (!(fp = fopen(file->file_path, "r")))
    {
        error_printf("%s: %s: %s\n", INVOCATION_NAME, file->file_path, strerror(errno));
        return -1;
    }
    SeqRecordArray *record_array = &file->record_array;
    int retcode = reader(fp, record_array);
    if (retcode != 0)
    {
        error_printf("%s: %s: Error processing file (code %d)\n", INVOCATION_NAME, file->file_path, retcode);
        return -1;
    }

    // Get max_len
    size_t max_len = 0;
    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->data + i;
        if (record->len > max_len)
            max_len = record->len;
    }
    if (max_len > INT_MAX)
    {
        error_printf("%s: %s: A sequence exceeds the maximum length\n", INVOCATION_NAME, file->file_path);
        return -1;
    }
    file->metadata.max_len = max_len;

    return 0;
}

int io_set_seq_types(FileState *file, const char *seq_type_arg)
{
    SeqRecordArray *record_array = &(file->record_array);
    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->data + i;
        SeqType seq_type_from_seq = sequences_seq_to_seq_type(record->seq);
        SeqType seq_type_from_arg = argparse_seq_type(seq_type_arg, n_seq_type_options, seq_type_options);
        if (seq_type_from_seq == SEQ_TYPE_ERROR && seq_type_from_arg != SEQ_TYPE_UNKNOWN) // Must explicitly specify UTF
            return -1;

        if (seq_type_from_arg != SEQ_TYPE_UNSPECIFIED) // Use provided arg if given
            record->type = seq_type_from_arg;
        else if (seq_type_from_seq == SEQ_TYPE_RNA || seq_type_from_seq == SEQ_TYPE_DNA)
            record->type = SEQ_TYPE_NUCLEIC;
        else if (seq_type_from_seq == SEQ_TYPE_INDETERMINATE && record->len >= config_nucleic_tiebreak_len)
            record->type = SEQ_TYPE_NUCLEIC;
        else
            record->type = seq_type_from_seq;
    }
    return 0;
}

int io_set_unaligned_indices(FileState *file)
{

    if (file->record_array.len == 0)
        return 0;
    if (sequences_init_unaligned_indices_array(&file->metadata.indices_array, file->record_array.len) != 0)
        return -1;

    for (size_t i = 0; i < file->record_array.len; i++)
    {
        SeqRecord *record = file->record_array.data + i;
        UnalignedIndices *unaligned_indices = file->metadata.indices_array.data + i;
        Alphabet *alphabet = sequences_seq_type_to_alphabet(record->type);
        if (record->len > 0 && alphabet)
            sequences_index_nongap_syms(alphabet, record, unaligned_indices);
    }
    return 0;
}
