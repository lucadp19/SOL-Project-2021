#include "api.h"
#include "api/globals.h"

#include "util/list.h"

#include "server-api-protocol.h"

static const char* remove_path_from_name(const char* path){
    if(path == NULL) {
        return NULL;
    }

    // if there are no / in path, the whole path is the name
    const char* name;
    if( (name = strrchr(path, '/')) == NULL )
        name = path;
    return name;
}

// Taken from https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
static int mkdir_p(const char *path) {
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[PATH_MAX];
    char *p; 

    errno = 0;

    /* Copy string so its mutable */
    if (len > sizeof(_path)-1) {
        errno = ENAMETOOLONG;
        return -1; 
    }   
    strcpy(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';

            if (mkdir(_path, S_IRWXU) != 0) {
                if (errno != EEXIST)
                    return -1; 
            }

            *p = '/';
        }
    }   

    if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
            return -1; 
    }   

    return 0;
}

static void custom_free_funct(node_t* node){
    if(node == NULL) return;

    size_and_buf_t* data = node->data;
    free(data->buf);
    free(data);
    free((void*)node->key);
    free(node);
}

int write_expelled_files(const char* dirname){
    // ---- READING FILES FROM SERVER ---- //
    list_t* files;
    if( (files = empty_list()) == NULL) return -1;

    while(true){
        int l;
        int path_len;
        char* path;
        size_and_buf_t* file;

        debug("reading pathlen\n");
        if( (l = readn(fd_sock, &path_len, sizeof(int))) == -1 || l == 0){
            debug("l = %d\n", l);
            list_delete(&files, custom_free_funct);
            return -1;
        }

        // ended list
        debug("pathlen is zero!");
        if( path_len == 0 ) break;

        file = safe_calloc(1, sizeof(size_and_buf_t));

        // must read another file
        path = safe_calloc(path_len + 1, sizeof(char));

        if( (l = readn(fd_sock, path, path_len + 1)) == -1 || l == 0) {
            list_delete(&files, custom_free_funct);
            free((void*)path);
            free(file);
            return -1;
        }

        if( (l = readn(fd_sock, &(file->size), sizeof(int))) == -1 || l == 0) {
            list_delete(&files, custom_free_funct);
            free((void*)path);
            free(file);
            return -1;
        }

        // empty file => no content
        if(file->size == 0) {
            file->buf = safe_malloc(file->size);

            if( (l = readn(fd_sock, file->buf, file->size)) == -1 || l == 0) {
                list_delete(&files, custom_free_funct);
                free((void*)path);
                free(file->buf);
                free(file);
                return -1;
            }
        }

        if( list_push_back(files, path, (void*)file) == -1) {
            list_delete(&files, custom_free_funct);
            free((void*)path);
            free(file);
            return -1;
        }
    }

    // ----- CREATING DIRECTORY ----- //
    if( mkdir_p(dirname) == -1){
        list_delete(&files, custom_free_funct);
        return -1;
    }

    // ----- WRITING FILE IN DIRECTORY ----- //
    node_t* curr = files->head;
    int path_to_write_len = -1;
    char* path_to_write = NULL;

    while(curr != NULL){
        const char* base_name = remove_path_from_name(curr->key);

        // creating new path name
        int old_len = path_to_write_len;
        path_to_write_len = strlen(base_name) + strlen(dirname) + 2;
        if(old_len < path_to_write_len)
            path_to_write = safe_realloc(path_to_write, path_to_write_len * sizeof(char));
        snprintf(path_to_write, path_to_write_len, "%s/%s", dirname, base_name);

        FILE* to_write;
        if( (to_write = fopen(path_to_write, "wb")) == NULL){
            list_delete(&files, custom_free_funct);
            free(path_to_write);
            return -1;
        }

        size_and_buf_t* file = curr->data;
        if( file->size > 0 && fwrite(file->buf, 1, file->size, to_write) < file->size){
            fclose(to_write);
            list_delete(&files, custom_free_funct);
            free(path_to_write);
            return -1;
        }

        fclose(to_write);
    }
    
    if(path_to_write != NULL) free(path_to_write);
    list_delete(&files, custom_free_funct);
    debug("still no error\n");
    
    return 0;
}