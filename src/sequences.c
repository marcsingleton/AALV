#include <ctype.h>

#include "sequences.h"

Alphabet NUCLEIC_ALPHABET = {.name = "nucleic", .syms = "ACGTUN.-", .case_sensitive = false};
Alphabet PROTEIN_ALPHABET = {.name = "protein", .syms = "ACDEFGHIKLMNPQRSTVWYX.-", .case_sensitive = false};

void sequences_free_seq_records(SeqRecord *records, size_t nrecords)
{
    if (records == NULL)
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
    sequences_free_seq_records(record_array->records, record_array->len);
    record_array->len = 0;
}

int sequences_init_alphabet(Alphabet *alphabet, char *name, char *syms, bool case_sensitive)
{
    if (!alphabet || !name || !syms)
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
        code = sequences_init_alphabet(alphabet, alphabet->name, alphabet->syms, alphabet->case_sensitive);
        if (code != 0)
            return code;
    }
    return code;
}

int sequences_in_alphabet(Alphabet *alphabet, SeqRecord *record)
{
    for (char *sym = record->seq; *sym != '\0'; sym++)
    {
        if (!isascii(*sym))
            return -1;
        unsigned int index = *sym;
        if (alphabet->index_map[index] == -1)
            return 0;
    }
    return 1;
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
