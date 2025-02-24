#ifndef UTILS_H
#define UTILS_H

typedef struct
{
    int (*func_ptr)(void);
    char *func_name;
} TestFunction;

#endif // UTILS_H