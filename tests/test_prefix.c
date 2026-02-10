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
    if (retcode != 0)
        return 1;

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
    if (retcode != 0)
        return 1;

    key = "A";
    value = 0;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 2;

    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 3;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 4;

    key = "A";
    if (prefix_tree_delete(&tree, key) != 0)
        return 5;
    ptr = prefix_tree_get(&tree, key);
    if (ptr)
        return 6;

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
    if (retcode != 0)
        return 1;

    // Insert
    key = "A";
    value = 0;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 2;
    key = "AB";
    value = 1;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 3;
    key = "ABC";
    value = 2;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 3;

    // Get
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 4;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 5;

    key = "AB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 6;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 7;

    key = "ABC";
    expected_value = 2;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 6;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 7;

    // Delete
    key = "AB";
    if (prefix_tree_delete(&tree, key) != 0)
        return 8;
    ptr = prefix_tree_get(&tree, key);
    if (ptr)
        return 9;

    key = "ABC";
    expected_value = 2;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 10;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 11;

    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 12;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 13;

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
    if (retcode != 0)
        return 1;

    // Insert
    key = "A";
    value = 0;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 2;
    key = "AB";
    value = 1;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 3;

    // Get
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 4;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 5;

    key = "AB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 6;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 7;

    // Delete
    key = "A";
    if (prefix_tree_delete(&tree, key) != 0)
        return 8;
    ptr = prefix_tree_get(&tree, key);
    if (ptr)
        return 9;

    key = "AB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 10;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 11;

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
    if (retcode != 0)
        return 1;

    // Insert
    key = "A";
    value = 0;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 2;
    key = "AB";
    value = 1;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 3;

    // Get
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 4;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 5;

    key = "AB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 6;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 7;

    // Delete
    key = "AB";
    if (prefix_tree_delete(&tree, key) != 0)
        return 8;
    ptr = prefix_tree_get(&tree, key);
    if (ptr)
        return 9;

    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 10;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 11;

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
    if (retcode != 0)
        return 1;

    // Insert
    key = "A";
    value = 0;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 2;

    key = "AAA";
    value = 1;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 3;

    // Get
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    if (!ptr)
        return 4;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 5;

    key = "AA";
    expected_value = 1;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    if (!ptr)
        return 6;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 7;

    key = "AAA";
    expected_value = 1;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    if (!ptr)
        return 8;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 9;

    // Insert prefix
    key = "AA";
    value = 2;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 10;

    // Get prefix
    key = "A";
    expected_value = 0;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    if (!ptr)
        return 11;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 12;

    key = "AAA";
    expected_value = 1;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    if (!ptr)
        return 13;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 14;

    key = "AA";
    expected_value = 2;
    ptr = prefix_tree_get_prefix_match(&tree, key);
    if (!ptr)
        return 15;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 16;

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
    if (retcode != 0)
        return 1;

    // Insert
    key = "AA";
    value = 0;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 2;
    key = "AAB";
    value = 1;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 3;

    // Get
    key = "AA";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 4;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 5;

    key = "AAB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 6;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 7;

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
    if (retcode != 0)
        return 1;

    // Insert
    key = "AAA";
    value = 0;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 2;
    key = "AAB";
    value = 1;
    if (prefix_tree_insert(&tree, key, &value) != 0)
        return 3;

    // Get
    key = "AAA";
    expected_value = 0;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 4;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 5;

    key = "AAB";
    expected_value = 1;
    ptr = prefix_tree_get(&tree, key);
    if (!ptr)
        return 6;
    returned_value = *(int *)ptr;
    if (expected_value != returned_value)
        return 7;

    key = "AA";
    ptr = prefix_tree_get(&tree, key);
    if (ptr)
        return 8;

    if (tree.root.value)
        return 9;

    return 0;
}

int test_tree_insert_out_of_char_map(void)
{
    PrefixTree tree;
    char s[] = "ABC";

    int retcode = prefix_tree_init(&tree, s, sizeof(int));
    if (retcode != 0)
        return 1;

    char *key = "abc";
    int value = 10;
    if (prefix_tree_insert(&tree, key, &value) != 1)
        return 2;

    return 0;
}

TestFunction tests[] = {
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

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
