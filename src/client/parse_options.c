#include "client.h"

int parse_options(list_t* request_list, int argc, char* argv[]){
    int opt;

    while( (opt = getopt(argc, argv, "hf:t:pa:w:")) != -1 ){
        switch(opt){
            
            // -h prints helper message
            case 'h': {
                h_option = true;
                return 0;
            }

            // -f sets socket name
            case 'f': {
                if(!f_option){
                    config.socket = optarg;
                    f_option = true;
                    break;
                } else {
                    fprintf(stderr, "Option -f can only be set once.\n");
                    return -1;
                }
            }

            // -w sends a directory to server
            case 'w': {
                char* directory;
                char* no_of_files_str;
                char* save = NULL;
                long files = -1;

                directory = strtok_r(optarg, ",", &save);
                if(directory == NULL){
                    fprintf(stderr, "Error in parsing option -w: couldn't read directory name.\n");
                    return -1;
                }
                no_of_files_str = strtok_r(NULL, ",", &save);
                if(no_of_files_str != NULL && sscanf(no_of_files_str, "n=%ld", &files) == EOF){
                    fprintf(stderr, "Error in parsing option -w: couldn't read number of files.\n");
                    return -1;
                }

                str_long_pair_t* arg;
                if( (arg = malloc(sizeof(str_long_pair_t))) == NULL){
                    perror("Error in memory allocation");
                    return -1;
                }
                arg->dir = directory;
                arg->n_files = files;

                if(list_push_back(request_q, "w", (void*)arg) == -1){
                    perror("List push back error");
                    return -1;
                }
                break;
            }

            // -t sets time between requests
            case 't': {
                long time;

                if(str_to_long(optarg, &time) != 0){
                    perror("str_to_long error");
                    fprintf(stderr, "Error in parsing option -t.\n");
                    return -1;
                }

                if(list_push_back(request_list, "t", (void*)time) != 0){
                    perror("List push_back error");
                    fprintf(stderr, "Error in parsing option -t.\n");
                    return -1;
                }
                break;
            }
            
            // -p option sets printing mode
            case 'p': {
                if(!p_option){
                    config.print_to_stdout = true;
                    p_option = true;
                    break;
                } else {
                    fprintf(stderr, "Error in parsing options: option -p can only be set once.\n");
                    return -1;
                }
            }

            // -a option sets time to await before giving a timeout
            case 'a': {
                if(!a_option){
                    if(str_to_long(optarg, &config.waiting_sec) == -1){
                        perror("str_to_long error");
                        fprintf(stderr, "Error in parsing option -a.\n");
                        return -1;
                    }

                    if(config.waiting_sec < 0){
                        fprintf(stderr, "Error in parsing option -a: negative time in seconds.\n");
                        return -1;
                    }
                    
                    a_option = true;
                    break;
                } else {
                    fprintf(stderr, "Error in parsing options: option -a can only be set once.\n");
                    return -1;
                }
            }

            // Option without an argument
            case ':': {
                fprintf(stdout, "Error in parsing options: option -%c requires an argument.\n", optopt);
                return -1;
            }
            
            case '?': {
                fprintf(stdout, "Error in parsing options: option -%c is not recognized.\n", optopt);
                return -1;
            }
        }
    }

    return 0;
}

