#ifndef _SERVER_API_PROTOCOL_H
#define _SERVER_API_PROTOCOL_H

/** Type of operation requested by API */
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
/** Operation successful */
#define SA_SUCCESS  0
/** Fatal error in operation */
#define SA_ERROR    -1
/** Client closed connection */
#define SA_CLOSE    -2

// --- Opening/Creation --- //
/** File already exists */
#define SA_EXISTS         -10
/** File doesn't exist */
#define SA_NO_FILE        -11
/** File is already locked */
#define SA_ALREADY_LOCKED -12

// --------- Use --------- //
/** File is not locked by client */
#define SA_NOT_LOCKED -20
/** File is not empty */
#define SA_NOT_EMPTY  -21
/** File is too big */
#define SA_TOO_BIG    -22
/** Client did not open file */
#define SA_NO_OPEN    -23

#endif