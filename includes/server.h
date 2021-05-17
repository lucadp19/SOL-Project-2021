#ifndef _SERVER_H
#define _SERVER_H

#include "util.h"
#include "node.h"
#include "list.h"
 
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#define MAX_SOCKET_PATH 120

typedef struct {
    unsigned int n_workers;
    size_t max_space;
    unsigned int max_files;
    char socket_path[MAX_SOCKET_PATH];
} server_config_t;

typedef struct {
    int* pipe;
} worker_arg_t;

// ------ GLOBAL VARIABLES ------ //
extern server_config_t server_config;

// --------- FUNCTIONS --------- //
int get_server_config(const char* path_to_config);

int install_sig_handler(int* pipe, pthread_t* sig_handler_tid);
void* sig_handler_thread(void* arg);

int install_workers(pthread_t* worker_ids, int** worker_pipes);
void* worker_thread(void* arg);

#endif