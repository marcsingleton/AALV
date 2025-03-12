#ifndef UTILS_H
#define UTILS_H

/*
 * Test runners
 */

#include <stdio.h>

typedef struct
{
    int (*func_ptr)(void);
    char *func_name;
} TestFunction;

void run_tests(TestFunction *tests, int ntests, char *module_name)
{
    int npasses = 0;
    for (int i = 0; i < ntests; i++)
    {
        TestFunction test = tests[i];
        printf("Running %s...", test.func_name);
        int code = (*test.func_ptr)();
        if (code == 0)
        {
            printf("passed!\n");
            npasses++;
        }
        else
            printf("failed with code %d.\n", code);
    }
    printf("%d / %d tests passed in module %s\n", npasses, ntests, module_name);
}

#endif // UTILS_H
