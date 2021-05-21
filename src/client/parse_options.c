#include "client.h"

static int comma_sep(list_t* list, char* arg);

int parse_options(list_t* request_list, int argc, char* argv[]){
    int opt;

    while( (opt = getopt(argc, argv, ":hf:t:pa:w:W:D:r:R::")) != -1 ){
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
                long files = 0;

                directory = strtok_r(optarg, ",", &save);
                if(directory == NULL){
                    fprintf(stderr, "Error in parsing option -w: couldn't read directory name.\n");
                    return -1;
                }
                no_of_files_str = strtok_r(NULL, ",", &save);
                if(no_of_files_str != NULL && (sscanf(no_of_files_str, "%ld", &files) == EOF || files < 0)){
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

            // -W sends a list of files to server
            case 'W': {
                list_t* files;

                if( (files = empty_list()) == NULL){
                    perror("Malloc error");
                    fprintf(stderr, "Error in parsing option -W.\n");
                    return -1;
                }

                if( comma_sep(files, optarg) == -1){
                    fprintf(stderr, "Error in parsing option -W.\n");
                    return -1;
                }

                if( list_push_back(request_list, "W", (void*)files) == -1){
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

            // -D option sets directory in which to write files "expelled" by server app
            case 'D': {
                if(list_push_back(request_list, "D", (void*)optarg) == -1){
                    perror("List push back error");
                    fprintf(stderr, "Error in parsing option -D.\n");
                    return -1;
                }
            }

            // -r reads a list of files from server
            case 'r': {
                list_t* files;

                if( (files = empty_list()) == NULL){
                    perror("Malloc error");
                    fprintf(stderr, "Error in parsing option -r.\n");
                    return -1;
                }

                if( comma_sep(files, optarg) == -1){
                    fprintf(stderr, "Error in parsing option -r.\n");
                    return -1;
                }

                if( list_push_back(request_list, "r", (void*)files) == -1){
                    perror("List push back error");
                    return -1;
                }
                break;                
            }

            // -R option reads n files from server
            case 'R': {
                long n = 0;
                
                if(optarg != NULL) {
                    if(sscanf(optarg, "%ld", &n)  == EOF || n < 0){
                        fprintf(stderr, "Error in parsing option -R.\n");
                        return -1;
                    }
                }
                if(list_push_back(request_list, "R", (void*)n) == -1){
                    perror("Error in adding element to list");
                    fprintf(stderr, "Error in parsing option -R.\n");
                    return -1;
                }
                break;
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

static int comma_sep(list_t* list, char* arg){
    if(list == NULL){
        errno = EINVAL;
        return -1;
    }

    char* save;
    char* token = strtok_r(arg, ",", &save);

    while(token) {
        if(list_push_back(list, token, NULL) == -1)
            return -1;
        token = strtok_r(NULL, ",", &save);
    }
    return 0;
}