#ifndef _SERVER_H
#define _SERVER_H

#include "util/util.h"
#include "util/node.h"
#include "util/list.h"
#include "util/hash.h"

typedef struct {
    unsigned int n_workers;
    size_t max_space;
    unsigned int max_files;
    char socket_path[PATH_MAX];
} server_config_t;

typedef struct {
    int code;
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

int open_file(long fd_client);

int add_file_to_fs(file_t* file);
void file_delete(file_t* file);

#endif