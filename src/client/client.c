#include "client.h"
#include "util/files.h"

client_conf_t config;
list_t* request_q = NULL;

#ifdef DEBUG
static void print_request_q();
#endif

static void print_helper();
static void clean_req_node(node_t* node);
static int add_to_current_time(long sec, long nsec, struct timespec* res);
static void cleanup();

// #define TEST

int main(int argc, char* argv[]){
    if( atexit(cleanup) == -1){
        perror("Error in setting up cleanup function");
        return -1;
    }

    // ----------- INIT CONFIG ----------- //
    config.socket = NULL;
    config.print_to_stdout = false;
    config.waiting_sec = 5;
    config.print_helper = false;

    if( (request_q = empty_list()) == NULL){
        perror("Fatal error in creating list");
        fprintf(stderr, "Aborting.\n");
        return -1;
    }

    if( parse_options(request_q, argc, argv) == -1) {
        fprintf(stderr, "\nPrinting helper message.\n\n");
        print_helper();
        return -1;
    }

    if( config.print_helper ) {
        print_helper();
        return 0;
    }

    // check that options respect the given restrictions
    validate_options();

    // ------------ CONNECT ------------ //
    int err;
    struct timespec abstime;
    
    add_to_current_time(
        config.waiting_sec, 
        0, 
        &abstime
    );
    
    if( (err = openConnection(config.socket, TIME_BETWEEN_CONN, abstime)) == -1){
        perror("Connection failed");
        return -1;
    } else if( config.print_to_stdout )
        printf("Connected to %s!\n", config.socket);

    execute_requests();

    // ------- CLOSING CONNECTION ------ //
    if( (err = closeConnection(config.socket)) == -1){
        perror("Couldn't close connection");
        return -1;
    } else if( config.print_to_stdout )
        printf("Connection to %s closed!\n", config.socket);

    return 0;
}

static int add_to_current_time(long sec, long nsec, struct timespec* res){
    // TODO: maybe check its result
    clock_gettime(CLOCK_REALTIME, res);
    res->tv_sec += sec;
    res->tv_nsec += nsec;

    return 0;
}

static void print_helper(){
    printf("A client application for the File Storage System.\n");
    printf("Usage: ./bin/client [options]\n");
    printf("Possible options:\n");
    printf("\t-h \t\t\tPrints this helper.\n");
    printf("\t-f <sock> \t\tSets socket name to <sock>. \033[0;31m This option must be set once and only once. \033[0m\n");
    printf("\t-p \t\t\tIf set, every operation will be printed to stdout. \033[0;31m This option must be set at most once. \033[0m\n");
    printf("\t-t <time> \t\tSets the waiting time (in milliseconds) between requests. Default is 0.\n");
    printf("\t-a <time> \t\tSets the time (in seconds) after which the app will stop attempting to connect to server. Default value is 5 seconds. \033[0;31m This option must be set at most once. \033[0m\n");
    printf("\t-w <dir>[,<num>] \tSends the content of the directory <dir> (and its subdirectories) to the server. If <num> is specified, at most <num> files will be sent to the server.\n");
    printf("\t-W <file>{,<files>}\tSends the files passed as arguments to the server.\n");
    printf("\t-D <dir>\t\tWrites into directory <dir> all the files expelled by the server app. \033[0;31m This option must follow one of -w or -W. \033[0m\n");
    printf("\t-r <file>{,<files>}\tReads the files specified in the argument list from the server.\n");
    printf("\t-R[<num>] \t\tReads <num> files from the server. If <num> is not specified, reads all files from the server. \033[0;31m There must be no space bewteen -R and <num>.\033[0m\n");
    printf("\t-d <dir> \t\tWrites into directory <dir> the files read from server. If it not specified, files read from server will be lost. \033[0;31m This option must follow one of -r or -R. \033[0m\n");
    printf("\t-l <file>{,<files>} \tLocks all the files given in the file list.\n");
    printf("\t-u <file>{,<files>} \tUnlocks all the files given in the file list.\n");
    printf("\t-c <file>{,<files>} \tDeletes from server all the files given in the file list, if they exist.\n");
    printf("\n");
}

#ifdef DEBUG
static void print_request_q(){
    node_t* curr = request_q->head;

    while(curr != NULL){
        // printf("%s: %ld\n", curr->key, (long)curr->data);
        if(curr->key == NULL) {
            printf("curr->key is NULL :(\n");
            continue;
        }
        switch(curr->key[0]){
            case 't': 
                printf("-t %ld\n", (long)curr->data);
                break;
            case 'w': {
                str_long_pair_t* arg;
                arg = (str_long_pair_t*)curr->data;
                printf("-w %s %ld\n", arg->dir, arg->n_files);
                break;
            }
            case 'R':
                printf("-R %ld\n", (long)curr->data);
                break;
            case 'D': 
            case 'd': {
                printf("-%c %s\n", curr->key[0], (char*)curr->data);
                break;
            }
            case 'W':
            case 'r':
            case 'l':
            case 'u': 
            case 'c': {
                list_t* args;
                args = (list_t*)curr->data;
                printf("-%c", curr->key[0]);

                node_t* curr = args->head;
                while(curr != NULL){
                    printf(" %s", curr->key);
                    curr = curr->next;
                }
                printf("\n");
                break;
            }
        }
        curr = curr->next;
    }
}
#endif

static void clean_req_node(node_t* node){
    if(node == NULL) return;
    if(node->key == NULL){
        free(node);
        return;
    }
    
    switch(node->key[0]){
        case 'w': {
            str_long_pair_t* arg = (str_long_pair_t*)node->data;
            free(arg->dir);
            free(arg);
            free(node);
            break;
        }
        case 't':
        case 'D':
        case 'R':      
        case 'd':
            free(node);
            break; 
        case 'W':
        case 'r':
        case 'l':
        case 'u':
        case 'c':
            list_delete((list_t**)&(node->data), free_node_and_key);
            free(node);
            break;
    }
}


static void cleanup(){
    #ifdef DEBUG
    print_request_q();
    #endif
    
    if(request_q != NULL)
        list_delete(&request_q, clean_req_node);
}