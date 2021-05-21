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
    char current_opt[10];
    for(int i = 0; i < 4; i++){
        FSCANF(config, " %9c = ", current_opt, err);

        // max_files
        if(strncmp(current_opt, "max_files", 9) == 0){
            FSCANF(config, "%u", &server_config.max_files, err);
            debug("option max_files = %u\n", server_config.max_files);
            continue;
        }
        
        // max_space
        if(strncmp(current_opt, "max_space", 9) == 0){
            FSCANF(config, "%lu", &server_config.max_space, err);
            debug("option max_space = %lu\n", server_config.max_space);
            continue;
        }

        // no_worker
        if(strncmp(current_opt, "no_worker", 9) == 0){
            FSCANF(config, "%u", &server_config.n_workers, err);
            debug("option no_worker = %u\n", server_config.n_workers);
            continue;
        }

        // sock_path
        if(strncmp(current_opt, "sock_path", 9) == 0){
            FSCANF(config, "%s", server_config.socket_path, err);
            debug("option sock_path = %s\n", server_config.socket_path);
            continue;
        }

        fprintf(stderr, "Unknown option in config file. Aborting.\n");
        return -1;
    }

    fclose(config);

    return 0;
}