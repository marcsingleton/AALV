#include "fasta.h"
#include "io.h"
#include "str.h"

FormatOption format_options[] = {
    {"FASTA", "fasta,fa,faa,fna,afa", &fasta_fread},
    {"A2M/A3M", "a2m,a3m", &fasta_fread},
    // CLUSTAL
    // PHYLIP
    // STOCKHOLM
};
unsigned int n_format_options = sizeof(format_options) / sizeof(FormatOption);

SeqTypeOption seq_type_options[] = {
    {"nucleic", "nucleic,nt", SEQ_TYPE_NUCLEIC, &nucleic_alphabet},
    {"protein", "protein,aa", SEQ_TYPE_PROTEIN, &protein_alphabet},
    {"unicode", "unicode,utf", SEQ_TYPE_UNKNOWN, NULL},
};
unsigned int n_seq_type_options = sizeof(seq_type_options) / sizeof(SeqTypeOption);

FileReader io_get_reader(const char *format_arg,
                         unsigned int n_format_options,
                         FormatOption *format_options)
{
    FileReader reader = NULL;
    for (unsigned int i = 0; i < n_format_options; i++)
    {
        FormatOption *format_option = format_options + i;
        const char *exts = format_option->exts;
        if (str_is_in_strsep(exts, OPTION_DELIM, format_arg))
            return format_option->reader;
    }
    return reader;
}

SeqType io_get_seq_type(const char *seq_type_arg,
                        unsigned int n_seq_type_options,
                        SeqTypeOption *seq_type_options)
{
    SeqType seq_type = SEQ_TYPE_UNSPECIFIED;
    for (unsigned int i = 0; i < n_seq_type_options; i++)
    {
        SeqTypeOption *seq_type_option = seq_type_options + i;
        const char *identifiers = seq_type_option->identifiers;
        if (str_is_in_strsep(identifiers, OPTION_DELIM, seq_type_arg))
            return seq_type_option->type;
    }
    return seq_type;
}

int io_load_seqs(FileState *file, FILE *fp, FileReader reader)
{
    SeqRecordArray *record_array = &(file->record_array);
    int retcode = reader(fp, record_array);
    if (retcode != 0)
        return -1;

    size_t max_len = 0;
    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->data + i;
        if (record->len > max_len)
            max_len = record->len;
    }
    if (max_len > INT_MAX)
        return -2;
    file->metadata.max_len = max_len;

    return 0;
}

int io_set_seq_types(FileState *file, const char *seq_type_arg, unsigned int nucleic_tiebreak_len)
{
    SeqRecordArray *record_array = &(file->record_array);
    SeqType seq_type_from_arg = io_get_seq_type(seq_type_arg, n_seq_type_options, seq_type_options);
    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->data + i;
        SeqType seq_type_from_seq = sequences_seq_to_seq_type(record->seq);
        if (seq_type_from_seq == SEQ_TYPE_ERROR && seq_type_from_arg != SEQ_TYPE_UNKNOWN) // Must explicitly specify UTF
            return -1;

        if (seq_type_from_arg != SEQ_TYPE_UNSPECIFIED) // Use provided arg if given
            record->type = seq_type_from_arg;
        else if (seq_type_from_seq == SEQ_TYPE_RNA || seq_type_from_seq == SEQ_TYPE_DNA)
            record->type = SEQ_TYPE_NUCLEIC;
        else if (seq_type_from_seq == SEQ_TYPE_INDETERMINATE && record->len >= nucleic_tiebreak_len)
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
