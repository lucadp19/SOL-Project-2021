#include "server.h"

int install_workers(pthread_t worker_tids[], int** worker_pipes){
    if(worker_tids == NULL || worker_pipes == NULL){
        errno = EINVAL;
        perror("Null arguments to install_workers");
        return -1;
    }

    int err;

    for(int i = 0; i < server_config.n_workers; i++){
        worker_arg_t arg;
        arg.pipe = worker_pipes[i];
        #ifdef DEBUG
        printf("After worker_pipes[%d]", i);
        #endif
        if( (err = pthread_create(&(worker_tids[i]), NULL, worker_thread, (void*)&arg)) != 0){
            errno = err;
            return -1;
        }
        

        #ifdef DEBUG
            printf("Created worker %d\n", i);
            fflush(stdout);
        #endif        
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

    worker_arg_t* w_arg = (worker_arg_t*)arg;

    sleep(5);

    #ifdef DEBUG
        printf("Closing thread!\n");
    #endif

    close(w_arg->pipe[W_ENDP]);
    w_arg->pipe[W_ENDP] = -1;
    return NULL;
}