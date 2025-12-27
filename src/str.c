#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "str.h"

#include <stdio.h>

static int copy_field(char **field_ptr, const char *s, const size_t len)
{
    char *field = malloc((len + 1) * sizeof(char));
    if (!field)
        return 1;
    memcpy(field, s, len * sizeof(char));
    field[len] = '\0';
    *field_ptr = field;
    return 0;
}

size_t str_count(const char *s, const char c)
{
    if (!s)
        return 0;

    size_t n = 0;
    for (; *s != '\0'; s++)
    {
        if (*s == c)
            n++;
        if (n == SIZE_MAX)
            break;
    }
    return n;
}

int str_is_in_split(const char **ss, unsigned int n, const char *t)
{
    for (unsigned int i = 0; i < n; i++)
        if (strcmp(ss[i], t) == 0)
            return 1;
    return 0;
}

int str_is_in_strsep(const char *s, const char *delim, const char *t)
{
    char *s0, *s1;
    s0 = strdup(s);
    if (!s0)
        return -1;
    s1 = s0;

    int retcode = 0;
    char *token;
    while ((token = strsep(&s1, delim)))
        if (strcmp(token, t) == 0)
        {
            retcode = 1;
            break;
        }

    free(s0);

    return retcode;
}

ssize_t str_split(char ***fields_ptr, const char *s, const char d)
{
    if (!s)
        return -1;

    size_t m = str_count(s, d) + 1;
    if (m > SIZE_MAX / sizeof(char *))
        return -1;
    ssize_t n = m;
    char **fields = malloc(n * sizeof(char *));
    if (!fields)
        return -1;
    *fields_ptr = fields;

    size_t i, j, k;
    i = j = k = 0;
    for (; s[j] != '\0'; j++)
        if (s[j] == d)
        {
            if (copy_field(fields + k, s + i, j - i) != 0)
                goto error;
            i = j + 1;
            k++;
        }
    if (copy_field(fields + k, s + i, j - i) != 0)
        goto error;

    return n;

error:
    for (unsigned int i = 0; i < k; i++)
        free(fields[i]);
    free(fields);

    return -1;
}

void str_free_split(char **fields, const unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
        free(fields[i]);
    free(fields);
}
