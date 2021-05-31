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
 *  - wrong socket name, or not connected to any socket, errno = [ENOTCONN].
 */
int closeConnection(const char* sockname);

/**
 * Given a file name and flags sends a request to open the given file
 * with the given flags.
 * Returns 0 on success and -1 on error, setting errno.
 * Possible errors are:
 *  - client is not connected to server, errno = [ENOTCONN]
 *  - trying to create an already existing file, errno = [EEXIST];
 *  - trying to open (without O_CREAT) a file that doesn't exist, errno = [ENOENT];
 *  - trying to open-lock a file that is already locked, errno = [EBUSY];
 *  - fatal server or API error, errno = [ENOTRECOVERABLE];
 *  - fatal error in communication, errno = [EBADE].
 */
int openFile(const char* pathname, int flags);

/**
 * Given a file name, a pointer to a buffer and a pointer to a size variable
 * sends a request to read the contents of the given file and writes them into
 * the buffer.
 * Returns 0 on success and -1 on error, setting errno.
 * Possible errors are:
 *  - client is not connected to server, errno = [ENOTCONN];
 *  - trying to read a file that doesn't exist, errno = [ENOENT];
 *  - trying to read a non opened file, errno = [ENOKEY];
 *  - fatal server or API error, errno = [ENOTRECOVERABLE];
 *  - fatal error in communication, errno = [EBADE].
 */
int readFile(const char* pathname, void** buf, size_t* size);

/**
 * Takes an integer N and a directory name: if N is greater than 0 
 * reads N files from server and writes them into dirname, whereas if
 * N is 0 reads all files from server and writes them into dirname.
 * If dirname == NULL files read are destroyed.
 * Returns 0 on success and -1 on error, setting errno.
 * Possible errors are:
 *  - client is not connected to server, errno = [ENOTCONN];
 *  - could not create directory dirname, errno = [ENOTEMPTY];
 *  - could not write every file into dirname, errno = [ECANCELED];
 *  - fatal server or API error, errno = [ENOTRECOVERABLE];
 *  - fatal error in communication, errno = [EBADE].
 */
int readNFiles(int N, const char* dirname);

/**
 * Takes a pathname for a file and a directory name and writes 
 * the contents of file pathname into the (already created) file
 * in the server. If the server expells some files as a result of
 * the replacement algorithm, those files are written in dirname.
 * If dirname == NULL the expelled files are destroyed.
 * Returns 0 on success, -1 on error and sets errno.
 * Possible errors are:
 *  - client is not connected to server, errno = [ENOTCONN];
 *  - client last request was not a successful open-lock-create on pathname, errno = [EINVAL];
 *  - pathname is not the path of a file on server, errno = [ENOENT];
 *  - client did not open pathname, errno = [ENOKEY];
 *  - client did not lock pathname, errno = [EPERM];
 *  - the contents of pathname are too big to fit into server, errno = [EFBIG];
 *  - could not open file pathname, errno = [EIO];
 *  - could not create directory dirname, errno = [ENOTEMPTY];
 *  - could not write every file into dirname, errno = [ECANCELED];
 *  - fatal server or API error, errno = [ENOTRECOVERABLE];
 *  - fatal error in communication, errno = [EBADE].
 */
int writeFile(const char* pathname, const char* dirname);

/**
 * Given a file name, a buffer and its size and a directory name,
 * appends the contents of the buffer to the file with the given filename
 * in the server. If the server expells some files as a result of
 * the replacement algorithm, those files are written in dirname.
 * If dirname == NULL the expelled files are destroyed.
 * Returns 0 on success, -1 on error and sets errno.
 * Possible errors are:
 *  - client is not connected to server, errno = [ENOTCONN];
 *  - pathname is not the path of a file on server, errno = [ENOENT];
 *  - client did not open pathname, errno = [ENOKEY];
 *  - the contents of the buffer are too big to fit into server, errno = [EFBIG];
 *  - could not create directory dirname, errno = [ENOTEMPTY];
 *  - could not write every file into dirname, errno = [ECANCELED];
 *  - fatal server or API error, errno = [ENOTRECOVERABLE];
 *  - fatal error in communication, errno = [EBADE].
 */
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);

/** 
 * NOT IMPLEMENTED YET
 * Always returns -1 and sets errno to ENOSYS.
 */
int lockFile(const char* pathname);

/** 
 * NOT IMPLEMENTED YET
 * Always returns -1 and sets errno to ENOSYS.
 */
int unlockFile(const char* pathname);

/**
 * Given a pathname, closes the file.
 * Returns 0 on success (also when the file was not opened by client) 
 * and if pathname was locked, removes the lock.
 * On error returns -1 and sets errno.
 * Possible errors are:
 *  - client is not connected to server, errno = [ENOTCONN];
 *  - pathname is not the path of a file on server, errno = [ENOENT];
 *  - fatal server or API error, errno = [ENOTRECOVERABLE];
 *  - fatal error in communication, errno = [EBADE].
 */
int closeFile(const char* pathname);

/**
 * Given a pathname, removes the given file from the server.
 * This file must be locked by the client before trying to remove it.
 * On success returns 0, on error -1 and sets errno.
 * Possible errors are:
 *  - client is not connected to server, errno = [ENOTCONN];
 *  - pathname is not the path of a file on server, errno = [ENOENT];
 *  - pathname was not opened by client, errno = [ENOKEY];
 *  - pathname was not locked by client, errno = [EPERM];
 *  - fatal server or API error, errno = [ENOTRECOVERABLE];
 *  - fatal error in communication, errno = [EBADE].
 */
int removeFile(const char* pathname);

/**
 * Prints a string to stderr and appends a specific message
 * based on the current errno value.
 * If errno is not one of the errnos set by this API,
 * it prints the usual perror message.
 */ 
void api_perror(const char* msg);

#endif