#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#include <unistd.h>

/**
 * Reading endpoint for a pipe.
 */
#define R_ENDP 0
/**
 * Writing endpoint for a pipe.
 */
#define W_ENDP 1

/**
 * Takes an int vector with two positions and sets them both to -1.
 */
void pipe_init(int pipe[]);

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

/**
 * Like the system call read, but avoids partial reads.
 * Returns:
 *      - the number of bytes read (> 0) on success;
 *      - 0 when EOF is reached;
 *      - -1 on error (and sets errno).
 */
int readn(long fd, void *buf, size_t size);


/**
 * Like the system call write, but avoids partial writings.
 * Returns:
 *      - 1 on success;
 *      - 0 when write returns 0;
 *      - -1 on error (and sets errno).
 */
int writen(long fd, void *buf, size_t size);



#endif