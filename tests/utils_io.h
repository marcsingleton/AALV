#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <string.h>

#include "sequences.h"
#include "utils.h"

int records_equal(SeqRecordArray *record_array_1, SeqRecordArray *record_array_2)
{
    if (record_array_1->len != record_array_2->len)
    {
        test_result_printf("len mismatch: %zu != %zu\n", record_array_1->len, record_array_2->len);
        return 0;
    }

    for (size_t i = 0; i < record_array_1->len; i++)
    {
        SeqRecord *record1 = record_array_1->data + i;
        SeqRecord *record2 = record_array_2->data + i;

        if (strcmp(record1->header, record2->header) != 0)
        {
            test_result_printf("header mismatch: %s != %s\n", record1->header, record2->header);
            return 0;
        }
        if (strcmp(record1->id, record2->id) != 0)
        {
            test_result_printf("id mismatch: %s != %s\n", record1->id, record2->id);
            return 0;
        }
        if (strcmp(record1->seq, record2->seq) != 0)
        {
            test_result_printf("seq mismatch: %s != %s\n", record1->seq, record2->seq);
            return 0;
        }
        if (record1->len != record2->len)
        {
            test_result_printf("seq len mismatch: %zu != %zu\n", record1->len, record2->len);
            return 0;
        }
    }

    return 1;
}

#endif // IO_UTILS_H
