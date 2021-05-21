#ifndef _API_GLOBALS_H
#define _API_GLOBALS_H

/**
 * File descriptor for the currently connected socket.
 */
extern long fd_sock;
/**
 * Currently connected socket path.
 */
extern const char* socket_path;

/**
 * Simple macro to reset connected socket.
 */
#define RESET_SOCK do { \
        fd_sock = -1; \
        socket_path = NULL; \
    } while(0)

#endif