#ifndef _SERVER_H
#define _SERVER_H

#include "util/util.h"
#include "util/node.h"
#include "util/list.h"
#include "util/hash.h"

#include "server-api-protocol.h"

/** A server configuration. */
typedef struct {
    unsigned int n_workers;
    size_t max_space;
    unsigned int max_files;
    char socket_path[PATH_MAX];
    char log_dir_path[PATH_MAX];
} server_config_t;

/** 
 * Represents the state of the server, 
 * which means the number of files it currently has in store,
 * the max number of files it has concurrently stored since its start,
 * the size (in bytes) of the occupied space and the max size that has been
 * occupied since the start.
 */
typedef struct {
    /* no. of files */
    unsigned int files;
    /* occupied space */
    size_t space;
    /* number of connections */
    unsigned int conn;
    /* maximum number of files */
    unsigned int max_files;
    /* maximum space occupied */
    size_t max_space;
    /* maximum number of connections */
    unsigned int max_conn;
} server_state_t;

/** The code for the result of a worker elaboration of a client request. */
typedef enum {
    MW_SUCCESS,
    MW_CLOSE,
    MW_FATAL_ERROR,
    MW_NON_FATAL_ERROR
} worker_code_t;

/** Argument to pass to a worker thread. */
typedef struct {
    int worker_no;
    int* pipe;
} worker_arg_t;

/** 
 * The result of a worker elaboration of a client request,
 * together with the client file descriptor.
 */
typedef struct {
    worker_code_t code;
    long fd_client;
} worker_res_t;

/** 
 * The current server mode. Can be
 *  - ACCEPT_CONN: server accepts new connections and requests,
 *  - REFUSE_CONN: server only accepts new requests from already connected clients,
 *  - CLOSE_SERVER: server is about to close and won't accept any new request.
 */
typedef enum {
    /** Server accepts both new connections and requests. */
    ACCEPT_CONN,
    /** Server only accepts new requests from already connected clients. */
    REFUSE_CONN,
    /** Server is about to close and won't accept any new request. */
    CLOSE_SERVER
} server_mode_t;

// ------------- FILES ------------- //

/**
 * A file in the server filesystem.
 */
typedef struct {
    /** Path to file. */
    char* path_name;
    /** Contents of the file. */
    void* contents;
    /** Size of the contents of the file. */
    size_t size;
    /** File descriptor of the client who has locked this file.
     * -1 if no client is currently locking this file.
     */
    long fd_lock;
    /** Hashtable containing all the file descriptors of clients 
     * who have opened this file.
     */
    hashtbl_t* fd_open;
    /** Mutex to regulate access to this file. */
    pthread_mutex_t order_mtx;
    /** Mutex to modify this file. */
    pthread_mutex_t file_mtx;
    /** Conditional variable to gain access to this file. */
    pthread_cond_t access_cond;
    /** Number of readers who are now using this file. */
    unsigned int n_readers;
    /** Number of writers who are now using this file. */
    unsigned int n_writers;
    /** Time of last use to implement LRU. */
    time_t last_use;
    /** Bool to decide if file currently can or cannot be expelled
     * by the LRU algorithm.
     */ 
    bool can_be_expelled;
} file_t;

/** Locks a file to allow reading operations. */
void file_reader_lock(file_t* file);
/** Unlocks a file previously locked in reading mode. */
void file_reader_unlock(file_t* file);
/** Locks a file to allow reading and writing operations. */
void file_writer_lock(file_t* file);
/** Unlocks a file previously locked in writing mode. */
void file_writer_unlock(file_t* file);

// ------ GLOBAL VARIABLES ------ //
/** Server config as read from the config file. */
extern server_config_t server_config;
/** Current server mode. */
extern server_mode_t mode;

/** Current server state. */
extern server_state_t curr_state;
/** Mutex to access the current server state. */
extern pthread_mutex_t curr_state_mtx;

/** Log file. */
extern FILE* log_file;
/** Mutex to write into the log file. */
extern pthread_mutex_t log_file_mtx;

/** The queue of requests by clients. */
extern list_t* request_queue;
/** Mutex to access the request queue. */
extern pthread_mutex_t request_queue_mtx;
/** Conditional variable to regulate access to request queue. */
extern pthread_cond_t request_queue_nonempty;

/** Hashmap containing all files currently stored in the server. */
extern hashmap_t* files;
/** Mutex to access the files hashmap. */
extern pthread_mutex_t files_mtx;

// --------- FUNCTIONS --------- //
/**
 * Given a path to a config file tries 
 * to read the server configuration from the file.
 * Returns 0 on success, otherwise -1.
 * Possible errors are:
 *  - some options are not set;
 *  - some options are set more than once;
 *  - some options are not recognized.
 */
int get_server_config(const char* path_to_config);

/** 
 * Takes a pipe (that is, an array with two positions) and a pointer to a pthread_t variable.
 * Masks SIGINT, SIGTERM and SIGHUP for all threads and sets SIGPIPE to ignore, then
 * creates a new thread which will handle the three aforementioned signals, storing its
 * thread id into sig_handler_tid.
 * On success returns 0, on error -1.
 */
int install_sig_handler(int* pipe, pthread_t* sig_handler_tid);
/**
 * The signal handler thread: handles SIGINT, SIGTERM and SIGHUP.
 */
void* sig_handler_thread(void* arg);

/** 
 * Takes an array of length server_config.no_workers and an array
 * of pipes (that is, arrays with two positions) and creates the
 * worker threads, assigning to each of them a positive integer and
 * a pipe.
 * Returns 0 on success, -1 on error.
 */
int install_workers(pthread_t* worker_ids, int** worker_pipes);
/**
 * A worker thread: executes requests from clients.
 */
void* worker_thread(void* arg);

/** Opens and initializes the log file. */
int init_log_file();
/** 
 * Prints the formatted string fmt into the log file
 * in mutual exclusion with other threads. 
 */
void logger(const char* fmt, ...);

// -------- API REQUEST FUNCTIONS -------- //

/**
 * Deals with an openFile request from the API.
 * Can return:
 *      SA_SUCCESS  in case of success;
 *      SA_ERROR    if there is an unspecified error
 *      SA_CLOSE    if the client closed its connection
 *      SA_EXISTS   if the client is trying to create an already existing file
 *      SA_NO_FILE  if the client is trying to open a non-existing file
 *      SA_ALREADY_LOCKED
 *                  if the client is trying to lock an already locked file
 */
int open_file(int worker_no, long fd_client);
/**
 * Deals with a closeFile request from the API.
 * Can return:
 *      SA_SUCCESS  in case of success (also when the client didn't open the given file)
 *      SA_ERROR    if there is an unspecified error
 *      SA_CLOSE    if the client closed its connection
 *      SA_NO_FILE  if the client is trying to close a non-existing file
 */
int close_file(int worker_no, long fd_client);
/**
 * Deals with a writeFile request from the API.
 * Can return:
 *      SA_SUCCESS      in case of success
 *      SA_ERROR        if there is an unspecified error
 *      SA_CLOSE        if the client closed its connection
 *      SA_NO_FILE      if the client is trying to write into a non-existing file
 *      SA_NO_OPEN      if the client hasn't opened this file
 *      SA_NOT_LOCKED   if the file isn't locked by the client
 *      SA_TOO_BIG      if the file is larger than the server capacity
 *      SA_NOT_EMPTY    if the file has already been written
 */
int write_file(int worker_no, long fd_client);
/**
 * Deals with a readFile request from the API.
 * Can return:
 *      SA_SUCCESS      in case of success
 *      SA_ERROR        if there is an unspecified error
 *      SA_CLOSE        if the client closed its connection
 *      SA_NO_FILE      if the client is trying to write into a non-existing file
 *      SA_NO_OPEN      if the client hasn't opened this file
 */
int read_file(int worker_no, long fd_client);
/**
 * Deals with a readFile request from the API.
 * Can return:
 *      SA_SUCCESS      in case of success
 *      SA_ERROR        if there is an unspecified error
 *      SA_CLOSE        if the client closed its connection
 */
int read_n_files(int worker_no, long fd_client);
/**
 * Deals with a removeFile request from the API.
 * Can return:
 *      SA_SUCCESS      in case of success (also when the file doesn't exist)
 *      SA_ERROR        if there is an unspecified error
 *      SA_CLOSE        if the client closed its connection
 *      SA_NO_OPEN      if the client hasn't opened the file
 *      SA_NOT_LOCKED   if the file isn't locked by the client
 */
int remove_file(int worker_no, long fd_client);

/** 
 * Deals with an appendToFile request from the API.
 * Can return
 *      SA_SUCCESS      in case of success
 *      SA_ERROR        if there is an unspecified error
 *      SA_CLOSE        if the client closed its connection
 *      SA_NO_FILE      if the file doesn't exist
 *      SA_NO_OPEN      if the client hasn't opened the file
 *      SA_TOO_BIG      if the contents of the file plus the new buffer exeed the maximum storage capacity
 */
int append_to_file(int worker_no, long fd_client);

// -------- AUXILIARY API FUNCTIONS -------- //
/**
 * Given a single file, sends the contents of the file to client. 
 * If send_path is true, before sending the contents sends the pathname.
 * If a file has empty contents, no content shall be sent.
 * Returns 0 on success, -1 on error.
 */
int send_single_file(int worker_no, long fd_client, file_t* file, bool send_path);
/**
 * Given a list of files, sends all of them to client.
 * If send_path is true, before sending the contents of a file sends also its pathname.
 * If a file has empty contents, no content shall be sent.
 * Returns 0 on success (every file has been sent), -1 on error.
 */
int send_list_of_files(int worker_no, long fd_client, list_t* files, bool send_path);

/** Given a node containing a file, frees all memory. */
void files_node_cleaner(node_t* node);
/** Given a file, frees all memory. */
void file_delete(file_t* file);

/**
 * Expells a single file from the server using a LRU algorithm.
 * Saves the pointer to the expelled file  
 * while holding both files_mtx and curr_state_mtx lock!
 * 
 * Returns 0 on success, -1 on error. Possible causes of errors are:
 *  - there are no files in the server,
 *  - somehow the files hashmap is NULL.
 */
int expell_LRU(file_t** file_ptr);
/**
 * Expells multiple files from the server until size_to_free bytes have been freed using a LRU algorithm.
 * Saves the pointers to the expelled files in expelled_list.
 * 
 * This function must be called 
 * while holding both files_mtx and curr_state_mtx locks!
 * 
 * Returns 0 on success, -1 on error. Possible causes of errors are:
 *  - size_to_free is greater than the current occupied space;
 *  - there are no files in the server;
 *  - there is an allocation error, but in that case the process wil be aborted.
 */
int expell_multiple_LRU(size_t size_to_free, list_t* expelled_list);

#endif