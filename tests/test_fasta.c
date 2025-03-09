#include <stdio.h>
#include <string.h>

#include "fasta.h"
#include "seqrecord.h"
#include "utils.h"

#define MODULE_NAME "test_fasta"

SeqRecord records[] = {
    {"header1", NULL, "This is the first sequence.", 1},
    {"header2", NULL, "This is the second sequence. It is at least 50 characters.", 1},
};

int records_equal(SeqRecord *records_1, SeqRecord *records_2, int nrecords)
{
    for (int i = 0; i < nrecords; i++)
    {
        SeqRecord record_1, record_2;
        record_1 = records_1[i];
        record_2 = records_2[i];
        if (strcmp(record_1.header, record_2.header) != 0)
            return 0;
        if (strcmp(record_1.seq, record_2.seq) != 0)
            return 0;
    }
    return 1;
}

int test_read_write()
{
    char buffer[2048];
    FILE *fp = fmemopen(buffer, 2048, "rw");
    fasta_fwrite(fp, records, 2, 100);
    fseek(fp, 0, SEEK_SET);
    SeqRecord *new_records = NULL;
    int nrecords = fasta_parse(fp, &new_records);
    int code = records_equal(records, new_records, 2);
    if (code == 1)
        return 0;
    return 1;
}

TestFunction tests[] = {
    {&test_read_write, "test_read_write"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main()
{
    run_tests(tests, NTESTS, MODULE_NAME);
}