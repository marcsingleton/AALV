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
#define MAXLEN 10      // Make small enough to ensure above records will wrap a few times

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

void wrap_string_with_blanks(FILE *fp, const char *s, const int len, const int maxlen)
{
    int nlines = len / maxlen;
    int j;
    for (j = 0; j < nlines; j++)
    {
        fwrite(s + j * maxlen, sizeof(char), maxlen, fp);
        fputs("\n\n", fp);
    }
    int nchars = len % maxlen;
    if (nchars > 0)
    {
        fwrite(s + j * maxlen, sizeof(char), nchars, fp);
        fputs("\n\n", fp);
    }
}

int test_read_write()
{
    char buffer[BUFFERLEN];
    FILE *fp = fmemopen(buffer, BUFFERLEN, "rw");
    fasta_fwrite(fp, records, NRECORDS, MAXLEN);
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
    fasta_wrap_string(fp, records[0].seq, records[0].len, MAXLEN);
    fasta_fwrite(fp, records + 1, NRECORDS - 1, MAXLEN);
    fseek(fp, 0, SEEK_SET);
    SeqRecord *new_records = NULL;
    int nrecords = fasta_fread(fp, &new_records);
    if (nrecords != FASTA_ERROR_INVALID_FORMAT)
        return 1;
    return 0;
}

int test_empty_file()
{
    char buffer[BUFFERLEN];
    FILE *fp = fmemopen(buffer, 1, "rw");
    fgetc(fp); // Consume the single byte of buffer
    SeqRecord *new_records = NULL;
    int nrecords = fasta_fread(fp, &new_records);
    if (nrecords != 0)
        return 1;
    return 0;
}

int test_blank_lines()
{
    char buffer[BUFFERLEN];
    FILE *fp = fmemopen(buffer, BUFFERLEN, "rw");
    fputs("\n\n\n", fp);
    for (int i = 0; i < NRECORDS; i++)
    {
        SeqRecord *record = records + i;
        fprintf(fp, ">%s\n\n", record->header);
        wrap_string_with_blanks(fp, record->seq, record->len, MAXLEN);
    }
    fseek(fp, 0, SEEK_SET);
    SeqRecord *new_records = NULL;
    int nrecords = fasta_fread(fp, &new_records);
    if (nrecords != NRECORDS)
        return 1;
    if (records_equal(records, new_records, NRECORDS) != 1)
        return 1;
    return 0;
}

int test_non_fasta()
{
    char buffer[BUFFERLEN] =
        "Here's a multiline\n"
        "file that's definitely not\n"
        "a FASTA.";
    FILE *fp = fmemopen(buffer, BUFFERLEN, "rw");
    SeqRecord *new_records = NULL;
    int nrecords = fasta_fread(fp, &new_records);
    if (nrecords != FASTA_ERROR_INVALID_FORMAT)
        return 1;
    return 0;
}

TestFunction tests[] = {
    {&test_read_write, "test_read_write"},
    {&test_no_header, "test_no_header"},
    {&test_empty_file, "test_empty_file"},
    {&test_blank_lines, "test_blank_lines"},
    {&test_non_fasta, "test_non_fasta"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main()
{
    run_tests(tests, NTESTS, MODULE_NAME);
}