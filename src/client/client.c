#include "client.h"

client_conf_t config;
list_t* request_q = NULL;

bool h_option = false;
bool f_option = false;
bool p_option = false;
bool w_option = false;

#ifdef DEBUG
static void print_request_q();
#endif

static void print_helper();
static void clean_req_node(node_t* node);
static int add_to_current_time(long sec, long nsec, struct timespec* res);
static void cleanup();


int main(int argc, char* argv[]){
    if( atexit(cleanup) == -1){
        perror("atexit");
        return -1;
    }

    // ----------- INIT CONFIG ----------- //
    config.socket = NULL;
    config.print_to_stdout = false;
    config.waiting_sec = 5;

    if( (request_q = empty_list()) == NULL){
        perror("list malloc");
        return -1;
    }

    if( parse_options(request_q, argc, argv) == -1) {
        fprintf(stderr, "Printing helper message.\n\n");
        print_helper();
        return -1;
    }

    if(h_option) {
        print_helper();
        return 0;
    }

    if(!f_option){
        fprintf(stderr, "Option -f must be present once and only once, but it wasn't specified.\n");
        return -1;
    }


    // ------------ CONNECT ------------ //
    // try to connect and then disconnect from socket
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

    sleep(3);

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
    printf("Possible options:\n");
    printf("\t-h \t\t\tPrints this helper.\n");
    printf("\t-f <sock> \t\tSets socket name to <sock>. \033[0;31m This option must be set once and only once. \033[0m\n");
    printf("\t-p \t\t\tIf set, every operation will be printed to stdout. \033[0;31m This option must be set at most once. \033[0m\n");
    printf("\t-t <time> \t\tSets the waiting time (in milliseconds) between requests. Default is 0.\n");
    printf("\t-w <time> \t\tSets the time (in seconds) after which the app will stop attempting to connect to server. Default value is 0. \033[0;31m This option must be set at most once. \033[0m\n");
    printf("\n");
}

#ifdef DEBUG
static void print_request_q(){
    node_t* curr = request_q->head;

    while(curr != NULL){
        printf("%s: %ld\n", curr->key, (long)curr->data);
        curr = curr->next;
    }
}
#endif

static void clean_req_node(node_t* node){
    free(node);
}


static void cleanup(){
    #ifdef DEBUG
    print_request_q();
    #endif
    
    if(request_q != NULL)
        list_delete(&request_q, clean_req_node);
}