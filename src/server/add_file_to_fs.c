#include "server.h"

int add_file_to_fs(file_t* file){
    if(file == NULL) return -1;

    safe_pthread_mutex_lock(&files_mtx);
    hashmap_insert(&files, file->path_name, file);
    safe_pthread_mutex_unlock(&files_mtx);

    return 0;
}