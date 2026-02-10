#include <ctype.h>
#include <string.h>

#include "sequences.h"

Alphabet RNA_ALPHABET = {
    .name = "rna",
    .syms = "ACGUN.-",
    .gaps = ".-",
    .case_sensitive = false,
};
Alphabet DNA_ALPHABET = {
    .name = "dna",
    .syms = "ACGTN.-",
    .gaps = ".-",
    .case_sensitive = false,
};
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

Alphabet *BASE_ALPHABETS[] = {
    &RNA_ALPHABET,
    &DNA_ALPHABET,
    &NUCLEIC_ALPHABET,
    &PROTEIN_ALPHABET,
};
size_t N_BASE_ALPHABETS = sizeof(BASE_ALPHABETS) / sizeof(Alphabet *);

int sequences_init_seq_record(SeqRecord *record, char *header, char *seq, char *id)
{
    if (!record || !header || !seq || !id)
        return 1;
    char *new_header = strdup(header);
    char *new_seq = strdup(seq);
    char *new_id = strdup(id);
    if (!new_header || !new_seq || !new_id)
    {
        free(new_header);
        free(new_seq);
        free(new_id);
        return 1;
    }
    record->header = new_header;
    record->seq = new_seq;
    record->id = new_id;
    record->type = SEQ_TYPE_UNSPECIFIED;
    return 0;
}

void sequences_deinit_seq_record(SeqRecord *record)
{
    if (!record)
        return;
    free(record->header);
    free(record->seq);
    free(record->id);
    record->type = SEQ_TYPE_UNSPECIFIED;
}

SeqRecord *sequences_create_seq_record(char *header, char *seq, char *id)
{
    SeqRecord *record = malloc(sizeof(SeqRecord));
    if (!record)
        return NULL;
    int retcode = sequences_init_seq_record(record, header, seq, id);
    if (retcode > 0)
    {
        free(record);
        return NULL;
    }
    return record;
}

void sequences_destroy_seq_record(SeqRecord *record)
{
    sequences_deinit_seq_record(record);
    free(record);
}

int sequences_init_seq_record_array(SeqRecordArray *record_array, size_t len)
{
    if (!record_array)
        return 1;
    SeqRecord *data = calloc(len, sizeof(SeqRecord));
    if (!data)
        return 1;
    record_array->data = data;
    record_array->len = len;
    return 0;
}

void sequences_deinit_seq_record_array(SeqRecordArray *record_array)
{
    if (!record_array)
        return;
    for (size_t i = 0; i < record_array->len; i++)
        sequences_deinit_seq_record(record_array->data + i);
    free(record_array->data);
    record_array->len = 0;
}

int sequences_init_alphabet(Alphabet *alphabet, char *name, char *syms, char *gaps, bool case_sensitive)
{
    if (!alphabet || !name || !syms || !gaps)
        return 1;
    Alphabet test_alphabet;
    char *new_name = strdup(name);
    char *new_syms = strdup(syms);
    char *new_gaps = strdup(gaps);
    if (!new_name || !new_syms || !new_gaps)
        goto error;

    // Check syms
    for (unsigned int i = 0; i < CHAR_MAX; i++)
        test_alphabet.index_map[i] = -1;
    unsigned short len = 0;
    for (char *sym = syms; *sym != '\0'; sym++)
    {
        if (!isascii(*sym))
            goto error;
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
            if (test_alphabet.index_map[index_0] != -1)
                continue;
            test_alphabet.index_map[index_0] = len;
            test_alphabet.index_map[index_1] = len;
            len++;
        }
        else
        {
            unsigned int index = *sym;
            if (test_alphabet.index_map[index] != -1)
                continue;
            test_alphabet.index_map[index] = len;
            len++;
        }
    }

    // Check gaps
    for (char *sym = gaps; *sym != '\0'; sym++)
        if (sequences_sym_in_alphabet(&test_alphabet, *sym) < 1)
            goto error;

    alphabet->name = new_name;
    alphabet->syms = new_syms;
    alphabet->gaps = new_gaps;
    alphabet->len = len;
    alphabet->case_sensitive = case_sensitive;
    memcpy(alphabet->index_map, test_alphabet.index_map, sizeof(test_alphabet.index_map));
    return 0;

error:
    free(new_name);
    free(new_syms);
    free(new_gaps);
    return 1;
}

void sequences_deinit_alphabet(Alphabet *alphabet)
{
    if (!alphabet)
        return;
    for (unsigned int i = 0; i < CHAR_MAX; i++)
        alphabet->index_map[i] = -1;
    free(alphabet->name);
    free(alphabet->syms);
    free(alphabet->gaps);
    alphabet->name = NULL;
    alphabet->syms = NULL;
    alphabet->gaps = NULL;
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
        if (retcode > 0)
            return retcode;
    }
    return retcode;
}

void sequences_deinit_base_alphabets(void)
{
    for (unsigned int i = 0; i < N_BASE_ALPHABETS; i++)
    {
        Alphabet *alphabet = BASE_ALPHABETS[i];
        sequences_deinit_alphabet(alphabet);
    }
}

int sequences_init_unaligned_indices(UnalignedIndices *unaligned_indices, size_t len)
{
    if (!unaligned_indices)
        return 1;
    size_t *indices = malloc(len * sizeof(size_t));
    if (!indices)
        return 1;
    unaligned_indices->indices = indices;
    unaligned_indices->len = len;
    return 0;
}

void sequences_deinit_unaligned_indices(UnalignedIndices *unaligned_indices)
{
    if (!unaligned_indices)
        return;
    free(unaligned_indices->indices);
    unaligned_indices->len = 0;
}

int sequences_init_unaligned_indices_array(UnalignedIndicesArray *indices_array, size_t len)
{
    if (!indices_array)
        return 1;
    UnalignedIndices *data = calloc(len, sizeof(UnalignedIndices));
    if (!data)
        return 1;
    indices_array->data = data;
    indices_array->len = len;
    return 0;
}

void sequences_deinit_unaligned_indices_array(UnalignedIndicesArray *indices_array)
{
    if (!indices_array)
        return;
    for (size_t i = 0; i < indices_array->len; i++)
        sequences_deinit_unaligned_indices(indices_array->data + i);
    free(indices_array->data);
    indices_array->len = 0;
}

int sequences_index_nongap_syms(Alphabet *alphabet, SeqRecord *record, UnalignedIndices *unaligned_indices)
{
    if (!alphabet || !record || !unaligned_indices)
        return 1;
    int retcode = sequences_init_unaligned_indices(unaligned_indices, record->len);
    if (retcode > 0)
        return 1;
    size_t index = 0;
    for (size_t i = 0; i < record->len; i++)
    {
        unaligned_indices->indices[i] = index;

        char sym = record->seq[i];
        int is_gap = sequences_sym_is_gap(alphabet, sym);
        if (!is_gap)
            index++;
    }
    return 0;
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

int sequences_seq_in_alphabet(Alphabet *alphabet, char *seq)
{
    if (!alphabet || !seq)
        return -1;
    for (char *sym = seq; *sym != '\0'; sym++)
    {
        int retcode = sequences_sym_in_alphabet(alphabet, *sym);
        if (retcode < 1)
            return retcode;
    }
    return 1;
}

int sequences_seq_is_rna(char *seq)
{
    return sequences_seq_in_alphabet(&RNA_ALPHABET, seq);
}

int sequences_seq_is_dna(char *seq)
{
    return sequences_seq_in_alphabet(&DNA_ALPHABET, seq);
}

int sequences_seq_is_nucleic(char *seq)
{
    return sequences_seq_in_alphabet(&NUCLEIC_ALPHABET, seq);
}

int sequences_seq_is_protein(char *seq)
{
    return sequences_seq_in_alphabet(&PROTEIN_ALPHABET, seq);
}

SeqType sequences_seq_to_seq_type(char *seq)
{
    int is_rna = sequences_seq_is_rna(seq);
    int is_dna = sequences_seq_is_dna(seq);
    int is_nucleic = sequences_seq_is_nucleic(seq);
    int is_protein = sequences_seq_is_protein(seq);

    if (is_rna == -1 || is_dna == -1 || is_nucleic == -1 || is_protein == -1)
        return SEQ_TYPE_ERROR;

    if ((is_nucleic == 1) && (is_protein == 1))
        return SEQ_TYPE_INDETERMINATE;
    else if (is_nucleic == 1)
    {
        if (is_rna == 1 && is_dna == 0)
            return SEQ_TYPE_RNA;
        else if (is_rna == 0 && is_dna == 1)
            return SEQ_TYPE_DNA;
        else if (is_rna == 1 && is_dna == 1) // No T or U
            return SEQ_TYPE_NUCLEIC;
        else // T and U
            return SEQ_TYPE_NUCLEIC;
    }
    else if (is_protein == 1)
        return SEQ_TYPE_PROTEIN;
    else
        return SEQ_TYPE_UNKNOWN;
}

Alphabet *sequences_seq_type_to_alphabet(SeqType type)
{
    Alphabet *alphabet;
    switch (type)
    {
    case SEQ_TYPE_RNA:
        alphabet = &RNA_ALPHABET;
        break;
    case SEQ_TYPE_DNA:
        alphabet = &DNA_ALPHABET;
        break;
    case SEQ_TYPE_NUCLEIC:
        alphabet = &NUCLEIC_ALPHABET;
        break;
    case SEQ_TYPE_PROTEIN:
        alphabet = &PROTEIN_ALPHABET;
        break;
    default:
        alphabet = NULL;
    }
    return alphabet;
}
