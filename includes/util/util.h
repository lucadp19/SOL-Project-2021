#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>

#include <errno.h>
#include <ctype.h>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

// --------------- GENERAL --------------- //

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
 * Auxiliary function for debugging.
 */
int _debug(const char* format, ...);

#ifdef DEBUG
/**
 * Prints its output on stderr if and only if DEBUG is defined.
 */
#define debug(...) \
    _debug(__VA_ARGS__)
#else
/**
 * Prints its output on stderr if and only if DEBUG is defined.
 */
#define debug(...)
#endif

// -------------- FLAGS -------------- //
#define O_NOFLAG 0
#define O_CREATE 1
#define O_LOCK   2

/**
 * Checks is flag (O_CREATE or O_LOCK) is set in val.
 */
#define IS_FLAG_SET(val, flag) \
    (val >> (flag/2)) & 1U // only works because flag = 1 or 2

/**
 * Clears a flag (O_CREATE or O_LOCK).
 */
#define CLEAR_FLAG(val, flag) \
    val &= ~(1U << (flag/2))

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
void safe_pthread_cond_signal(pthread_cond_t* cond);

/**
 * Wakes a single thread sleeping on the given cv,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_cond_broadcast(pthread_cond_t* cond);

// -------------- ALLOCATION -------------- //

/**
 * Tries to allocate size bytes of memory; 
 * on success returns a pointer to the newly allocated memory,
 * on error aborts the process.
 */
void* safe_malloc(size_t size);

/**
 * Tries to allocate an array of nmemb elements of size bytes each; 
 * on success returns a pointer to the newly allocated memory,
 * on error aborts the process.
 */
void* safe_calloc(size_t nmemb, size_t size);

/**
 * Tries to change the size of the block pointed by ptr to size bytes;
 * on success returns a pointer to the newly allocated memory,
 * on error aborts the process.
 */
void* safe_realloc(void* ptr, size_t size);

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

/**
 * Given a non-negative integer representing a time in milliseconds,
 * initializes a struct timespec with the given time.
 */
int set_timespec_from_msec(long msec, struct timespec *req);

#endif