#include <ctype.h>

#include "sequences.h"

Alphabet NUCLEIC_ALPHABET = {
    .name = "nucleic",
    .syms = "ACGTUN.-",
    .gaps = ".-",
    .case_sensitive = false,
};
Alphabet PROTEIN_ALPHABET = {
    .name = "protein",
    .syms = "ACDEFGHIKLMNPQRSTVWYX.-",
    .gaps = ".-",
    .case_sensitive = false,
};

Alphabet *BASE_ALPHABETS[] = {&NUCLEIC_ALPHABET, &PROTEIN_ALPHABET};
size_t N_BASE_ALPHABETS = sizeof(BASE_ALPHABETS) / sizeof(Alphabet *);

void sequences_free_seq_records(SeqRecord *records, size_t nrecords)
{
    if (!records)
        return;
    for (size_t i = 0; i < nrecords; i++)
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
    if (!record_array)
        return;
    sequences_free_seq_records(record_array->records, record_array->len);
    record_array->len = 0;
}

int sequences_init_alphabet(Alphabet *alphabet, char *name, char *syms, char *gaps, bool case_sensitive)
{
    if (!alphabet || !name || !syms || !gaps)
        return 1;
    alphabet->name = name;
    alphabet->syms = syms;
    for (unsigned int i = 0; i < 128; i++)
        alphabet->index_map[i] = -1;
    unsigned short len = 0;
    for (char *sym = alphabet->syms; *sym != '\0'; sym++)
    {
        if (!isascii(*sym))
            return 1;
        if (!case_sensitive && isalpha(*sym))
        {
            unsigned int index_0, index_1;
            if (isupper(*sym))
            {
                index_0 = *sym;
                index_1 = tolower(*sym);
            }
            else
            {
                index_0 = toupper(*sym);
                index_1 = *sym;
            }
            if (alphabet->index_map[index_0] != -1)
                continue;
            alphabet->index_map[index_0] = len;
            alphabet->index_map[index_1] = len;
            len++;
        }
        else
        {
            unsigned int index = *sym;
            if (alphabet->index_map[index] != -1)
                continue;
            alphabet->index_map[index] = len;
            len++;
        }
    }
    alphabet->len = len;
    for (char *sym = gaps; *sym != '\0'; sym++)
        if (sequences_sym_in_alphabet(alphabet, *sym) < 1)
            return 1;
    alphabet->gaps = gaps;
    return 0;
}

int sequences_init_base_alphabets(void)
{
    int retcode = 0;
    for (unsigned int i = 0; i < N_BASE_ALPHABETS; i++)
    {
        Alphabet *alphabet = BASE_ALPHABETS[i];
        retcode = sequences_init_alphabet(alphabet, alphabet->name,
                                          alphabet->syms, alphabet->gaps,
                                          alphabet->case_sensitive);
        if (retcode != 0)
            return retcode;
    }
    return retcode;
}

int sequences_sym_in_alphabet(Alphabet *alphabet, char sym)
{
    if (!alphabet)
        return -1;
    if (!isascii(sym))
        return -1;
    unsigned int index = sym;
    if (alphabet->index_map[index] == -1)
        return 0;
    return 1;
}

int sequences_sym_is_gap(Alphabet *alphabet, char sym)
{
    if (!alphabet)
        return -1;
    int retcode = sequences_sym_in_alphabet(alphabet, sym);
    if (retcode < 1)
        return retcode;
    for (char *gap_sym = alphabet->gaps; *gap_sym != 0; gap_sym++)
        if (*gap_sym == sym)
            return 1;
    return 0;
}

int sequences_seq_in_alphabet(Alphabet *alphabet, SeqRecord *record)
{
    if (!alphabet || !record)
        return -1;
    for (char *sym = record->seq; *sym != '\0'; sym++)
    {
        int retcode = sequences_sym_in_alphabet(alphabet, *sym);
        if (retcode < 1)
            return retcode;
    }
    return 1;
}

int sequences_seq_is_nucleic(SeqRecord *record)
{
    return sequences_seq_in_alphabet(&NUCLEIC_ALPHABET, record);
}

int sequences_seq_is_protein(SeqRecord *record)
{
    return sequences_seq_in_alphabet(&PROTEIN_ALPHABET, record);
}

int sequences_infer_seq_type(SeqRecord *record)
{
    int is_nucleic = sequences_seq_is_nucleic(record);
    int is_protein = sequences_seq_is_protein(record);
    if ((is_nucleic == 1) && (is_protein == 1))
        record->type = SEQ_TYPE_INDETERMINATE;
    else if (is_nucleic == 1)
        record->type = SEQ_TYPE_NUCLEIC;
    else if (is_protein == 1)
        record->type = SEQ_TYPE_PROTEIN;
    else if ((is_nucleic == -1) || (is_protein == -1))
    {
        record->type = SEQ_TYPE_ERROR;
        return 2;
    }
    else
    {
        record->type = SEQ_TYPE_UNKNOWN;
        return 1;
    }
    return 0;
}
