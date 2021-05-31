#include "client.h"

bool h_option = false;
bool f_option = false;
bool p_option = false;
bool a_option = false;

/** 
 * Given an empty list and a string, separates the string into
 * comma separated substrings and adds them all into list.
 * If absolute == true, substring are path for files and the function will automatically
 * convert them into absolute paths.
 * 
 * If it cannot convert a path into an absolute one, or it cannot add nodes to the given list,
 * this function shall print an error message and abort the process.
 * 
 * Returns 0 on success, -1 if the given list is NULL (sets errno to EINVAL).
 */
static int comma_sep(list_t* list, char* arg, bool absolute);

int parse_options(list_t* request_list, int argc, char* argv[]){
    int opt;

    while( (opt = getopt(argc, argv, ":hf:t:pa:w:W:D:r:R::d:l:u:c:")) != -1 ){
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
                char* abs_directory;
                char* no_of_files_str;
                char* save = NULL;
                long files = 0;

                // getting directory name
                directory = strtok_r(optarg, ",", &save);
                if(directory == NULL){
                    fprintf(stderr, "Error in parsing option -w: couldn't read directory name.\n");
                    return -1;
                }

                // converting it into absolute path
                if( (abs_directory = realpath(directory, NULL)) == NULL){
                    fprintf(stderr, "Error in parsing option -w: directory doesn't exist.\n");
                    return -1;
                }

                no_of_files_str = strtok_r(NULL, ",", &save);
                if(no_of_files_str != NULL && (sscanf(no_of_files_str, "%ld", &files) == EOF || files < 0)){
                    fprintf(stderr, "Error in parsing option -w: couldn't read number of files.\n");
                    return -1;
                }

                str_long_pair_t* arg;
                arg = safe_malloc(sizeof(str_long_pair_t));
                arg->dir = abs_directory;
                arg->n_files = files;

                if(list_push_back(request_list, "w", (void*)arg) == -1){
                    perror("Error in adding element to list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }

            // -W sends a list of files to server
            case 'W': {
                list_t* files;

                if( (files = empty_list()) == NULL){
                    perror("Error in creating list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }

                comma_sep(files, optarg, true);

                if( list_push_back(request_list, "W", (void*)files) == -1){
                    perror("Error in adding element to list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }

            // -t sets time between requests
            case 't': {
                long time;

                if(str_to_long(optarg, &time) != 0) {
                    perror("Error in converting string to integer");
                    fprintf(stderr, "Error in parsing option -t.\n");
                    return -1;
                }

                if(time < 0) {
                    fprintf(stderr, "Error in parsing option -t: time must be a positive integer.\n");
                    return -1;
                }

                if(list_push_back(request_list, "t", (void*)time) != 0){
                    perror("Error in adding element to list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }
            // -p option sets printing mode
            case 'p': {
                if(!p_option){
                    config.print_to_stdout = true;
                    p_option = true;
                } else {
                    fprintf(stderr, "Error in parsing options: option -p can only be set once.\n");
                    return -1;
                }
                break;
            }

            // -D option sets directory in which to write files "expelled" by server app
            case 'D': {
                if(list_push_back(request_list, "D", (void*)optarg) == -1){
                    perror("Error in adding element to list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }

            // -r reads a list of files from server
            case 'r': {
                list_t* files;

                if( (files = empty_list()) == NULL){
                    perror("Error in creating list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }

                comma_sep(files, optarg, true);

                if( list_push_back(request_list, "r", (void*)files) == -1){
                    perror("Error in adding element to list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
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
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }

            // -d sets the directory in which files read from server will be printed
            case 'd': {
                if( list_push_back(request_list, "d", (void*)optarg) == -1){
                    perror("Error in pushing element to list");
                    fprintf(stderr, "Error in parsing option -d.\n");
                }
                break;
            }

            // -l locks a list of files
            case 'l': {
                list_t* files;

                if( (files = empty_list()) == NULL){
                    perror("Error in creating list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }

                comma_sep(files, optarg, true);

                if( list_push_back(request_list, "l", (void*)files) == -1){
                    perror("Error in adding element to list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;    
            }

            // -u unlocks a list of files
            case 'u': {
                list_t* files;

                if( (files = empty_list()) == NULL){
                    perror("Error in creating list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }

                comma_sep(files, optarg, true);

                if( list_push_back(request_list, "u", (void*)files) == -1){
                    perror("Error in adding element to list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;    
            }
           
            // -c deletes a list of files from server
            case 'c': {
                list_t* files;

                if( (files = empty_list()) == NULL){
                    perror("Error in creating list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }

                comma_sep(files, optarg, true);

                if( list_push_back(request_list, "c", (void*)files) == -1){
                    perror("Error in adding element to list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;    
            } 
            // -a option sets time to await before giving a timeout
            case 'a': {
                if(!a_option){
                    if(str_to_long(optarg, &config.waiting_sec) == -1){
                        perror("Error in converting string to integer");
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

static int comma_sep(list_t* list, char* arg, bool absolute){
    if(list == NULL){
        errno = EINVAL;
        return -1;
    }

    char* save;
    char* token = strtok_r(arg, ",", &save);
    char* abs_token;

    while(token) {

        // converting token into absolute path if necessary
        if(absolute){
            if( (abs_token = realpath(token, NULL)) == NULL){
                perror("Error in converting relative path into an absolute one");
                exit(EXIT_FAILURE);
            }
        } else abs_token = token;

        if(list_push_back(list, abs_token, NULL) == -1){
            perror("Error in adding element to list");
            fprintf(stderr, "Aborting.\n");
            exit(EXIT_FAILURE);
        }
        token = strtok_r(NULL, ",", &save);
    }

    return 0;
}

void validate_options(){
    if(request_q == NULL) {
        fprintf(stderr, "Request queue is NULL, aborting.\n");
        exit(EXIT_FAILURE);
    }

    if(!f_option){
        fprintf(stderr, "Option -f must be present once and only once, but it wasn't specified.\n");
        exit(EXIT_FAILURE);
    }

    node_t* curr = request_q->head;    
    while(curr != NULL){
        switch(curr->key[0]){
            case 'd': 
                if(curr->prev == NULL || (curr->prev->key[0] != 'r' && curr->prev->key[0] != 'R')) {
                    fprintf(stderr, "Option -d must only be used after one of -r or -R. Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'D': 
                if(curr->prev == NULL || (curr->prev->key[0] != 'w' && curr->prev->key[0] != 'W')) {
                    fprintf(stderr, "Option -d must only be used after one of -r or -R. Aborting.\n");
                    exit(EXIT_FAILURE);
                }
                break;
        
            default: break;
        }
        curr = curr->next;
    }
}