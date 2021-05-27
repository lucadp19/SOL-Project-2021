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

    // variables to decide if an option has already been read
    bool log_file = false;
    bool max_files = false;
    bool max_space = false;
    bool socket_name = false;
    bool no_workers = false;


    if( (config = fopen(path_to_config, "r")) == NULL){
        return -1;
    }

    int err;
    char current_opt[10];
    for(int i = 0; i < 5; i++){
        FSCANF(config, " %9c = ", current_opt, err);

        // max_files
        if(strncmp(current_opt, "max_files", 9) == 0 && !max_files){
            max_files = true;
            FSCANF(config, "%u", &server_config.max_files, err);
            debug("option max_files = %u\n", server_config.max_files);
            continue;
        }
        
        // max_space
        if(strncmp(current_opt, "max_space", 9) == 0 && !max_space){
            max_space = true;
            FSCANF(config, "%lu", &server_config.max_space, err);
            debug("option max_space = %lu\n", server_config.max_space);
            continue;
        }

        // no_worker
        if(strncmp(current_opt, "no_worker", 9) == 0 && !no_workers){
            no_workers = true;
            FSCANF(config, "%u", &server_config.n_workers, err);
            debug("option no_worker = %u\n", server_config.n_workers);
            continue;
        }

        // sock_path
        if(strncmp(current_opt, "sock_path", 9) == 0 && !socket_name){
            socket_name = true;
            FSCANF(config, "%s", server_config.socket_path, err);
            debug("option sock_path = %s\n", server_config.socket_path);
            continue;
        }

        // path_dlog
        if(strncmp(current_opt, "path_dlog", 9) == 0 && !log_file){
            log_file = true;
            FSCANF(config, "%s", server_config.log_dir_path, err);
            debug("option path_dlog = %s\n", server_config.log_dir_path);
            continue;
        }

        fprintf(stderr, "Unknown or repeated option in config file. Aborting.\n");
        return -1;
    }

    fclose(config);

    return 0;
}