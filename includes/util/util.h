#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

// --------------- GENERAL --------------- //

#define PATH_MAX 128

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

// ------------- MUTEX & COND ------------- //
/**
 * Locks the given mutex,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_mutex_lock(pthread_mutex_t* mtx);

/**
 * Unlocks the given mutex,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_mutex_unlock(pthread_mutex_t* mtx);

/**
 * Puts the thread in waiting on the given cv and mutex,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mtx);

/**
 * Wakes a single thread sleeping on the given cv,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_mutex_signal(pthread_cond_t* cond);

/**
 * Wakes a single thread sleeping on the given cv,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_cond_broadcast(pthread_cond_t* cond);

// ---------------- PIPES ---------------- //

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