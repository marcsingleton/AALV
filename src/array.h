#ifndef ARRAY_H
#define ARRAY_H

/*
 * Dynamic arrays
 */

#include <stddef.h>

typedef struct
{
    void *data;
    size_t size;
    size_t capacity;
    size_t max_capacity;
    size_t len;
} Array;

int array_init(Array *array, size_t size);
void array_deinit(Array *array);
int array_append(Array *array, const void *value);
int array_extend(Array *array, const void *values, size_t len);
void *array_get(Array *array, size_t index);
void *array_pop(Array *array);
int array_shrink(Array *array);

#endif // ARRAY_H
