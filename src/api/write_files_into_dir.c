#include "api.h"
#include "api/globals.h"

#include "util/list.h"
#include "util/files.h"

#include "server-api-protocol.h"


int write_files_sent_by_server(const char* dirname){
    // ---- READING FILES FROM SERVER ---- //
    list_t* files;
    if( (files = empty_list()) == NULL) return -1;
    int l;

    while(true){
        int path_len;
        char* path;
        size_n_buf_t* file;

        if( (l = readn(fd_sock, &path_len, sizeof(int))) == -1 || l == 0){
            list_delete(&files, free_node_size_n_buf);
            return -1;
        }

        // ended list
        if( path_len == 0 ) break;

        // must read another file
        file = safe_calloc(1, sizeof(size_n_buf_t));
        path = safe_calloc(path_len + 1, sizeof(char));

        if( (l = readn(fd_sock, path, path_len + 1)) == -1 || l == 0) {
            list_delete(&files, free_node_size_n_buf);
            free((void*)path);
            free(file);
            return -1;
        }

        if( (l = readn(fd_sock, &(file->size), sizeof(long))) == -1 || l == 0) {
            list_delete(&files, free_node_size_n_buf);
            free((void*)path);
            free(file);
            return -1;
        }

        // empty file => no content
        if(file->size != 0) {
            file->buf = safe_malloc(file->size);

            if( (l = readn(fd_sock, file->buf, file->size)) == -1 || l == 0) {
                list_delete(&files, free_node_size_n_buf);
                free((void*)path);
                free(file->buf);
                free(file);
                return -1;
            }
        }

        if( list_push_back(files, path, (void*)file) == -1) {
            list_delete(&files, free_node_size_n_buf);
            free((void*)path);
            free(file);
            return -1;
        }
    }

    // ----- READING FINAL ANSWER FROM SERVER ----- //
    int res;
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        errno = EBADF;
        return -1;
    } if (res != SA_SUCCESS){
        errno = convert_res_to_errno(res);
        return -1;
    }

    
    if( write_list_of_files_into_dir(files, dirname) == -1 ) {
        list_delete(&files, free_node_size_n_buf);
        return -1;
    }
    
    list_delete(&files, free_node_size_n_buf);
    return 0;
}