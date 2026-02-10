#ifndef PREFIX_H
#define PREFIX_H

/*
 * Prefix tree
 */

#include <stdlib.h>

#define PREFIX_CHAR_MAX 128

typedef struct
{
    int map[PREFIX_CHAR_MAX];
    unsigned int len;
} CharMap;

typedef struct PrefixTreeNode
{
    struct PrefixTreeNode **children;
    unsigned int capacity;
    size_t size;
    char *prefix;
    void *value;
} PrefixTreeNode;

typedef struct
{
    PrefixTreeNode root;
    CharMap char_map;
} PrefixTree;

int prefix_tree_init(PrefixTree *tree, char *s, size_t size);
void prefix_tree_deinit(PrefixTree *tree);
PrefixTree *prefix_tree_create(char *s, size_t size);
void prefix_tree_destroy(PrefixTree *tree);
int prefix_tree_insert(PrefixTree *tree, char *key, void *value);
void *prefix_tree_get(PrefixTree *tree, char *key);
void *prefix_tree_get_prefix_match(PrefixTree *tree, char *key);
int prefix_tree_delete(PrefixTree *tree, char *key);
void prefix_tree_print(PrefixTree *tree, void (*print_value_fn)(PrefixTreeNode *));

#endif // PREFIX_H
