#ifndef SEQUENCES_H
#define SEQUENCES_H

/*
 * Sequence data structures
 */

#include <stddef.h>
#include <stdlib.h>

typedef enum
{
    SEQ_TYPE_UNSPECIFIED = 0,
    SEQ_TYPE_NUCLEIC,
    SEQ_TYPE_PROTEIN,
    SEQ_TYPE_INDETERMINATE,
    SEQ_TYPE_ERROR,
} SeqType;

typedef struct
{
    char *name;
    char *syms;
    unsigned int len;
    int index_map[128];
} Alphabet;

typedef struct
{
    char *header;
    char *seq;
    char *id;
    size_t len;
    SeqType type;
} SeqRecord;

typedef struct
{
    SeqRecord *records;
    unsigned int len;
} SeqRecordArray;

extern Alphabet NUCLEIC_ALPHABET;
extern Alphabet PROTEIN_ALPHABET;

void sequences_free_seq_records(SeqRecord *records, size_t nrecords);
void sequences_free_seq_record_array(SeqRecordArray *record_array);
int sequences_init_alphabet(Alphabet *alphabet, char *name, char *syms);
int sequences_init_base_alphabets(void);
int sequences_in_alphabet(Alphabet *alphabet, SeqRecord *record);
int sequences_is_nucleic(SeqRecord *record);
int sequences_is_protein(SeqRecord *record);
int sequences_infer_seq_type(SeqRecord *record);
#endif // SEQUENCES_H
