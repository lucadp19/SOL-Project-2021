#include "server.h"

int install_workers(pthread_t worker_tids[], int** worker_pipes){
    if(worker_tids == NULL || worker_pipes == NULL){
        errno = EINVAL;
        perror("Null arguments to install_workers");
        return -1;
    }

    int err;

    for(int i = 0; i < server_config.n_workers; i++){
        if( (err = pthread_create(&(worker_tids[i]), NULL, worker_thread, (void*)(worker_pipes[i]))) != 0){
            errno = err;
            return -1;
        }
    }
    return 0;
}

void* worker_thread(void* arg){
    if(arg == NULL){
        // returns 22l -> code for EINVAL
        return (void*)22l;
    }

    #ifdef DEBUG
        printf("Hello I'm a worker thread!\n");
        fflush(stdout);
    #endif

    int* pipe = (int*) arg;

    sleep(5);

    #ifdef DEBUG
        printf("Closing thread!\n");
    #endif

    close(pipe[W_ENDP]);
    pipe[W_ENDP] = -1;
    return NULL;
}