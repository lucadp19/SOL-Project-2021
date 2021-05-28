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

// ---------- CONSTANTS ---------- //

#define SA_SUCCESS 0
#define SA_ERROR  -1
#define SA_CLOSE  -2

// --- Opening/Creation --- //
#define SA_EXISTS         -10
#define SA_NO_FILE        -11
#define SA_ALREADY_LOCKED -12

// --------- Use --------- //
#define SA_NOT_LOCKED -20
#define SA_NOT_EMPTY  -21
#define SA_TOO_BIG    -22



#endif