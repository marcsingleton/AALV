#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_MAX 128

typedef struct
{
    int map[CHAR_MAX];
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

// Utilities
unsigned int prefix_tree_common_prefix_len(char *s, char *t)
{
    unsigned int i = 0;
    while (s[i] == t[i] && s[i] != '\0' && t[i] != '\0')
        i++;
    return i;
}

unsigned int prefix_tree_node_count_children(PrefixTreeNode *node)
{
    if (!node)
        return 0;

    unsigned int child_count = 0;
    for (unsigned int i = 0; i < node->capacity; i++)
    {
        PrefixTreeNode *child = node->children[i];
        if (child)
            child_count++;
    }
    return child_count;
}

PrefixTreeNode *prefix_tree_node_get_child(PrefixTreeNode *node, unsigned int index)
{
    if (!node)
        return NULL;

    for (unsigned int i = 0; i < node->capacity; i++)
    {
        PrefixTreeNode *child = node->children[i];
        if (child)
        {
            if (index == 0)
                return child;
            index--;
        }
    }

    return NULL;
}

// CharMap
int char_map_init(CharMap *char_map, char *s)
{
    if (!char_map || !s)
        return 1;

    for (unsigned int i = 0; i < CHAR_MAX; i++)
        char_map->map[i] = -1;

    unsigned int len = 0;
    for (char *c = s; *c != '\0'; c++)
    {
        int index = *c;
        if (char_map->map[index] == -1)
        {
            char_map->map[index] = len;
            len++;
        }
    }
    char_map->len = len;

    return 0;
}

CharMap *char_map_create(char *s)
{
    if (!s)
        return NULL;

    CharMap *char_map = malloc(sizeof(CharMap));
    if (!char_map)
        return NULL;

    int retcode = char_map_init(char_map, s);
    if (retcode > 0)
    {
        free(char_map);
        return NULL;
    }

    return char_map;
}

int char_map_is_in(CharMap *char_map, char *s)
{
    if (!char_map || !s)
        return -1;

    for (; *s != '\0'; s++)
    {
        int c = *s;
        if (char_map->map[c] == -1)
            return 0;
    }
    return 1;
}

void char_map_print(CharMap *char_map)
{
    if (!char_map)
        return;

    for (unsigned int i = 0; i < CHAR_MAX; i++)
    {
        if (char_map->map[i] != -1)
        {
            printf("%c -> %d\n", i, char_map->map[i]);
        }
    }
}

// Node
int prefix_tree_node_init(PrefixTreeNode *node, unsigned int capacity, size_t size)
{
    if (!node)
        return 1;

    PrefixTreeNode **children = calloc(capacity, sizeof(PrefixTreeNode *));
    if (!children)
        return 1;
    node->children = children;
    node->capacity = capacity;
    node->size = size;
    node->prefix = NULL;
    node->value = NULL;
    return 0;
}

void prefix_tree_node_deinit(PrefixTreeNode *node)
{
    if (!node)
        return;

    for (unsigned int i = 0; i < node->capacity; i++)
    {
        PrefixTreeNode *child = node->children[i];
        if (child)
        {
            prefix_tree_node_deinit(child);
        }
    }
    free(node->children);
    free(node->prefix);
    free(node->value);
    node->children = NULL;
    node->capacity = 0;
    node->size = 0;
    node->prefix = NULL;
    node->value = NULL;
}

PrefixTreeNode *prefix_tree_node_create(unsigned int capacity, size_t size)
{
    PrefixTreeNode *node = malloc(sizeof(PrefixTreeNode));
    if (!node)
        return NULL;
    int retcode = prefix_tree_node_init(node, capacity, size);
    if (retcode > 0)
    {
        free(node);
        return NULL;
    }

    return node;
}

void prefix_tree_node_destroy(PrefixTreeNode *node)
{
    if (!node)
        return;

    prefix_tree_node_deinit(node);
    free(node);
}

int prefix_tree_node_insert(PrefixTreeNode *node, CharMap *char_map, char *key, void *value)
{
    if (!node || !char_map || !key)
        return 1;

    unsigned int key_index = 0;
    unsigned int keylen = strlen(key);
    while (key_index < keylen)
    {
        char *suffix = key + key_index;
        unsigned int suffixlen = keylen - key_index;
        unsigned int prefixlen = strlen(node->prefix);
        unsigned int commonlen = prefix_tree_common_prefix_len(node->prefix, suffix);

        if (commonlen == suffixlen && commonlen == prefixlen) // Exact prefix/suffix match; write value to node
        {
            void *new_value = malloc(node->size);
            if (!new_value)
                return 1;
            memcpy(new_value, value, node->size);
            free(node->value);
            node->value = new_value;
            return 0;
        }
        else if (commonlen == prefixlen) // Consumes entire prefix with remaining suffix
        {
            key_index += commonlen;
            int c = key[key_index];
            unsigned int child_index = char_map->map[c];
            PrefixTreeNode *child = node->children[child_index];
            if (child) // Go to child if it exists
                node = child;
            else // Otherwise make child
            {
                char *new_child_prefix = strdup(key + key_index);
                void *new_child_value = malloc(node->size);
                PrefixTreeNode *new_child = prefix_tree_node_create(char_map->len, node->size);
                if (!new_child_prefix || !new_child_value || !new_child)
                {
                    free(new_child_prefix);
                    prefix_tree_node_deinit(child);
                    return 1;
                }
                memcpy(new_child_value, value, node->size);
                new_child->prefix = new_child_prefix;
                new_child->value = new_child_value;
                node->children[child_index] = new_child;
                return 0;
            }
            continue;
        }
        else if (commonlen < prefixlen) // Split node
        {
            int c = node->prefix[commonlen];
            unsigned int child_index = char_map->map[c];

            // Allocate all new memory
            char *new_prefix = strndup(node->prefix, commonlen);
            void *new_value = malloc(node->size);
            char *new_child_prefix = strdup(node->prefix + commonlen);
            PrefixTreeNode *new_child = prefix_tree_node_create(char_map->len, node->size);
            if (!new_prefix || !new_child_prefix || !new_value || !new_child)
            {
                free(new_prefix);
                free(new_value);
                free(new_child_prefix);
                prefix_tree_node_deinit(new_child);
                return 1;
            }
            memcpy(new_value, value, node->size);

            // Copy current node to new child
            new_child->prefix = new_child_prefix;
            new_child->value = node->value;
            for (unsigned int i = 0; i < node->capacity; i++)
                new_child->children[i] = node->children[i];

            // Reset current node
            for (unsigned int i = 0; i < node->capacity; i++)
                node->children[i] = NULL;
            free(node->prefix);
            node->value = NULL;

            // Set current node with new values
            node->prefix = new_prefix;
            node->children[child_index] = new_child;
        }
        else
            return 1;
    }

    return 0;
}

void *prefix_tree_node_get(PrefixTreeNode *node, CharMap *char_map, char *key)
{
    if (!node || !char_map || !key)
        return NULL;

    unsigned int key_index = 0;
    unsigned int keylen = strlen(key);
    while (key_index < keylen)
    {
        char *suffix = key + key_index;
        unsigned int suffixlen = keylen - key_index;
        unsigned int prefixlen = strlen(node->prefix);
        unsigned int commonlen = prefix_tree_common_prefix_len(node->prefix, suffix);
        if (commonlen == suffixlen && commonlen == prefixlen) // Exact prefix/suffix match; get value at node
            return node->value;
        else if (commonlen == prefixlen) // Consumes entire prefix with remaining suffix
        {
            key_index += commonlen;
            int c = key[key_index];
            unsigned int child_index = char_map->map[c];
            PrefixTreeNode *child = node->children[child_index];
            if (!child) // Go to child if it exists
                return NULL;
            node = child;
            continue;
        }
        else if (commonlen < prefixlen) // Incomplete prefix match
            return NULL;
        else
            return NULL;
    }

    return NULL;
}

void *prefix_tree_node_get_prefix_match(PrefixTreeNode *node, CharMap *char_map, char *key)
{
    if (!node || !char_map || !key)
        return NULL;

    unsigned int key_index = 0;
    unsigned int keylen = strlen(key);
    while (key_index < keylen)
    {
        char *suffix = key + key_index;
        unsigned int suffixlen = keylen - key_index;
        unsigned int prefixlen = strlen(node->prefix);
        unsigned int commonlen = prefix_tree_common_prefix_len(node->prefix, suffix);
        if (commonlen == suffixlen && commonlen == prefixlen) // Exact prefix/suffix match; get value at node
            return node->value;
        else if (commonlen == prefixlen) // Consumes entire prefix with remaining suffix
        {
            key_index += commonlen;
            int c = key[key_index];
            unsigned int child_index = char_map->map[c];
            PrefixTreeNode *child = node->children[child_index];
            if (!child) // Go to child if it exists
                return NULL;
            node = child;
            continue;
        }
        else if (commonlen < prefixlen) // Incomplete prefix match
        {
            unsigned int child_count = prefix_tree_node_count_children(node);
            if (child_count == 0)
                return node->value;
            else
                return NULL;
        }
        else
            return NULL;
    }

    return NULL;
}

int prefix_tree_node_delete(PrefixTreeNode *node, CharMap *char_map, char *key)
{
    if (!node || !char_map || !key)
        return 1;

    // Find matching node
    unsigned int key_index = 0;
    unsigned int keylen = strlen(key);
    PrefixTreeNode *parent = NULL;
    unsigned int node_index = 0;
    while (key_index < keylen)
    {
        char *suffix = key + key_index;
        unsigned int suffixlen = keylen - key_index;
        unsigned int prefixlen = strlen(node->prefix);
        unsigned int commonlen = prefix_tree_common_prefix_len(node->prefix, suffix);
        if (commonlen == suffixlen && commonlen == prefixlen) // Exact prefix/suffix match; get value at node
            break;
        else if (commonlen == prefixlen) // Consumes entire prefix with remaining suffix
        {
            key_index += commonlen;
            int c = key[key_index];
            unsigned int child_index = char_map->map[c];
            PrefixTreeNode *child = node->children[child_index];
            if (!child) // Go to child if it exists
                return 1;
            parent = node;
            node = child;
            node_index = child_index;
            continue;
        }
        else if (commonlen < prefixlen) // Incomplete prefix match
            return 1;
        else
            return 1;
    }

    // Unset value
    node->value = NULL;

    // Clean-up tree
    int child_count = prefix_tree_node_count_children(node);
    if (child_count == 1) // Node has 1 child - merge with it
    {
        PrefixTreeNode *child = prefix_tree_node_get_child(node, 0);

        // Merge node's prefix with child's prefix
        unsigned int new_prefix_len = strlen(node->prefix) + strlen(child->prefix);
        char *new_prefix = malloc(new_prefix_len + 1);
        if (!new_prefix)
            return 1;

        strcpy(new_prefix, node->prefix);
        strcat(new_prefix, child->prefix);

        free(node->prefix);
        node->prefix = new_prefix;
        node->value = child->value;

        // Adopt child's children
        for (unsigned int i = 0; i < node->capacity; i++)
            node->children[i] = child->children[i];

        // Free child without destroying its children
        free(child->prefix);
        free(child->children);
        free(child);
    }
    else if (child_count == 0 && parent) // Node has no children - delete it
    {
        prefix_tree_node_destroy(node);
        parent->children[node_index] = NULL;
        if (parent->value || prefix_tree_node_count_children(parent) > 1)
            return 0;

        // Merge parent's prefix with its child's prefix
        PrefixTreeNode *child = prefix_tree_node_get_child(parent, 0);

        unsigned int new_prefix_len = strlen(parent->prefix) + strlen(child->prefix);
        char *new_prefix = malloc(new_prefix_len + 1);
        if (!new_prefix)
            return 1;
        strcpy(new_prefix, parent->prefix);
        strcat(new_prefix, child->prefix);

        free(parent->prefix);
        parent->prefix = new_prefix;
        parent->value = child->value;

        // Adopt child's children
        for (unsigned int i = 0; i < parent->capacity; i++)
            parent->children[i] = child->children[i];

        // Free child without destroying its children
        free(child->prefix);
        free(child->children);
        free(child);
    }

    return 0;
}

void prefix_tree_node_print(PrefixTreeNode *node, void (*print_value_fn)(PrefixTreeNode *), unsigned int depth)
{
    if (!node)
        return;

    unsigned int child_count = prefix_tree_node_count_children(node);
    for (unsigned int i = 0; i < depth; i++)
        fputs("  ", stdout);
    printf("<prefix=%s, value=", node->prefix);
    print_value_fn(node);
    printf(", child_count=%d>\n", child_count);
    for (unsigned int i = 0; i < node->capacity; i++)
    {
        PrefixTreeNode *child = node->children[i];
        if (child)
            prefix_tree_node_print(child, print_value_fn, depth + 1);
    }
}

// Tree
int prefix_tree_init(PrefixTree *tree, char *s, size_t size)
{
    if (!tree)
        return 1;

    int retcode = char_map_init(&tree->char_map, s);
    if (retcode > 0)
        return 1;
    retcode = prefix_tree_node_init(&tree->root, tree->char_map.len, size);
    if (retcode > 0)
        return 1;

    return 0;
}

void prefix_tree_deinit(PrefixTree *tree)
{
    if (!tree)
        return;

    prefix_tree_node_deinit(&tree->root);
}

PrefixTree *prefix_tree_create(char *s, size_t size)
{
    PrefixTree *tree = malloc(sizeof(PrefixTree));
    if (!tree)
        return NULL;

    int retcode = prefix_tree_init(tree, s, size);
    if (retcode > 0)
    {
        free(tree);
        return NULL;
    }

    return tree;
}

void prefix_tree_destroy(PrefixTree *tree)
{
    if (!tree)
        return;

    prefix_tree_deinit(tree);
    free(tree);
}

int prefix_tree_insert(PrefixTree *tree, char *key, void *value)
{
    if (!tree)
        return 1;
    if (!char_map_is_in(&tree->char_map, key))
        return 1;

    PrefixTreeNode *node = &tree->root;
    if (!node->prefix)
    {
        char *new_prefix = strdup(key);
        void *new_value = malloc(node->size);
        if (!new_prefix || !new_value)
        {
            free(new_prefix);
            free(new_value);
        }
        memcpy(new_value, value, node->size);
        node->prefix = new_prefix;
        node->value = new_value;
        return 0;
    }
    CharMap *char_map = &tree->char_map;
    int retcode = prefix_tree_node_insert(node, char_map, key, value);

    return retcode;
}

void *prefix_tree_get(PrefixTree *tree, char *key)
{
    if (!tree)
        return NULL;
    if (!char_map_is_in(&tree->char_map, key))
        return NULL;

    PrefixTreeNode *node = &tree->root;
    if (!node->prefix)
        return NULL;
    CharMap *char_map = &tree->char_map;
    void *value = prefix_tree_node_get(node, char_map, key);

    return value;
}

void *prefix_tree_get_prefix_match(PrefixTree *tree, char *key)
{
    if (!tree)
        return NULL;
    if (!char_map_is_in(&tree->char_map, key))
        return NULL;

    PrefixTreeNode *node = &tree->root;
    if (!node->prefix)
        return NULL;
    CharMap *char_map = &tree->char_map;
    void *value = prefix_tree_node_get_prefix_match(node, char_map, key);

    return value;
}

int prefix_tree_delete(PrefixTree *tree, char *key)
{
    if (!tree)
        return 1;
    if (!char_map_is_in(&tree->char_map, key))
        return 1;

    PrefixTreeNode *node = &tree->root;
    if (!node->prefix)
        return 1;
    CharMap *char_map = &tree->char_map;
    int retcode = prefix_tree_node_delete(node, char_map, key);

    return retcode;
}

void prefix_tree_print(PrefixTree *tree, void (*print_value_fn)(PrefixTreeNode *))
{
    prefix_tree_node_print(&tree->root, print_value_fn, 0);
}
