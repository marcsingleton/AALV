#include "sequences.h"
#include "utils.h"

#define MODULE_NAME "test_sequences"

int test_init_alphabet_base(void)
{
    Alphabet test_alphabet = {
        .name = "test",
        .syms = "ABC.-",
        .gaps = ".-",
        .case_sensitive = false,
    };
    unsigned int len = 5;

    int retcode = sequences_init_alphabet(&test_alphabet, test_alphabet.name,
                                          test_alphabet.syms, test_alphabet.gaps,
                                          test_alphabet.case_sensitive);
    if (retcode > 0)
        return retcode;

    if (test_alphabet.len != len)
        return 1;

    return 0;
}

int test_init_alphabet_duplicated_sym(void)
{
    Alphabet test_alphabet = {
        .name = "test",
        .syms = "AABC.-",
        .gaps = ".-",
        .case_sensitive = false,
    };
    unsigned int len = 5;

    int retcode = sequences_init_alphabet(&test_alphabet, test_alphabet.name,
                                          test_alphabet.syms, test_alphabet.gaps,
                                          test_alphabet.case_sensitive);
    if (retcode > 0)
        return retcode;

    if (test_alphabet.len != len)
        return 1;

    return 0;
}

int test_init_alphabet_out_of_alphabet_gap(void)
{
    Alphabet test_alphabet = {
        .name = "test",
        .syms = "ABC.-",
        .gaps = ".-X",
        .case_sensitive = false,
    };

    int retcode = sequences_init_alphabet(&test_alphabet, test_alphabet.name,
                                          test_alphabet.syms, test_alphabet.gaps,
                                          test_alphabet.case_sensitive);
    if (retcode == 0)
        return 1;

    return 0;
}

int test_init_base_alphabets(void)
{
    return sequences_init_base_alphabets();
}

int test_sym_in_alphabet(void)
{
    Alphabet test_alphabet = {
        .name = "test",
        .syms = "ABC.-",
        .gaps = ".-",
        .case_sensitive = false,
    };
    char in_sym = 'A';
    char out_sym = 'X';
    char non_ascii_sym = '\xE1';

    int retcode = sequences_init_alphabet(&test_alphabet, test_alphabet.name,
                                          test_alphabet.syms, test_alphabet.gaps,
                                          test_alphabet.case_sensitive);
    if (retcode > 0)
        return retcode;

    if (sequences_sym_in_alphabet(&test_alphabet, in_sym) != 1)
        return 1;
    if (sequences_sym_in_alphabet(&test_alphabet, out_sym) != 0)
        return 1;
    if (sequences_sym_in_alphabet(&test_alphabet, non_ascii_sym) != -1)
        return 1;

    return 0;
}

int test_sym_is_gap(void)
{
    Alphabet test_alphabet = {
        .name = "test",
        .syms = "ABC.-",
        .gaps = ".-",
        .case_sensitive = false,
    };
    char in_sym = 'A';
    char out_sym = 'X';
    char non_ascii_sym = '\xE1';
    char gap_sym = '-';

    int retcode = sequences_init_alphabet(&test_alphabet, test_alphabet.name,
                                          test_alphabet.syms, test_alphabet.gaps,
                                          test_alphabet.case_sensitive);
    if (retcode > 0)
        return retcode;

    if (sequences_sym_is_gap(&test_alphabet, in_sym) != 0)
        return 1;
    if (sequences_sym_is_gap(&test_alphabet, out_sym) != 0)
        return 1;
    if (sequences_sym_is_gap(&test_alphabet, non_ascii_sym) != -1)
        return 1;
    if (sequences_sym_is_gap(&test_alphabet, gap_sym) != 1)
        return 1;

    return 0;
}

int test_seq_in_alphabet(void)
{
    Alphabet test_alphabet = {
        .name = "test",
        .syms = "ABC.-",
        .gaps = ".-",
        .case_sensitive = false,
    };
    SeqRecord seq_1 = {.seq = "AABBCC", .len = 6};
    SeqRecord seq_2 = {.seq = "AABBXX", .len = 6};

    int retcode = sequences_init_alphabet(&test_alphabet, test_alphabet.name,
                                          test_alphabet.syms, test_alphabet.gaps,
                                          test_alphabet.case_sensitive);
    if (retcode > 0)
        return retcode;

    if (sequences_seq_in_alphabet(&test_alphabet, &seq_1) != 1)
        return 1;
    if (sequences_seq_in_alphabet(&test_alphabet, &seq_2) == 1)
        return 1;

    return 0;
}

TestFunction tests[] = {
    {&test_init_alphabet_base, "test_init_alphabet_base"},
    {&test_init_alphabet_duplicated_sym, "test_init_alphabet_duplicated_sym"},
    {&test_init_alphabet_out_of_alphabet_gap, "test_init_alphabet_out_of_alphabet_gap"},
    {&test_init_base_alphabets, "test_init_base_alphabets"},
    {&test_sym_in_alphabet, "test_sym_in_alphabet"},
    {&test_sym_is_gap, "test_sym_is_gap"},
    {&test_seq_in_alphabet, "test_seq_in_alphabet"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
