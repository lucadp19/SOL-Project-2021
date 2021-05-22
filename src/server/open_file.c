#include "server.h"
#include "util/files.h"


static int create_file(file_t** file, char* pathname, long flags, long fd_client);

int open_file(long fd_client){
    int l;
    char* pathname = NULL;
    int pathname_len;
    int flags;

    if( (l = readn(fd_client, &pathname_len, sizeof(int))) == -1 ){
        return -1;
    } if (l == 0) return -2;
    debug("pathname_len = %d\n", pathname_len);

    if( (pathname = calloc((pathname_len + 1), sizeof(char))) == NULL){
        return -3;
    }

    if( (l = readn(fd_client, (void*)pathname, pathname_len+1)) == -1){
        free(pathname);
        return -1;
    } if( l == 0 ) {
        free(pathname);
        return -2;
    }
    debug("pathname = <%s>\n", pathname);

    if( (l = readn(fd_client, &flags, sizeof(int))) == -1){
        free(pathname);
        return -1;
    } if( l == 0 ) {
        free(pathname);
        return -2;
    }
    debug("flag = %d\n", flags);

    if(IS_FLAG_SET(flags, O_CREATE)){
        safe_pthread_mutex_lock(&files_mtx);
        if(hashmap_contains(files, pathname)){
            safe_pthread_mutex_unlock(&files_mtx);
            debug("Already contained!\n");
            free(pathname);
            return -2;
        }
        safe_pthread_mutex_unlock(&files_mtx);

        file_t* file;
        create_file(&file, pathname, flags, fd_client);
        add_file_to_fs(file);
        debug("File added to fs!\n");
    } else {
        free(pathname);
    }

    return 0;
}

static int create_file(file_t** file, char* pathname, long flags, long fd_client){
    *file = calloc(1, sizeof(file_t));

    (*file)->contents = NULL;
    (*file)->path_name = pathname;
    if(IS_FLAG_SET(flags, O_LOCK))
        (*file)->fd_lock = fd_client;
    else (*file)->fd_lock = -1;
    pthread_mutex_init(&((*file)->file_mtx), NULL);
    (*file)->last_use = time(NULL);

    return 0;
}