#include "util/files.h"

int write_list_of_files_into_dir(list_t* files, const char* dirname){
    if(dirname == NULL) return 0;

    // ----- CREATING DIRECTORY ----- //
    if( mkdir_p(dirname) == -1){
        list_delete(&files, free_node_size_n_buf);
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
        if(old_len < path_to_write_len) // reallocate memory
            path_to_write = safe_realloc(path_to_write, path_to_write_len * sizeof(char));
        snprintf(path_to_write, path_to_write_len, "%s/%s", dirname, base_name);

        FILE* to_write;
        if( (to_write = fopen(path_to_write, "wb")) == NULL){ 
            // free(path_to_write);
            // return -1;
            continue;
        }

        size_n_buf_t* file = curr->data;
        int l;
        if( file->size > 0 && (l = fwrite(file->buf, 1, file->size, to_write)) < file->size){
            fclose(to_write);
            // free(path_to_write);
            // return -1;
            continue;
        }

        fclose(to_write);
        curr = curr->next;
    }
    
    if(path_to_write != NULL) free(path_to_write);
    
    return 0;
}

void free_node_size_n_buf(node_t* node){
    if(node == NULL) return;

    size_n_buf_t* snb = (size_n_buf_t*)node->data; 
    if(snb != NULL){
        if(snb->buf != NULL) free(snb->buf);
        free(snb);
    }
    if(node->key != NULL) free((void*)node->key);
    free(node);
}

const char* remove_path_from_name(const char* path){
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
int mkdir_p(const char *path) {
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