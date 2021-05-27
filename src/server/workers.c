#include "server.h"
#include "server-api-protocol.h"

int install_workers(pthread_t worker_tids[], int** worker_pipes){
    if(worker_tids == NULL || worker_pipes == NULL){
        errno = EINVAL;
        perror("Null arguments to install_workers");
        return -1;
    }

    int err;

    for(int i = 0; i < server_config.n_workers; i++){
        if( (err = pthread_create(&(worker_tids[i]), NULL, worker_thread, (void*)(worker_pipes[i]))) != 0){
            errno = err;
            return -1;
        }
    }
    return 0;
}

void* worker_thread(void* arg){
    if(arg == NULL){
        // returns 22l -> code for EINVAL
        return (void*)22l;
    }

    debug("Hello I'm a worker thread!\n");
    
    int* pipe = (int*) arg;

    while(mode != CLOSE_SERVER){
        long fd_client;

        safe_pthread_mutex_lock(&request_queue_mtx);
        if(mode != CLOSE_SERVER && request_queue->nelem == 0)
            safe_pthread_cond_wait(&request_queue_nonempty, &request_queue_mtx);
        
        if(mode == CLOSE_SERVER) {
            safe_pthread_mutex_unlock(&request_queue_mtx);
            break;
        } 
        // request queue isn't empty
        list_pop_front(request_queue, NULL, (void**)&fd_client);
        safe_pthread_mutex_unlock(&request_queue_mtx);

        // do something with client
        worker_res_t result;
        // memsetting otherwise valgrind will complain
        memset(&result, 0, sizeof(worker_res_t));
        result.fd_client = fd_client;

        // TODO: actual worker code
        debug("> THREAD: got request from fd %ld.\n", fd_client);

        int l;
        op_code_t op_code;

        if( (l = readn(fd_client, &op_code, sizeof(op_code_t))) == -1){ // error in reading
            result.code = MW_FATAL_ERROR;
            if( writen(pipe[W_ENDP], &result, sizeof(worker_res_t)) == -1){
                perror("Error writen");
                return (void*)-1l; // TODO: same thing :(
            }
            continue;
        }

        if( l == 0 ){ // closed connection!
            result.code = MW_CLOSE;
            if( writen(pipe[W_ENDP], &result, sizeof(worker_res_t)) == -1){
                perror("Error writen");
                return (void*)-1l; // TODO: same thing :(
            }
            continue;
        }

        debug("op_code read. It is %d\n", op_code);

        switch (op_code) {
            case OPEN_FILE: {
                int res = open_file(fd_client);

                // setting result code for main thread
                if(res == SA_SUCCESS)
                    result.code = MW_SUCCESS;
                else if(res == SA_CLOSE || res == SA_ERROR)
                    result.code = MW_FATAL_ERROR;
                else result.code = MW_NON_FATAL_ERROR;

                // setting result code for client
                int err;
                // TODO: maybe make function res -> errno
                switch(res){
                    case SA_SUCCESS:
                        err = 0;
                        break;
                    case SA_EXISTS:
                        err = EEXIST;
                        break;
                    case SA_NO_FILE:
                        err = ENOENT;
                        break;
                    case SA_ALREADY_LOCKED:
                        err = EBUSY;
                        break;
                    case SA_CLOSE:
                    case SA_ERROR:
                    default:
                        err = -1;
                        break;
                }

                if( writen(fd_client, &err, sizeof(int)) == -1){
                    perror("Error in writing to client");
                    result.code = MW_FATAL_ERROR;
                    break;
                }

                break;
            }

            default:
                result.code = MW_FATAL_ERROR;
                break;
        }

        if( writen(pipe[W_ENDP], &result, sizeof(worker_res_t)) == -1){
            perror("Error writen");
            return (void*)-1l; // TODO: same thing :(
        }
        debug("> Job finished!\n");
    }
    
    debug("Closing thread!\n");

    close(pipe[W_ENDP]);
    pipe[W_ENDP] = -1;
    return NULL;
}