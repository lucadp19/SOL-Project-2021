#ifndef _SERVER_H
#define _SERVER_H

#include "util.h"
#include "node.h"
#include "list.h"
#include "hash.h"
 
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

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

// ------ GLOBAL VARIABLES ------ //
extern server_config_t server_config;
extern server_mode_t mode;

extern list_t* request_queue;
extern pthread_mutex_t request_queue_mtx;
extern pthread_cond_t request_queue_nonempty;

// --------- FUNCTIONS --------- //
int get_server_config(const char* path_to_config);

int install_sig_handler(int* pipe, pthread_t* sig_handler_tid);
void* sig_handler_thread(void* arg);

int install_workers(pthread_t* worker_ids, int** worker_pipes);
void* worker_thread(void* arg);

#endif