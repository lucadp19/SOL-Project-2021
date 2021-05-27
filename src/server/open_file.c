#include "server.h"
#include "util/files.h"


static int create_file(file_t** file, char* pathname, long flags, long fd_client);

int open_file(long fd_client){
    int l;
    char* pathname = NULL;
    int pathname_len;
    int flags;

    if( (l = readn(fd_client, &pathname_len, sizeof(int))) == -1 ){
        return SA_ERROR;
    } if (l == 0) return SA_CLOSE;
    debug("pathname_len = %d\n", pathname_len);

    pathname = safe_calloc((pathname_len + 1), sizeof(char));

    if( (l = readn(fd_client, (void*)pathname, pathname_len+1)) == -1){
        free(pathname);
        return SA_ERROR;
    } if( l == 0 ) {
        free(pathname);
        return SA_CLOSE;
    }
    debug("pathname = <%s>\n", pathname);

    if( (l = readn(fd_client, &flags, sizeof(int))) == -1){
        free(pathname);
        return SA_ERROR;
    } if( l == 0 ) {
        free(pathname);
        return SA_CLOSE;
    }
    debug("flag = %d\n", flags);

    if(IS_FLAG_SET(flags, O_CREATE)){ // creating new file
        safe_pthread_mutex_lock(&files_mtx);

        // already contained
        if(hashmap_contains(files, pathname)){
            safe_pthread_mutex_unlock(&files_mtx);
            debug("File already exists! :(\n");
            free(pathname);
            return SA_EXISTS;
        }

        file_t* file;
        create_file(&file, pathname, flags, fd_client);
        
        file_t* expelled;
        if(curr_state.files == server_config.max_files){
            expell_LRU(&expelled);
        }
        // TODO: what should I do with this file?!

        hashmap_insert(&files, pathname, file);

        debug("File added to fs!\n");
        safe_pthread_mutex_unlock(&files_mtx);

        return SA_SUCCESS;     
    }

    // opening already created file
    safe_pthread_mutex_lock(&files_mtx);

    if(!hashmap_contains(files, pathname)){
        safe_pthread_mutex_unlock(&files_mtx);

        free(pathname);
        debug("File doesn't exist!\n");
        return SA_NO_FILE;
    } else debug("File exists!\n");

    file_t* file;
    // cannot return -1 because file exists
    hashmap_get_by_key(files, pathname, (void**)(&file));

    debug("File path: %s\n", file->path_name);
    safe_pthread_mutex_lock(&(file->file_mtx));

    if(IS_FLAG_SET(flags, O_LOCK)){ // want to lock
        if(file->fd_lock != -1){ // already locked
            safe_pthread_mutex_unlock(&(file->file_mtx));
            safe_pthread_mutex_unlock(&files_mtx);

            free(pathname);
            debug("File is already locked.\n");
            return SA_ALREADY_LOCKED;
        } else 
            file->fd_lock = fd_client;
    }
    // adding client to clients who have opened this file
    hashtbl_insert(&(file->fd_open), fd_client);
    // updating last use
    file->last_use = time(NULL);
    safe_pthread_mutex_unlock(&(file->file_mtx));
    safe_pthread_mutex_unlock(&files_mtx);

    free(pathname);
    return SA_SUCCESS;
}

static int create_file(file_t** file, char* pathname, long flags, long fd_client){
    *file = safe_calloc(1, sizeof(file_t));

    (*file)->contents = NULL;
    (*file)->path_name = pathname;
    (*file)->size = 0;

    if(IS_FLAG_SET(flags, O_LOCK))
        (*file)->fd_lock = fd_client;
    else (*file)->fd_lock = -1;

    hashtbl_init(&((*file)->fd_open), 4, default_hashtbl_hash);

    pthread_mutex_init(&((*file)->file_mtx), NULL);
    (*file)->last_use = time(NULL);

    return 0;
}