#include "server.h"
#include "server-api-protocol.h"

static const char* thread_res_to_msg(int res);

int install_workers(pthread_t worker_tids[], int** worker_pipes){
    if(worker_tids == NULL || worker_pipes == NULL){
        errno = EINVAL;
        perror("Null arguments to install_workers");
        return -1;
    }

    int err;

for(int i = 0; i < server_config.n_workers; i++){
        worker_arg_t* arg = safe_calloc(1, sizeof(worker_arg_t));
        arg->worker_no = i;
        arg->pipe = worker_pipes[i];
        if( (err = pthread_create(&(worker_tids[i]), NULL, worker_thread, (void*)arg)) != 0){
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

    worker_arg_t* w_arg = (worker_arg_t*)arg;
    int* pipe = w_arg->pipe;
    int worker_no = w_arg->worker_no;

    debug("Hello I'm worker no. %d!\n", worker_no);
    logger("[THREAD %d] Worker thread created.\n", worker_no);

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
        logger("[THREAD %d] [NEW_REQ] Accepted request from client %ld.\n", worker_no, fd_client);

        int l;
        op_code_t op_code;

        if( (l = readn(fd_client, &op_code, sizeof(op_code_t))) == -1){ // error in reading
            result.code = MW_FATAL_ERROR;
            logger("[THREAD %d] [FATALERROR] Fatal error in reading client request.\n");
            if( writen(pipe[W_ENDP], &result, sizeof(worker_res_t)) == -1){
                perror("Error writen");
                return (void*)-1l; // TODO: same thing :(
            }
            continue;
        }

        if( l == 0 ){ // closed connection!
            result.code = MW_CLOSE;
            logger("[THREAD %d] [CLOSE_CONN] Closing connection with client %ld.\n", worker_no, fd_client);
            if( writen(pipe[W_ENDP], &result, sizeof(worker_res_t)) == -1){
                perror("Error writen");
                return (void*)-1l; // TODO: same thing :(
            }
            continue;
        }

        debug("op_code read. It is %d\n", op_code);

        switch (op_code) {
            case OPEN_FILE: {
                logger("[THREAD %d] [OPEN_FILE] Request from client %ld is OPEN_FILE.\n", worker_no, fd_client);
                int res = open_file(worker_no, fd_client);
                
                if( writen(fd_client, &res, sizeof(int)) == -1){
                    perror("Error in writing to client");
                    result.code = MW_FATAL_ERROR;
                    break;
                }

                // setting result code for main thread
                if(res == SA_SUCCESS)
                    result.code = MW_SUCCESS;
                else if(res == SA_CLOSE || res == SA_ERROR) {
                    logger("[THREAD %d] [OPEN_FILE_FAIL] Fatal error in OPEN_FILE request from client %ld.\n", worker_no, fd_client);
                    result.code = MW_FATAL_ERROR;
                } else {
                    logger(
                        "[THREAD %d] [OPEN_FILE_FAIL] Non-fatal error in OPEN_FILE request from client %ld: %s.\n", 
                        worker_no, fd_client, thread_res_to_msg(res)
                    );
                    result.code = MW_NON_FATAL_ERROR;
                }

                break;
            }
            case CLOSE_FILE: {
                logger("[THREAD %d] [CLOSE_FILE] Request from client %ld is CLOSE_FILE.\n", worker_no, fd_client);
                int res = close_file(worker_no, fd_client);

                if( writen(fd_client, &res, sizeof(int)) == -1){
                    perror("Error in writing to client");
                    result.code = MW_FATAL_ERROR;
                    break;
                }

                // setting result code for main thread
                if(res == SA_SUCCESS)
                    result.code = MW_SUCCESS;
                else if(res == SA_CLOSE || res == SA_ERROR){
                    logger("[THREAD %d] [CLOSE_FILE_FAIL] Fatal error in CLOSE_FILE request from client %ld.\n", worker_no, fd_client);
                    result.code = MW_FATAL_ERROR;
                } else {
                    logger(
                        "[THREAD %d] [CLOSE_FILE_FAIL] Non-fatal error in CLOSE_FILE request from client %ld: %s.\n", 
                        worker_no, fd_client, thread_res_to_msg(res)
                    );
                    result.code = MW_NON_FATAL_ERROR;
                }

                break;
            }
            
            case WRITE_FILE: {
                logger("[THREAD %d] [WRITE_FILE] Request from client %ld is WRITE_FILE.\n", worker_no, fd_client);
                int res = write_file(worker_no, fd_client);

                if( writen(fd_client, &res, sizeof(int)) == -1){
                    perror("Error in writing to client");
                    result.code = MW_FATAL_ERROR;
                    break;
                }

                // setting result code for main thread
                if(res == SA_SUCCESS)
                    result.code = MW_SUCCESS;
                else if(res == SA_CLOSE || res == SA_ERROR){
                    logger("[THREAD %d] [WRITE_FILE_FAIL] Fatal error in WRITE_FILE request from client %ld.\n", worker_no, fd_client);
                    result.code = MW_FATAL_ERROR;
                } else {
                    logger(
                        "[THREAD %d] [WRITE_FILE_FAIL] Non-fatal error in WRITE_FILE request from client %ld: %s.\n", 
                        worker_no, fd_client, thread_res_to_msg(res)
                    );
                    result.code = MW_NON_FATAL_ERROR;
                }

                break;
            }
            
            case APPEND_TO_FILE: {
                logger("[THREAD %d] [APPEND_TO_FILE] Request from client %ld is APPEND_TO_FILE.\n", worker_no, fd_client);
                int res = append_to_file(worker_no, fd_client);

                if( writen(fd_client, &res, sizeof(int)) == -1){
                    perror("Error in writing to client");
                    result.code = MW_FATAL_ERROR;
                    break;
                }

                // setting result code for main thread
                if(res == SA_SUCCESS)
                    result.code = MW_SUCCESS;
                else if(res == SA_CLOSE || res == SA_ERROR){
                    logger("[THREAD %d] [APPEND_TO_FILE_FAIL] Fatal error in APPEND_TO_FILE request from client %ld.\n", worker_no, fd_client);
                    result.code = MW_FATAL_ERROR;
                } else {
                    logger(
                        "[THREAD %d] [APPEND_TO_FILE_FAIL] Non-fatal error in APPEND_TO_FILE request from client %ld: %s.\n", 
                        worker_no, fd_client, thread_res_to_msg(res)
                    );
                    result.code = MW_NON_FATAL_ERROR;
                }

                break;
            }

            case READ_FILE: {
                logger("[THREAD %d] [READ_FILE] Request from client %ld is READ_FILE.\n", worker_no, fd_client);
                int res = read_file(worker_no, fd_client);

                if( writen(fd_client, &res, sizeof(int)) == -1){
                    perror("Error in writing to client");
                    result.code = MW_FATAL_ERROR;
                    break;
                }

                // setting result code for main thread
                if(res == SA_SUCCESS)
                    result.code = MW_SUCCESS;
                else if(res == SA_CLOSE || res == SA_ERROR){
                    logger("[THREAD %d] [READ_FILE_FAIL] Fatal error in READ_FILE request from client %ld.\n", worker_no, fd_client);
                    result.code = MW_FATAL_ERROR;
                } else {
                    logger(
                        "[THREAD %d] [READ_FILE_FAIL] Non-fatal error in READ_FILE request from client %ld: %s.\n", 
                        worker_no, fd_client, thread_res_to_msg(res)
                    );
                    result.code = MW_NON_FATAL_ERROR;
                }

                break;
            } 
            case READ_N_FILES: {
                logger("[THREAD %d] [READ_N_FILES] Request from client %ld is READ_N_FILES.\n", worker_no, fd_client);
                int res = read_n_files(worker_no, fd_client);

                if( writen(fd_client, &res, sizeof(int)) == -1){
                    perror("Error in writing to client");
                    result.code = MW_FATAL_ERROR;
                    break;
                }

                // setting result code for main thread
                if(res == SA_SUCCESS)
                    result.code = MW_SUCCESS;
                else if(res == SA_CLOSE || res == SA_ERROR){
                    logger("[THREAD %d] [READ_N_FILES_FAIL] Fatal error in READ_N_FILES request from client %ld.\n", worker_no, fd_client);
                    result.code = MW_FATAL_ERROR;
                } else {
                    logger(
                        "[THREAD %d] [READ_N_FILES_FAIL] Non-fatal error in READ_N_FILES request from client %ld: %s.\n", 
                        worker_no, fd_client, thread_res_to_msg(res)
                    );
                    result.code = MW_NON_FATAL_ERROR;
                }

                break;
            }
            
            case REMOVE_FILE: {
                logger("[THREAD %d] [REMOVE_FILE] Request from client %ld is REMOVE_FILE.\n", worker_no, fd_client);
                int res = remove_file(worker_no, fd_client);

                if( writen(fd_client, &res, sizeof(int)) == -1){
                    perror("Error in writing to client");
                    result.code = MW_FATAL_ERROR;
                    break;
                }

                // setting result code for main thread
                if(res == SA_SUCCESS)
                    result.code = MW_SUCCESS;
                else if(res == SA_CLOSE || res == SA_ERROR){
                    logger("[THREAD %d] [REMOVE_FILE_FAIL] Fatal error in REMOVE_FILE request from client %ld.\n", worker_no, fd_client);
                    result.code = MW_FATAL_ERROR;
                } else {
                    logger(
                        "[THREAD %d] [REMOVE_FILE_FAIL] Non-fatal error in REMOVE_FILE request from client %ld: %s.\n", 
                        worker_no, fd_client, thread_res_to_msg(res)
                    );
                    result.code = MW_NON_FATAL_ERROR;
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
    logger("[THREAD %d] Closing thread.\n", worker_no);

    close(pipe[W_ENDP]);
    pipe[W_ENDP] = -1;

    free(w_arg);
    return NULL;
}

static const char* thread_res_to_msg(int res){
    switch(res){
        case SA_SUCCESS:
            return "success";
        case SA_EXISTS: 
            return "file already exists";
        case SA_NO_FILE: 
            return "file doesn't exist";
        case SA_ALREADY_LOCKED:
            return "file is already locked";
        case SA_CLOSE:
            return "client closed connection";
        case SA_ERROR:
        default:
            return "general fatal error";
    }
}