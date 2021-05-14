#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

/**
 * Takes a string and converts it into a long.
 * On success returns 0 and num references the correct result,
 * whereas
 *      - if str == NULL or str has length 0 or num_ptr == NULL
 *          it returns -1 and sets errno;
 *      - if the result overflows it sets errno and returns -2;
 *      - if str is not a number it returns -3 and sets errno.
 */
int str_to_long(const char* str, long* num_ptr);

#endif