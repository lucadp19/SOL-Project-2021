#include "client.h"
#include "util/files.h"

int execute_requests(){
    node_t* curr = request_q->head;
    struct timespec wait_time;
    set_timespec_from_msec(0, &wait_time);

    while(curr != NULL){
        // if it's a time, set it and go on
        if(curr->key[0] == 't'){
            if(p_option) printf("Setting waiting time (option -t).\n");
            set_timespec_from_msec((long)curr->data, &wait_time);
            curr = curr->next;
            continue;
        } else nanosleep(&wait_time, NULL); // else wait for the time interval
        
        switch(curr->key[0]){
            case 'W': {
                if(p_option)
                    printf("Sending files to server (option -W).\n");
                char* dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'D'){
                    dir = (char*)curr->next->data;
                    if(p_option) 
                        printf("Writing expelled files in folder %s (option -D was set).\n", dir);
                } else 
                    if(p_option) printf("Deleting expelled files (option -D was not set).\n");
                
                list_t* files = (list_t*)curr->data;
                node_t* file = files->head;
                while(file != NULL){
                    if( openFile(file->key, O_CREATE | O_LOCK) == -1 ){
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Open file in option -w");
                            continue;
                        }
                    }
                    if( writeFile(file->key, dir) == -1 ) {
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Write file in option -w");
                            continue;
                        }
                    }
                    if( closeFile(file->key) == -1 ) {
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Close file in option -w");
                            continue;
                        }
                    }
                    file = file->next;
                }

                if(dir == NULL) curr = curr->next;
                else curr = curr->next->next;
            }

            case 'r': {
                if(p_option)
                    printf("Reading files from server (option -r).\n");
                char* dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'd'){
                    dir = (char*)curr->next->data;
                    if(p_option)
                        printf("Writing read files in folder %s (option -r was set).\n", dir);
                } else 
                    if(p_option) printf("Deleting read files (option -r was not set).\n");
                
                list_t* files = (list_t*)curr->data;
                node_t* file = files->head;

                list_t* to_write;
                if((to_write = empty_list()) == NULL){
                    perror("Error in creating list");
                    return -1;
                }

                while(file != NULL){
                    if( openFile(file->key, O_NOFLAG) == -1 ){
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Open file in option -w");
                            continue;
                        }
                    }

                    size_n_buf_t* snb = safe_calloc(1, sizeof(size_n_buf_t));
                    if( readFile(file->key, &(snb->buf), &(snb->size)) == -1 ) {
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Write file in option -w");
                            continue;
                        }
                    }
                    list_push_back(to_write, file->key, snb);

                    if( closeFile(file->key) == -1 ) {
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Close file in option -w");
                            continue;
                        }
                    }
                    file = file->next;
                }

                write_list_of_files_into_dir(to_write, dir);

                if(dir == NULL) curr = curr->next;
                else curr = curr->next->next;
            }

            default:
                fprintf(stderr, "Option not implemented :D\n");
                curr = curr->next;
                break;
        }
    }
    return 0;
}