/* string.h - String utility functions */
#ifndef STRING_H
#define STRING_H

#include "types.h"

size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *dest, const char *src);
int string_equal(const char *s1, const char *s2);
int string_starts_with(const char *str, const char *prefix);

#endif