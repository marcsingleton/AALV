#ifndef UTILS_H
#define UTILS_H

/*
 * Test runners
 */

#include <stdio.h>

typedef struct
{
    int (*fn_ptr)(void);
    char *name;
} Test;

void run_tests(Test *tests, int ntests, char *module_name)
{
    int npasses = 0;
    for (int i = 0; i < ntests; i++)
    {
        Test *test = tests + i;
        printf("Running %s...", test->name);
        int retcode = (*test->fn_ptr)();
        if (retcode == 0)
        {
            printf("passed!\n");
            npasses++;
        }
        else
            printf("failed with code %d.\n", retcode);
    }
    printf("%d / %d tests passed in module %s\n", npasses, ntests, module_name);
}

#endif // UTILS_H
