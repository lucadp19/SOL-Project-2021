#include "server.h"

#define FSCANF(file, str, arg, err) do {\
        errno = 0; \
        err = fscanf((file), (str), (arg)); \
        if(err == EOF && errno == 0) { \
            fprintf(stderr, "Incomplete config file! Aborting.\n");\
            return -1;\
        } else if(err == -1 && errno != 0) { \
            perror("Error while reading config file:"); \
            fprintf(stderr, "\nAborting.\n"); \
            return -1; \
        } \
    } while(false)


int get_server_config(const char* path_to_config){
    FILE* config;
    if( (config = fopen(path_to_config, "r")) == NULL){
        return -1;
    }

    int err;
    // reading number of workers
    FSCANF(config, "%u\n", &server_config.n_workers, err);
    // reading max server space
    FSCANF(config, "%lu\n", &server_config.max_space, err);
    // reading max number of files
    FSCANF(config, "%u\n", &server_config.max_files, err);
    // reading socket path
    FSCANF(config, "%s", server_config.socket_path, err);

    #ifdef DEBUG
        printf("No. workers: %u.\n", server_config.n_workers);
        printf("Max space: %lu.\n", server_config.max_space);
        printf("Max files: %u.\n", server_config.max_files);
        printf("Socket path: %s.\n", server_config.socket_path);
    #endif

    return 0;
}