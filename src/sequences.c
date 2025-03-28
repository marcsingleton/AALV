#include "sequences.h"

void free_seq_record_array(SeqRecordArray *record_array)
{
    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->records + i;
        free(record->header);
        free(record->id);
        free(record->seq);
        record->len = 0;
    }
    free(record_array->records);
    record_array->len = 0;
}
