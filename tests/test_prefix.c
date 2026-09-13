#include "prefix.h"
#include "utils.h"

#define MODULE_NAME "test_prefix"

void print_int_value(PrefixTreeNode *node)
{
    if (node->value)
        printf("%d", *(int *)node->value);
    else
        printf("NULL");
}

void print_int_prefix_tree(PrefixTree *tree)
{
    prefix_tree_print(tree, print_int_value);
}

int test_tree_init(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    TEST_ASSERT(retcode == 0);

    return 0;
}

int test_tree_insert_get_delete_root(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    char *key;
    int value;

    int expected_value;
    int returned_value;
    void *ptr;

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    TEST_ASSERT(retcode == 0);

    key = "A";
    value = 0;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);

    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "A";
    TEST_ASSERT(prefix_tree_delete(&tree, key) == 0);
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr == NULL);

    return 0;
}

int test_tree_insert_get_delete_parent(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    char *key;
    int value;

    int expected_value;
    int returned_value;
    void *ptr;

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    TEST_ASSERT(retcode == 0);

    // Insert
    key = "A";
    value = 0;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);
    key = "AB";
    value = 1;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);
    key = "ABC";
    value = 2;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);

    // Get
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "ABC";
    expected_value = 2;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    // Delete
    key = "AB";
    TEST_ASSERT(prefix_tree_delete(&tree, key) == 0);
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr == NULL);

    key = "ABC";
    expected_value = 2;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    return 0;
}

int test_tree_insert_get_delete_parent_root(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    char *key;
    int value;

    int expected_value;
    int returned_value;
    void *ptr;

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    TEST_ASSERT(retcode == 0);

    // Insert
    key = "A";
    value = 0;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);
    key = "AB";
    value = 1;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);

    // Get
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    // Delete
    key = "A";
    TEST_ASSERT(prefix_tree_delete(&tree, key) == 0);
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr == NULL);

    key = "AB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    return 0;
}

int test_tree_insert_get_delete_leaf(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    char *key;
    int value;

    int expected_value;
    int returned_value;
    void *ptr;

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    TEST_ASSERT(retcode == 0);

    // Insert
    key = "A";
    value = 0;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);
    key = "AB";
    value = 1;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);

    // Get
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    // Delete
    key = "AB";
    TEST_ASSERT(prefix_tree_delete(&tree, key) == 0);
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr == NULL);

    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    return 0;
}

int test_tree_insert_get_prefix(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    char *key;
    int value;

    int expected_value;
    int returned_value;
    void *ptr;

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    TEST_ASSERT(retcode == 0);

    // Insert
    key = "A";
    value = 0;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);

    key = "AAA";
    value = 1;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);

    // Get
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AA";
    expected_value = 1;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AAA";
    expected_value = 1;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    // Insert prefix
    key = "AA";
    value = 2;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);

    // Get prefix
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AAA";
    expected_value = 1;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AA";
    expected_value = 2;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    return 0;
}

int test_tree_insert_split_leaf(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    char *key;
    int value;

    int expected_value;
    int returned_value;
    void *ptr;

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    TEST_ASSERT(retcode == 0);

    // Insert
    key = "AA";
    value = 0;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);
    key = "AAB";
    value = 1;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);

    // Get
    key = "AA";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AAB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    return 0;
}

int test_tree_insert_split_internal(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    char *key;
    int value;

    int expected_value;
    int returned_value;
    void *ptr;

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    TEST_ASSERT(retcode == 0);

    // Insert
    key = "AAA";
    value = 0;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);
    key = "AAB";
    value = 1;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) == 0);

    // Get
    key = "AAA";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AAB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr != NULL);
    returned_value = *(int *)ptr;
    TEST_ASSERT(expected_value == returned_value);

    key = "AA";
    ptr = prefix_tree_get(&tree, key);
    TEST_ASSERT(ptr == NULL);

    TEST_ASSERT(tree.root.value == NULL);

    return 0;
}

int test_tree_insert_out_of_char_map(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    TEST_ASSERT(retcode == 0);

    char *key = "abc";
    int value = 10;
    TEST_ASSERT(prefix_tree_insert(&tree, key, &value) != 0);

    return 0;
}

Test tests[] = {
    {&test_tree_init, "test_tree_init"},
    {&test_tree_insert_get_delete_root, "test_tree_insert_get_delete_root"},
    {&test_tree_insert_get_delete_parent, "test_tree_insert_get_delete_parent"},
    {&test_tree_insert_get_delete_parent_root, "test_tree_insert_get_delete_parent_root"},
    {&test_tree_insert_get_delete_leaf, "test_tree_insert_get_delete_leaf"},
    {&test_tree_insert_get_prefix, "test_tree_insert_get_prefix"},
    {&test_tree_insert_split_leaf, "test_tree_insert_split_leaf"},
    {&test_tree_insert_split_internal, "test_tree_insert_split_internal"},
    {&test_tree_insert_out_of_char_map, "test_tree_insert_out_of_char_map"},
};

#define NTESTS sizeof(tests) / sizeof(Test)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
