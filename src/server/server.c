#include "server.h"

server_config_t server_config;

static int yield_all_threads(pthread_t sig_handler_tid);

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

    // --- SIGNAL HANDLING --- //
    pthread_t sig_handler_tid;
    int* sig_handler_pipe;
    if( (sig_handler_pipe = malloc(2 * sizeof(int))) == NULL){
        perror("Error in malloc for sig_handler_pipe");
        return -1;
    }
    if( pipe(sig_handler_pipe) == -1){
        perror("Error in creating sig_handler_pipe");
        return -1;
    }

    if( install_sig_handler(sig_handler_pipe, &sig_handler_tid) == -1){
        perror("Error in installing signal handler");
        return -1;
    }

    // other things
    
    if( yield_all_threads(sig_handler_tid) == -1){
        perror("Error in thread yield");
        return -1;
    }

    return 0;
}

int yield_all_threads(pthread_t sig_handler_tid){
    int err;
    if( (err = pthread_join(sig_handler_tid, NULL)) != 0){
        errno = err;
        return -1;
    } 

    return 0;
}