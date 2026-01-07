#ifndef SEQUENCES_H
#define SEQUENCES_H

/*
 * Sequence data structures
 */

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum
{
    SEQ_TYPE_UNSPECIFIED = 0,
    SEQ_TYPE_NUCLEIC,
    SEQ_TYPE_PROTEIN,
    SEQ_TYPE_INDETERMINATE, // Nucleic or protein
    SEQ_TYPE_UNKNOWN,       // ASCII with unregistered alphabet
    SEQ_TYPE_ERROR,
} SeqType;

typedef struct
{
    char *name;
    char *syms;
    char *gaps;
    unsigned int len;
    bool case_sensitive;
    int index_map[CHAR_MAX];
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
    SeqRecord *data;
    size_t len;
} SeqRecordArray;

extern Alphabet NUCLEIC_ALPHABET;
extern Alphabet PROTEIN_ALPHABET;

extern Alphabet *BASE_ALPHABETS[];
extern size_t N_BASE_ALPHABETS;

int sequences_init_seq_record(SeqRecord *record, char *header, char *seq, char *id);
void sequences_deinit_seq_record(SeqRecord *record);
SeqRecord *sequences_create_seq_record(char *header, char *seq, char *id);
void sequences_destroy_seq_record(SeqRecord *record);
int sequences_init_seq_record_array(SeqRecordArray *record_array, size_t len);
void sequences_deinit_seq_record_array(SeqRecordArray *record_array);
int sequences_init_alphabet(Alphabet *alphabet, char *name, char *syms, char *gaps, bool case_sensitive);
int sequences_init_base_alphabets(void);
int sequences_sym_in_alphabet(Alphabet *alphabet, char sym);
int sequences_sym_is_gap(Alphabet *alphabet, char sym);
int sequences_seq_in_alphabet(Alphabet *alphabet, SeqRecord *record);
int sequences_seq_is_nucleic(SeqRecord *record);
int sequences_seq_is_protein(SeqRecord *record);
int sequences_infer_seq_type(SeqRecord *record);
#endif // SEQUENCES_H
