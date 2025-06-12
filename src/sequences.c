#include "sequences.h"

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
