#ifndef _API_GLOBALS_H
#define _API_GLOBALS_H

typedef struct {
    bool is_open;
    bool create;
    bool lock;
    bool success;
    char* path;
} op_t;

typedef struct {
    long size;
    void* buf;
} size_and_buf_t;

/**
 * File descriptor for the currently connected socket.
 */
extern long fd_sock;
/**
 * Currently connected socket path.
 */
extern const char* socket_path;
/**
 * Last operation - was it an openFile(pathname, O_CREATE | O_LOCK)?
 */
extern op_t last_op;

/**
 * Simple macro to reset connected socket.
 */
#define RESET_SOCK do { \
        fd_sock = -1; \
        socket_path = NULL; \
    } while(0)

/**
 * Simple macro to reset last operation.
 */
#define RESET_LAST_OP do { \
        last_op.is_open = false; \
        last_op.create  = false; \
        last_op.lock    = false; \
        last_op.success = false; \
        if(last_op.path != NULL) { \
            free(last_op.path); \
            last_op.path = NULL; \
        }                        \
    } while(0)

int write_files_sent_by_server(const char* dirname);
int convert_res_to_errno(int res);

#endif