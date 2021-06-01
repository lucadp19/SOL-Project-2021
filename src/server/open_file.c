#include "server.h"

static int create_file(file_t** file, char* pathname, long flags, long fd_client);

int open_file(int worker_no, long fd_client){
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
        
        file_t* expelled = NULL;
        safe_pthread_mutex_lock(&curr_state_mtx);
        debug("curr_state.files = %d, server_config.max_files = %d", curr_state.files, server_config.max_files);
        if(curr_state.files == server_config.max_files){
            expell_single_file(&expelled);
        }
        safe_pthread_mutex_unlock(&curr_state_mtx);
        file->can_be_expelled = true;

        hashmap_insert(&files, pathname, file);

        safe_pthread_mutex_lock(&curr_state_mtx);
        curr_state.files++;
        if(curr_state.files > curr_state.max_files) 
            curr_state.max_files = curr_state.files;
        logger("[STATS][CURRENT_FILES] %u files are currently stored.\n", curr_state.files);
        logger("[STATS][CURRENT_SPACE] %u bytes are currently occupied.\n", curr_state.space);
        safe_pthread_mutex_unlock(&curr_state_mtx);

        file_writer_unlock(file);
        safe_pthread_mutex_unlock(&files_mtx);
        debug("File added to fs!\n");

        // deleting the expelled file because there's nowhere to send it
        if(expelled != NULL) {
            logger(
                "[THREAD %d] [OPEN_FILE][THROWN_AWAY] The file %s is to be destroyed because expelled by the replacement algoritm because the maximum number of files was reached.\n",
                worker_no, expelled->path_name
            );
            file_delete(expelled);
        }
        // logging info
        if( IS_FLAG_SET(flags, O_LOCK)) {
            logger(
                "[THREAD %d] [OPEN_FILE_SUCCESS][LOCK][CREATE] Successfully created locked file \"%s\".\n", 
                worker_no, 
                pathname
            );
        } else {
            logger(
                "[THREAD %d] [OPEN_FILE_SUCCESS][CREATE] Successfully created file \"%s\".\n", 
                worker_no, 
                pathname
            );
        }

        return SA_SUCCESS;     
    }

    // opening already created file
    safe_pthread_mutex_lock(&files_mtx);

    file_t* file;
    if(hashmap_get_by_key(files, pathname, (void**)&file) == -1){
        if(errno == ENOENT) { // file doesn't exist
            safe_pthread_mutex_unlock(&files_mtx);
            free(pathname);
            return SA_NO_FILE;
        } else if (errno == EINVAL) { // files is NULL, abort
            fprintf(stderr, "Hashmap of files is NULL, aborting.\n");
            exit(EXIT_FAILURE);
        }
    } 
    
    debug("File path: %s\n", file->path_name);

    // locking file for writing operations
    file_writer_lock(file);

    if(IS_FLAG_SET(flags, O_LOCK)){ // want to lock
        if(file->fd_lock != -1 && file->fd_lock != fd_client){ // already locked by other client
            file_writer_unlock(file);
            safe_pthread_mutex_unlock(&files_mtx);

            free(pathname);
            debug("File is already locked.\n");
            return SA_ALREADY_LOCKED;
        } else 
            file->fd_lock = fd_client;
    }
    // adding client to clients who have opened this file
    if(!hashtbl_contains(file->fd_open, fd_client))
        hashtbl_insert(&(file->fd_open), fd_client);
    // no error if client has already opened it

    // updating last use
    if(clock_gettime(CLOCK_MONOTONIC, &(file->last_use)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.");
        exit(EXIT_FAILURE);
    }
    // file->last_use = time(NULL);

    // unlocking things
    file_writer_unlock(file);
    safe_pthread_mutex_unlock(&files_mtx);

    // logging info
    if( IS_FLAG_SET(flags, O_LOCK)) {
        logger(
            "[THREAD %d] [OPEN_FILE_SUCCESS][LOCK] Successfully opened locked file \"%s\".\n", 
            worker_no, 
            pathname
        );
    } else {
        logger(
            "[THREAD %d] [OPEN_FILE_SUCCESS] Successfully opened file \"%s\".\n", 
            worker_no, 
            pathname
        );
    }
    free(pathname);

    return SA_SUCCESS;
}

static int create_file(file_t** file, char* pathname, long flags, long fd_client){
    *file = safe_calloc(1, sizeof(file_t));

    // general attrs
    (*file)->contents = NULL;
    (*file)->path_name = pathname;
    (*file)->size = 0;
    (*file)->can_be_expelled = false;

    // setting lock
    if(IS_FLAG_SET(flags, O_LOCK))
        (*file)->fd_lock = fd_client;
    else (*file)->fd_lock = -1;

    // initializing list of openers
    // TODO: check errors
    hashtbl_init(&((*file)->fd_open), 4, default_hashtbl_hash);
    hashtbl_insert(&((*file)->fd_open), fd_client);

    // init mutex/cond
    pthread_mutex_init(&((*file)->file_mtx), NULL);
    pthread_mutex_init(&((*file)->order_mtx), NULL);
    pthread_cond_init(&((*file)->access_cond), NULL);
    (*file)->n_readers = 0;
    (*file)->n_writers = 0;

    // current thread gets mutex control
    file_writer_lock(*file);
    
    // updating time of creation
    if(clock_gettime(CLOCK_MONOTONIC, &((*file)->creation_time)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.");
        exit(EXIT_FAILURE);
    }
    // updating time of last use
    if(clock_gettime(CLOCK_MONOTONIC, &((*file)->last_use)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.");
        exit(EXIT_FAILURE);
    }

    return 0;
}