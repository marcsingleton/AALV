#include <stdio.h>
#include <string.h>

#include "fasta.h"
#include "seqrecord.h"
#include "utils.h"

#define MODULE_NAME "test_fasta"

#define SEQ1 "This is the first sequence."
#define SEQ2 "This is the second sequence. It is a bit longer than the first."
#define SEQ3 "This is the third sequence!"
SeqRecord records[] = {
    {.header = "id1 metadata1", .seq = SEQ1, .len = sizeof(SEQ1)},
    {.header = "id2 metadata2", .seq = SEQ2, .len = sizeof(SEQ2)},
    {.header = "id3 metadata3", .seq = SEQ3, .len = sizeof(SEQ3)},
};
#define NRECORDS sizeof(records) / sizeof(SeqRecord)

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
    int bufferlen = 1024;
    char buffer[bufferlen];
    FILE *fp = fmemopen(buffer, bufferlen, "rw");
    fasta_fwrite(fp, records, NRECORDS, 100);
    fseek(fp, 0, SEEK_SET);
    SeqRecord *new_records = NULL;
    int nrecords = fasta_fread(fp, &new_records);
    int code = records_equal(records, new_records, 2);
    if (code == 1)
        return 0;
    return 1;
}

int test_no_header()
{
    int bufferlen = 1024;
    char buffer[bufferlen];
    FILE *fp = fmemopen(buffer, bufferlen, "rw");
    fasta_wrap_string(fp, records[0].seq, 100);
    fasta_fwrite(fp, records + 1, NRECORDS - 1, 100);
    fseek(fp, 0, SEEK_SET);
    SeqRecord *new_records = NULL;
    int nrecords = fasta_fread(fp, &new_records);
    if (nrecords == FASTA_ERROR_INVALID_FORMAT)
        return 0;
    return 1;
}

TestFunction tests[] = {
    {&test_read_write, "test_read_write"},
    {&test_no_header, "test_no_header"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main()
{
    run_tests(tests, NTESTS, MODULE_NAME);
}