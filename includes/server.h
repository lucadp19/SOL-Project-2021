#ifndef _SERVER_H
#define _SERVER_H

#include "util/util.h"
#include "util/node.h"
#include "util/list.h"
#include "util/hash.h"

#include "server-api-protocol.h"

typedef struct {
    unsigned int n_workers;
    size_t max_space;
    unsigned int max_files;
    char socket_path[PATH_MAX];
    char log_dir_path[PATH_MAX];
} server_config_t;

typedef struct {
    unsigned int files;
    size_t space;
} server_state_t;

typedef enum {
    MW_SUCCESS,
    MW_CLOSE,
    MW_FATAL_ERROR,
    MW_NON_FATAL_ERROR
} worker_code_t;

typedef struct {
    worker_code_t code;
    long fd_client;
} worker_res_t;

typedef enum {
    ACCEPT_CONN,
    REFUSE_CONN,
    CLOSE_SERVER
} server_mode_t;

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
    /** The file descriptor of the client who currently has this file open. */
    long open;
    /** File descriptor of the client who has locked this file.
     * -1 if no client is currently locking this file.
     */
    long fd_lock;
    /** Hashtable containing all the file descriptors of clients 
     * who have opened this file.
     */
    hashtbl_t* fd_open;
    /** Mutex to modify this file. */
    pthread_mutex_t file_mtx;
    /** Time of last use to implement LRU. */
    time_t last_use;
} file_t;

// ------ GLOBAL VARIABLES ------ //
extern server_config_t server_config;
extern server_mode_t mode;
extern server_state_t curr_state;

extern FILE* log_file;

extern list_t* request_queue;
extern pthread_mutex_t request_queue_mtx;
extern pthread_cond_t request_queue_nonempty;

extern hashmap_t* files;
extern pthread_mutex_t files_mtx;

// --------- FUNCTIONS --------- //
int get_server_config(const char* path_to_config);

int install_sig_handler(int* pipe, pthread_t* sig_handler_tid);
void* sig_handler_thread(void* arg);

int install_workers(pthread_t* worker_ids, int** worker_pipes);
void* worker_thread(void* arg);

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
int open_file(long fd_client);
/**
 * Deals with a closeFile request from the API.
 * Can return:
 *      SA_SUCCESS  in case of success (also when the client didn't open the given file)
 *      SA_ERROR    if there is an unspecified error
 *      SA_CLOSE    if the client closed its connection
 *      SA_NO_FILE  if the client is trying to close a non-existing file
 */
int close_file(long fd_client);

int add_file_to_fs(file_t* file);
void file_delete(file_t* file);

int expell_LRU(file_t** file_ptr);
int expell_multiple_LRU(size_t size_to_free, list_t* expelled_list);

#endif