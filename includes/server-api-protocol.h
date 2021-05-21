#ifndef _SERVER_API_PROTOCOL_H
#define _SERVER_API_PROTOCOL_H

typedef enum {
    OPEN_FILE,
    CLOSE_FILE,
    READ_FILE,
    WRITE_FILE,
    APPEND_TO_FILE,
    READ_N_FILES,
    LOCK_FILE,
    UNLOCK_FILE,
    REMOVE_FILE
} op_code_t;

#endif