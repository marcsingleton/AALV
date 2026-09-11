#ifndef UTILS_H
#define UTILS_H

/*
 * Test runners
 */

#include <stdarg.h>
#include <stdio.h>

#define TEST_SUCCESS 0
#define TEST_FAILURE -1
#define TEST_RESULT_PREFIX "    "

typedef struct
{
    int (*fn_ptr)(void);
    char *name;
} Test;

void test_result_printf(const char *format, ...)
{
    va_list ap;
    fputs(TEST_RESULT_PREFIX, stdout);
    va_start(ap, format);
    printf(format, ap);
    va_end(ap);
}

void run_tests(Test *tests, int ntests, char *module_name)
{
    int npasses = 0;
    for (int i = 0; i < ntests; i++)
    {
        Test *test = tests + i;
        printf("Running %s...\n", test->name);
        int retcode = (*test->fn_ptr)();
        if (retcode == TEST_SUCCESS)
        {
            test_result_printf("\x1b[32mPassed!\x1b[39m\n");
            npasses++;
        }
        else
            test_result_printf("\x1b[31mFailed.\x1b[39m\n");
    }
    printf("%d / %d tests passed in module %s\n", npasses, ntests, module_name);
}

#endif // UTILS_H
