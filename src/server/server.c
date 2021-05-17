#include "server.h"

server_config_t server_config;

/** 
 * pthread_yield for the given threads.
 */
static int yield_all_threads(pthread_t sig_handler_tid, pthread_t worker_tids[]);
/**
 * Closing all the open pipes.
 */
static void close_all_pipes(int sig_handler_pipe[], int* worker_pipes[]);

int main(int argc, char* argv[]){ 
    if(argc > 2){
        fprintf(stderr, "There must be at most one additional argument: the path to the config file.\n");
        fprintf(stderr, "If no argument is supplied, the default is \"./config/config.txt\".\n");
        return -1;
    }

    char* config_path = (argc == 1) ? "config/config.txt" : argv[1];
     
    if( get_server_config(config_path) == -1){
        perror("Error in reading config file");
        return -1;
    }

    // ------- SIGNAL HANDLING ------- //
    pthread_t sig_handler_tid;
    int sig_handler_pipe[2];
    pipe_init(sig_handler_pipe);

    if( pipe(sig_handler_pipe) == -1){
        perror("Error in creating sig_handler_pipe");
        return -1;
    }

    if( install_sig_handler(sig_handler_pipe, &sig_handler_tid) == -1){
        perror("Error in installing signal handler");
        return -1;
    }

    // ------ WORKER CREATION ------ //
    pthread_t* worker_tids;
    int** worker_pipes;

    // allocating memory for worker thread ids
    if( (worker_tids = (pthread_t*)calloc(server_config.n_workers, sizeof(pthread_t))) == NULL){
        perror("Error in calloc for worker threads ids");
        return -1;
    }
    // setting them to -1
    for(int i = 0; i < server_config.n_workers; i++) worker_tids[i] = -1;

    // allocating memory for worker pipes' vector
    if( (worker_pipes = (int**)calloc(server_config.n_workers, sizeof(int*))) == NULL){
        perror("Error in calloc for worker pipes");
        return -1;
    }
    // allocating memory for worker pipes and creating pipes
    for(int i = 0; i < server_config.n_workers; i++){
        if( (worker_pipes[i] = (int*)calloc(2, sizeof(int*))) == NULL){
            perror("Error in calloc for worker pipes");
            return -1;
        }
        // setting pipes to -1
        pipe_init(worker_pipes[i]);

        if( pipe(worker_pipes[i]) == -1){
            perror("Error in creating pipe");
            return -1;
        }

        #ifdef DEBUG
            printf("worker_pipes[%d][0] = %d, worker_pipes[%d][1] = %d", i, worker_pipes[i][0], i, worker_pipes[i][1]);
        #endif
    }

    if( install_workers(worker_tids, worker_pipes) == -1){
        perror("Error in creating workers");
        return -1;
    }


    // other things
    
    if( yield_all_threads(sig_handler_tid, worker_tids) == -1){
        perror("Error in thread yield");
        return -1;
    }

    close_all_pipes(sig_handler_pipe, worker_pipes);

    return 0;
}

static int yield_all_threads(pthread_t sig_handler_tid, pthread_t worker_tids[]){
    int err;
    if(sig_handler_tid != -1){
        if( (err = pthread_join(sig_handler_tid, NULL)) != 0){
            errno = err;
            return -1;
        } 
        sig_handler_tid = -1;
    }

    for(int i = 0; i < server_config.n_workers; i++){
        if(worker_tids[i] == -1) continue;

        long ret_val;
        if( (err = pthread_join(worker_tids[i], (void**)&ret_val)) == -1){
            errno = err;
            return -1;
        } if(ret_val != 0){
            errno = err;
            return -1;
        }
    }

    return 0;
}

static void close_all_pipes(int sig_handler_pipe[], int* worker_pipes[]){
    if(sig_handler_pipe != NULL) {
        for(int j = 0; j < 2; j++)
            if(sig_handler_pipe[j] != -1)
                close(sig_handler_pipe[j]);
    }
    
    if(worker_pipes != NULL) {
        for(int i = 0; i < server_config.n_workers; i++){
            if(worker_pipes[i] == NULL) continue;

            for(int j = 0; j < 2; j++)
                if(worker_pipes[i][j] != -1)
                    close(worker_pipes[i][j]);
        }
    }
}