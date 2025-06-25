#ifndef STR_H
#define STR_H

/*
 * String functions
 */

unsigned int str_count(const char *s, const char c);
int str_split(char ***fields_ptr, const char *s, const char d);
void str_free_split(char **fields, const unsigned int n);

#endif // STR_H
