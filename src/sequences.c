#include <ctype.h>

#include "sequences.h"

Alphabet NUCLEIC_ALPHABET = {.name = "nucleic", .syms = "ACGTN.-", .init = false};
Alphabet PROTEIN_ALPHABET = {.name = "protein", .syms = "ACDEFGHIKLMNPQRSTVWYX.-", .init = false};

void sequences_free_seq_records(SeqRecord *records, int nrecords)
{
    for (int i = 0; i < nrecords; i++)
    {
        SeqRecord record = records[i];
        free(record.header);
        free(record.id);
        free(record.seq);
    }
    free(records);
}

void sequences_free_seq_record_array(SeqRecordArray *record_array)
{
    sequences_free_seq_records(record_array->records, record_array->len);
    record_array->len = 0;
}

int sequences_init_alphabet(Alphabet *alphabet, char *name, char *syms)
{
    if (alphabet->init)
        return 0;
    alphabet->name = name;
    alphabet->syms = syms;
    for (unsigned int i = 0; i < 128; i++)
        alphabet->index_map[i] = -1;

    unsigned int len = 0;
    for (char *sym = alphabet->syms; *sym != '\0'; sym++)
    {
        if (!isascii(*sym))
            return 1;
        unsigned int index = *sym;
        alphabet->index_map[index] = len;
        len++;
    }
    alphabet->len = len;
    alphabet->init = true;
    return 0;
}

int sequences_init_base_alphabets(void)
{
    Alphabet *BASE_ALPHABETS[] = {&NUCLEIC_ALPHABET, &PROTEIN_ALPHABET};
    size_t N_BASE_ALPHABETS = sizeof(BASE_ALPHABETS) / sizeof(Alphabet *);

    int code = 0;
    for (unsigned int i = 0; i < N_BASE_ALPHABETS; i++)
    {
        Alphabet *alphabet = BASE_ALPHABETS[i];
        code = sequences_init_alphabet(alphabet, alphabet->name, alphabet->syms);
        if (code != 0)
            return code;
    }
    return code;
}

int sequences_in_alphabet(Alphabet *alphabet, SeqRecord *record)
{
    int code = 1;
    if (!alphabet->init)
        return -1;
    for (char *sym = record->seq; *sym != '\0'; sym++)
    {
        if (!isascii(*sym))
            return -1;
        unsigned int index = *sym;
        if (alphabet->index_map[index] == -1)
            code = 0;
    }
    return code;
}

int sequences_is_nucleic(SeqRecord *record)
{
    return sequences_in_alphabet(&NUCLEIC_ALPHABET, record);
}

int sequences_is_protein(SeqRecord *record)
{
    return sequences_in_alphabet(&PROTEIN_ALPHABET, record);
}

int sequences_infer_seq_type(SeqRecord *record)
{
    int is_nucleic = sequences_is_nucleic(record);
    int is_protein = sequences_is_protein(record);
    if ((is_nucleic == 1) && (is_protein == 1))
        record->type = SEQ_TYPE_INDETERMINATE;
    else if (is_nucleic == 1)
        record->type = SEQ_TYPE_NUCLEIC;
    else if (is_protein == 1)
        record->type = SEQ_TYPE_PROTEIN;
    else
    {
        record->type = SEQ_TYPE_ERROR;
        return 1;
    }
    return 0;
}
