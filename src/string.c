/* string.c - String utility implementations */
#include "string.h"

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

char *strcpy(char *dest, const char *src)
{
    char *original_dest = dest;
    while ((*dest++ = *src++))
        ;
    return original_dest;
}

/* Check if two strings are equal */
int string_equal(const char *s1, const char *s2)
{
    return strcmp(s1, s2) == 0;
}

/* Check if string starts with prefix */
int string_starts_with(const char *str, const char *prefix)
{
    while (*prefix)
    {
        if (*str != *prefix)
        {
            return 0;
        }
        str++;
        prefix++;
    }
    return 1;
}