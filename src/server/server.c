#include "server.h"
#include "math.h"

server_config_t server_config;
server_mode_t mode = ACCEPT_CONN;

hashtbl_t* conn_client_table = NULL;

list_t* request_queue = NULL;
pthread_mutex_t request_queue_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t request_queue_nonempty = PTHREAD_COND_INITIALIZER;

hashmap_t* files = NULL;
pthread_mutex_t files_mtx = PTHREAD_MUTEX_INITIALIZER;

/** 
 * pthread_yield for the given threads.
 */
static int yield_all_threads(pthread_t sig_handler_tid, pthread_t worker_tids[]);
/**
 * Closing all the open pipes.
 */
static void close_all_pipes(int sig_handler_pipe[], int* worker_pipes[]);
/**
 * Unlinks socket.
 */
static void unlink_socket();
/**
 * Updates the maximum file descriptor set in a fd_set.
 * If no descriptor is set returns -1.
 */
static inline int update_max(fd_set set, int fd_max);
/**
 * A simple hash function.
 */
static long hash_funct(long val, long nlist);
/**
 * A simple string hash function.
 */
static void str_hash_funct(char* str, long nlist);



int main(int argc, char* argv[]){ 
    if(argc > 2){
        fprintf(stderr, "There must be at most one additional argument: the path to the config file.\n");
        fprintf(stderr, "If no argument is supplied, the default is \"./config/config.txt\".\n");
        return -1;
    }

    char* config_path = (argc == 1) ? "config/config.txt" : argv[1];
     
    if( get_server_config(config_path) == -1){
        // perror("Error in reading config file");
        return -1;
    }
    // ----- MAX FILE DESCRIPTOR ----- //
    long fd_max = -1;

    // ------- SIGNAL HANDLING ------- //
    pthread_t sig_handler_tid;
    int* sig_handler_pipe;
    if( (sig_handler_pipe = calloc(2, sizeof(int))) == NULL){
        perror("Signal handler pipe allocation failed");
        return -1;
    }
    pipe_init(sig_handler_pipe);

    if( pipe(sig_handler_pipe) == -1){
        perror("Error in creating sig_handler_pipe");
        return -1;
    }
    fd_max = sig_handler_pipe[R_ENDP];

    if( install_sig_handler(sig_handler_pipe, &sig_handler_tid) == -1){
        perror("Error in installing signal handler");
        return -1;
    }

    // ---------- CLIENT TABLE --------- //
    if( hashtbl_init(&conn_client_table, HASH_N_LIST, hash_funct) == -1){
        perror("Error while creating hashtable");
        return -1;
    }

    // ----------- FILES MAP ----------- //
    if( hashmap_init(&files, HASH_N_LIST, str_hash_funct, files_node_cleaner) == -1){
        perror("Error while creating hashmap");
        return -1;
    }

    // --------- REQUEST QUEUE --------- //
    if( (request_queue = empty_list()) == NULL){
        perror("Error while creating new list");
        return -1;
    }

    // ------ WORKER CREATION ------ //
    pthread_t* worker_tids = NULL;
    int** worker_pipes = NULL;
    
    // allocating memory for worker thread ids
    if( (worker_tids = (pthread_t*)calloc(server_config.n_workers, sizeof(pthread_t))) == NULL){
        perror("Error in calloc for worker threads ids");
        return -1;
    }
    // setting them to -1
    for(int i = 0; i < server_config.n_workers; i++) worker_tids[i] = -1;

    // allocating memory for worker pipes' vector
    if( (worker_pipes = (int**)calloc(server_config.n_workers, sizeof(int*))) == NULL){
        perror("Error in calloc for worker pipes");
        return -1;
    }
    // allocating memory for worker pipes and creating pipes
    for(int i = 0; i < server_config.n_workers; i++){
        if( (worker_pipes[i] = (int*)calloc(2, sizeof(int*))) == NULL){
            perror("Error in calloc for worker pipes");
            return -1;
        }
        // setting pipes to -1
        pipe_init(worker_pipes[i]);

        if( pipe(worker_pipes[i]) == -1){
            perror("Error in creating pipe");
            return -1;
        }
        if(worker_pipes[i][R_ENDP] > fd_max) fd_max = worker_pipes[i][R_ENDP];
    }

    if( install_workers(worker_tids, worker_pipes) == -1){
        perror("Error in creating workers");
        return -1;
    }

    // --------- SOCKET ---------- //
    long fd_listen = -1;
    unlink_socket();
    atexit(unlink_socket);

    if( (fd_listen = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("Error in socket creation");
        return -1;
    }

    struct sockaddr_un sock_addr;
    memset(&sock_addr, '0', sizeof(sock_addr));
    strncpy(sock_addr.sun_path, server_config.socket_path, strlen(server_config.socket_path) + 1);
    sock_addr.sun_family = AF_UNIX;
    
    if( bind(fd_listen, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1){
        perror("Error in socket binding");
        return -1;
    }
    if( listen(fd_listen, SOMAXCONN) == -1){
        perror("Error in socket binding");
        return -1;
    }
    if(fd_listen > fd_max) fd_max = fd_listen;

    // ------------ FD_SET ------------ //
    fd_set set, tmpset;
    // setting both sets to 0
    FD_ZERO(&set);
    FD_ZERO(&tmpset);

    FD_SET(fd_listen, &set);
    FD_SET(sig_handler_pipe[R_ENDP], &set);
    for(int i = 0; i < server_config.n_workers; i++)
        FD_SET(worker_pipes[i][R_ENDP], &set);


    // ------------------- MAIN LOOP -------------------- //

    while(mode != CLOSE_SERVER){
        tmpset = set;

        if( select(fd_max + 1, &tmpset, NULL, NULL, NULL) == -1){
            perror("Select failed");
            return -1;
        }

        for (int i = 0; i <= fd_max; i++){
            // i-th file descriptor is not set
            if(!FD_ISSET(i, &tmpset)) continue;
            // i is set
            
            if(i == fd_listen && mode == ACCEPT_CONN){ // new connection request
                long fd_client;
                if( (fd_client = accept(fd_listen, (struct sockaddr*)NULL, NULL)) == -1){
                    perror("Accept failed");
                    return -1;
                }

                debug("New connection! File descriptor: %ld.\n", fd_client);

                // adding client to master set
                FD_SET(fd_client, &set);
                if(fd_client > fd_max) fd_max = fd_client;

                // adding client to hashtable
                if( hashtbl_insert(&conn_client_table, fd_client) == -1){
                    perror("Error while adding client to table");
                    return -1;
                }

                continue;
            }

            // termination signal from sig_handler thread
            if(i == sig_handler_pipe[R_ENDP] && mode == CLOSE_SERVER){
                // waking up sleeping threads
                safe_pthread_cond_broadcast(&request_queue_nonempty);
                break;
            }

            // worker or client?
            bool is_client_request = true;
            for(int j = 0; j < server_config.n_workers; j++){
                if(i != worker_pipes[j][R_ENDP]) continue;

                // found the right pipe!
                debug("Reading result from thread %d\n", i);
                is_client_request = false;
                // reading the result from thread
                worker_res_t result;
                if( readn(worker_pipes[j][R_ENDP], &result, sizeof(worker_res_t)) == -1){
                    perror("Error while reading result from thread");
                    return -1;
                }
            
                switch (result.code){
                    case 0: // success :)
                        FD_SET(result.fd_client, &set);
                        if(result.fd_client > fd_max) fd_max = result.fd_client;
                        break;

                    case 1: // closing connection
                        #ifndef DEBUG
                            printf("Closed connection with client %ld!\n", result.fd_client);
                            fflush(stdout);
                        #endif
                        if( hashtbl_remove(conn_client_table, result.fd_client) == -1){
                            perror("Error while removing file descriptor from hashtable");
                            return -1;
                        }
                        close(result.fd_client);
                        break;
                    
                    default: // ?? unknown ??
                        fprintf(stderr, "Unknown option returned by worker thread. Closing.");
                        return -1;
                        break;
                }
            }
            if(!is_client_request) continue;

            // it's a client request
            long fd_client = i;

            debug("New request from client %ld!\n", fd_client);

            // inserting it into the request queue
            safe_pthread_mutex_lock(&request_queue_mtx);
            if( list_push_back(request_queue, NULL, (void*)fd_client) == -1){
                perror("Error while inserting new element in queue");
                return -1;
            }
            safe_pthread_cond_signal(&request_queue_nonempty);
            safe_pthread_mutex_unlock(&request_queue_mtx);
            // removing i from the select set
            FD_CLR(i, &set);
            if(i == fd_max) {
                fd_max = update_max(set, fd_max);
                if(fd_max == -1){
                    fprintf(stderr, "Fatal error: no file descriptor connected.");
                    exit(EXIT_FAILURE);
                }
            }
        }

        // no more connections
        if(mode == REFUSE_CONN && conn_client_table->nelem == 0){
            mode = CLOSE_SERVER;
            // waking up threads blocked on a pthread_cond_wait
            safe_pthread_cond_broadcast(&request_queue_nonempty);
        }
    }

    // -------------- CLOSING THINGS -------------- //
    
    if(fd_listen != -1){
        close(fd_listen);
        fd_listen = -1;
    }

    if( yield_all_threads(sig_handler_tid, worker_tids) == -1){
        perror("Error in thread yield");
        return -1;
    }

    close_all_pipes(sig_handler_pipe, worker_pipes);

    // closing all file descriptors still open
    hash_iter_t* iter = NULL;
    if( (iter = malloc(sizeof(hash_iter_t))) == NULL){
        perror("Error in iterator creation");
        return -1;
    }
    if(hash_iter_init(iter) == -1){
        perror("Error in iterator initialization");
        return -1;
    }
    while(true){
        int err = hashtbl_iter_get_next(iter, conn_client_table);
        
        if(err == 1) // end of table
            break;
        
        if(err == -1){ // actual error
            perror("Error in hashtbl_iter_get_next");
            return -1;
        }

        long fd_client = (long)iter->current_pos->data;
        close(fd_client);
    }
    free(iter);

    // freeing memory    
    hashtbl_free(&conn_client_table);
    list_delete(&request_queue, free_only_node);

    if(sig_handler_pipe != NULL) {
        free(sig_handler_pipe);
        sig_handler_pipe = NULL;
    }        
    if(worker_tids != NULL) {
        free(worker_tids);
        worker_tids = NULL;
    }
    if(worker_pipes != NULL) {
        for(int i = 0; i < server_config.n_workers; i++) {
            free(worker_pipes[i]);
            worker_pipes[i] = NULL;
        }
        free(worker_pipes);
        worker_pipes = NULL;
    }

    return 0;
}

static int yield_all_threads(pthread_t sig_handler_tid, pthread_t worker_tids[]){
    int err;
    if(sig_handler_tid != -1){
        if( (err = pthread_join(sig_handler_tid, NULL)) != 0){
            errno = err;
            return -1;
        } 
        sig_handler_tid = -1;
    }

    if(worker_tids != NULL) {
        for(int i = 0; i < server_config.n_workers; i++){
            if(worker_tids[i] == -1) continue;

            long ret_val;
            if( (err = pthread_join(worker_tids[i], (void**)&ret_val)) == -1){
                errno = err;
                return -1;
            } if(ret_val != 0){
                errno = err;
                return -1;
            }
        }
    }

    return 0;
}

static void close_all_pipes(int sig_handler_pipe[], int* worker_pipes[]){
    if(sig_handler_pipe != NULL) {
        for(int j = 0; j < 2; j++)
            if(sig_handler_pipe[j] != -1)
                close(sig_handler_pipe[j]);
    }
    
    if(worker_pipes != NULL) {
        for(int i = 0; i < server_config.n_workers; i++){
            if(worker_pipes[i] == NULL) continue;

            for(int j = 0; j < 2; j++)
                if(worker_pipes[i][j] != -1)
                    close(worker_pipes[i][j]);
        }
    }
}

static void unlink_socket(){
    unlink(server_config.socket_path);
}

static inline int update_max(fd_set set, int fd_max){
    for(int i = fd_max; i >= 0; i--)
        if(FD_ISSET(i, &set)) 
            return i;
    
    return -1;
}

// Taken from Cormen and the following link:
// https://www.cs.hmc.edu/~geoff/classes/hmc.cs070.200101/homework10/hashfuncs.html
static long hash_funct(long val, long nlist){
    double A = 0.5*(sqrt(5) - 1);
    double useless;
    return (long)floor(nlist*modf(val*A, &useless));
}

// Taken from the following link:
// http://www.cse.yorku.ca/~oz/hash.html
static long str_hash_funct(const char* str, long nlist){
    long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static void files_node_cleaner(node_t* node){
    if(node == NULL) return;

    file_t* file = (file_t*)node->data;
    file_delete(file);
}

void file_delete(file_t* file){
    if(file == NULL) return;

    if(file->path_name != NULL)
        free(file->path_name);
    if(file->contents != NULL)
        free(file->contents);
    
    free(file);
}