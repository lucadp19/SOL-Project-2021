#include "server.h"

int send_single_file(int worker_no, long fd_client, file_t* file){
    if(file == NULL) return -1;

    int path_len = strlen(file->path_name);

    if( writen(fd_client, &path_len, sizeof(int)) == -1)
        return -1;
    if( writen(fd_client, (void*)file->path_name, (path_len+1) * sizeof(char)) == -1)
        return -1;
    if( writen(fd_client, &file->size, sizeof(size_t)) == -1)
        return -1;
    if( file->size != 0 && writen(fd_client, file->contents, file->size) == -1)
        return -1;
    
    return 0;
}

int send_list_of_files(int worker_no, long fd_client, list_t* files){
    if(files == NULL) return -1;

    node_t* curr = files->head;

    while(curr != NULL){
        file_t* curr_file = (file_t*)curr->data;
        if( send_single_file(worker_no, fd_client, curr_file) == -1 )
            return -1;

        curr = curr->next;
    }

    // writing 0 to signal end of list
    int terminating_zero = 0;
    if( writen(fd_client, &terminating_zero, sizeof(int)) == -1)
        return -1;
    return 0;
}