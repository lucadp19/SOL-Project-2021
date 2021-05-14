#include "client.h"

client_conf_t conf;
list_t* request_q = NULL;

bool h_option = false;
bool f_option = false;
bool p_option = false;

#ifdef DEBUG
void print_request_q(){
    node_t* curr = request_q->head;

    while(curr != NULL){
        printf("%s: %d\n", curr->key, *(int*)curr->data);
        curr = curr->next;
    }
}
#endif

void clean_req_node(node_t* node){
    free(node->data);
    free(node);
}

void cleanup(){
    #ifdef DEBUG
    print_request_q();
    #endif
    
    if(request_q != NULL)
        list_delete(request_q, clean_req_node);
}

int main(int argc, char* argv[]){
    if( atexit(cleanup) == -1){
        perror("atexit");
        return -1;
    }

    // Initializing client configuration
    conf.socket = NULL;
    conf.print_to_stdout = false;

    if( (request_q = empty_list()) == NULL){
        perror("list malloc");
        return -1;
    }

    if( parse_options(request_q, argc, argv) == -1) {
        fprintf(stderr, "Error in parsing options.\n");
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

    return 0;
}

int parse_options(list_t* request_list, int argc, char* argv[]){
    int opt;

    while( (opt = getopt(argc, argv, "hf:t:p")) != -1 ){
        switch(opt){
            // if -h option is set no request should be fulfilled
            case 'h': {
                h_option = true;
                return 0;
            }

            // -f sets socket name
            case 'f': {
                if(!f_option){
                    conf.socket = optarg;
                    f_option = true;
                    break;
                } else {
                    fprintf(stderr, "Option -f can only be set once.\n");
                    return -1;
                }
            }

            // -t sets time between requests
            case 't': {
                node_t* time_node;
                long* ptr_val;

                if( (ptr_val = malloc(sizeof(long))) == NULL){
                    perror("-t option malloc");
                    return -1;
                }
                if(str_to_long(optarg, ptr_val) != 0){
                    perror("-t option str_to_long");
                    return -1;
                }

                if( (time_node = create_node("t", (void*)ptr_val)) == NULL){
                    perror("-t option create_node");
                    return -1;
                }

                if(list_push_back(request_list, time_node) != 0){
                    // TODO: how to perror?
                    return -1;
                }
                break;
            }
            
            // -p option sets printing mode
            case 'p': {
                if(!p_option){
                    conf.print_to_stdout = true;
                    p_option = true;
                    break;
                } else {
                    fprintf(stderr, "Option -p can only be set once.\n");
                    return -1;
                }
            }
            
            // Option without an argument
            case ':': {
                fprintf(stdout, "Option -%c requires an argument.\n", optopt);
                return -1;
            }
            
            case '?': {
                fprintf(stdout, "Option -%c is not recognized.\n", optopt);
                return -1;
            }
        }
    }

    return 0;
}

void print_helper(){
    printf("This helper has not been made yet :(\n");
}
