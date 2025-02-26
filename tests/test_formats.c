
#include "utils.h"

int test_stub()
{
    return 0;
}

TestFunction tests[] = {
    {&test_stub, "test_stub"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main()
{
    run_tests(tests, NTESTS, "test_formats");
}