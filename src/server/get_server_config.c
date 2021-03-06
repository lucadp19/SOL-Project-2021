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
    bool policy = false;


    if( (config = fopen(path_to_config, "r")) == NULL){
        return -1;
    }

    int err;
    char current_opt[10];
    for(int i = 0; i < 6; i++){
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
            // max_space is in MBytes
            server_config.max_space *= 1000000;
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
            // FSCANF(config, "%s", server_config.socket_path, err);
            if( fgets(server_config.socket_path, PATH_MAX, config) == NULL ){
                fprintf(stderr, "Error in config file! Aborting.\n");
                return -1;
            }
            // removing trailing newline
            server_config.socket_path[strcspn(server_config.socket_path, "\n")] = '\0';
            debug("option sock_path = %s\n", server_config.socket_path);
            continue;
        }

        // path_dlog
        if(strncmp(current_opt, "path_dlog", 9) == 0 && !log_file){
            log_file = true;
            // FSCANF(config, "%s", server_config.log_dir_path, err);
            if( fgets(server_config.log_dir_path, PATH_MAX, config) == NULL ){
                fprintf(stderr, "Error in config file! Aborting.\n");
                return -1;
            }
            // removing trailing newline
            server_config.log_dir_path[strcspn(server_config.log_dir_path, "\n")] = '\0';
            debug("option path_dlog = %s\n", server_config.log_dir_path);
            continue;
        }

        // policy
        if(strncmp(current_opt, "cache_pol", 9) == 0 && !policy){
            policy = true;
            char policy_code[5];
            if( fgets(policy_code, 5, config) == NULL ){
                fprintf(stderr, "Error in config file! Aborting.\n");
                return -1;
            }
            // removing trailing newline
            if( strncmp(policy_code, "LRU", 3) == 0 )
                server_config.policy = LRU;
            else if( strncmp(policy_code, "FIFO", 4) == 0 )
                server_config.policy = FIFO;
            else {
                fprintf(stderr, "Error in config file: unknown cache policy. Aborting.\n");
                return -1;
            }
            debug("option path_dlog = %s\n", server_config.log_dir_path);
            continue;
        }

        fprintf(stderr, "Unknown or repeated option in config file. Aborting.\n");
        return -1;
    }

    if(!policy || !max_files || !max_space || !log_file || !socket_name || !no_workers){
        fprintf(stderr, "Missing option in config. Aborting.\n");
        fprintf(stderr, "policy=%d, max_files=%d, max_space=%d, log_file=%d, socket_name=%d, no_workers=%d\n",
            policy, max_files, max_space, log_file, socket_name, no_workers
        );
        return -1;
    }

    fclose(config);

    return 0;
}