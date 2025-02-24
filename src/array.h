#ifndef ARRAY_H
#define ARRAY_H

/*
 * Dynamic arrays
 */

typedef struct
{
    void *data;
    size_t size;
    size_t capacity;
    size_t len;
} Array;

Array *array_create(size_t size);
int array_append(void *value, Array *array);
void *array_get(size_t index, Array *array);

#endif // ARRAY_H
