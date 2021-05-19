#ifndef _SERVER_API_H
#define _SERVER_API_H

#include "util/util.h"

/**
 * Takes a socket name, a time in milliseconds and an absolute time.
 * Tries to connect to sockname every msec milliseconds and gives up
 * abstime is reached.
 * Returns 0 on success and -1 on error, setting errno.
 * Possible errors are:
 *  - already connected to a socket, errno = [EISCONN];
 *  - sockname is NULL or msec < 0, errno = [EINVAL];
 *  - connection could not be established before abstime, errno = [ETIMEDOUT].
 */
int openConnection(const char* sockname, int msec, const struct timespec abstime);

/** 
 * Takes a socket name and closes the connection.
 * Returns 0 on success and -1 on error, setting errno.
 * Possible errors are:
 *  - wrong socket name, errno = [ENOTCONN].
 */
int closeConnection(const char* sockname);

int openFile(const char* pathname, int flags);

int readFile(const char* pathname, void** buf, size_t* size);

int writeFile(const char* pathname, const char* dirname);

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);

int lockFile(const char* pathname);

int unlockFile(const char* pathname);

int closeFile(const char* pathname);

int removeFile(const char* pathname);

#endif