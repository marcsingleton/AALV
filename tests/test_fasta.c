#include <stdio.h>
#include <string.h>

#include "fasta.h"
#include "seqrecord.h"
#include "utils.h"

#define MODULE_NAME "test_fasta"

#define SEQ1 "This is the first sequence."
#define SEQ2 "This is the second sequence. It is a bit longer than the first."
#define SEQ3 "This is the third sequence in the array!"
SeqRecord records[] = {
    {.header = "id1 metadata1", .seq = SEQ1, .len = sizeof(SEQ1) - 1},
    {.header = "id2 metadata2", .seq = SEQ2, .len = sizeof(SEQ2) - 1},
    {.header = "id3 metadata3", .seq = SEQ3, .len = sizeof(SEQ3) - 1},
};
#define NRECORDS sizeof(records) / sizeof(SeqRecord)
#define BUFFERLEN 1024 // Must be large enough to hold above SeqRecords

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
    char buffer[BUFFERLEN];
    FILE *fp = fmemopen(buffer, BUFFERLEN, "rw");
    fasta_fwrite(fp, records, NRECORDS, 10);
    fseek(fp, 0, SEEK_SET);
    SeqRecord *new_records = NULL;
    int nrecords = fasta_fread(fp, &new_records);
    if (nrecords != NRECORDS)
        return 1;
    if (records_equal(records, new_records, NRECORDS) != 1)
        return 1;
    return 0;
}

int test_no_header()
{
    char buffer[BUFFERLEN];
    FILE *fp = fmemopen(buffer, BUFFERLEN, "rw");
    fasta_wrap_string(fp, records[0].seq, records[0].len, 10);
    fasta_fwrite(fp, records + 1, NRECORDS - 1, 10);
    fseek(fp, 0, SEEK_SET);
    SeqRecord *new_records = NULL;
    int nrecords = fasta_fread(fp, &new_records);
    if (nrecords != FASTA_ERROR_INVALID_FORMAT)
        return 1;
    return 0;
}

int test_empty_file()
{
    char buffer[BUFFERLEN] = "";
    FILE *fp = fmemopen(buffer, 1, "rw");
    SeqRecord *new_records = NULL;
    int nrecords = fasta_fread(fp, &new_records);
    if (nrecords != 0)
        return 1;
    return 0;
}

int test_non_fasta()
{
    return 1;
}

TestFunction tests[] = {
    {&test_read_write, "test_read_write"},
    {&test_no_header, "test_no_header"},
    {&test_empty_file, "test_empty_file"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main()
{
    run_tests(tests, NTESTS, MODULE_NAME);
}