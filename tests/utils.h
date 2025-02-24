#ifndef UTILS_H
#define UTILS_H

typedef struct
{
    int (*func_ptr)(void);
    char *func_name;
} TestFunction;

void run_tests(TestFunction *tests, size_t ntests)
{
    for (int i = 0; i < ntests; i++)
    {
        TestFunction test = tests[i];
        printf("Testing function %s\n", test.func_name);
        int code = (*test.func_ptr)();
        if (code == 0)
            printf("    Passed!\n");
        else
            printf("    Failed with code %d\n", code);
    }
}

#endif // UTILS_H