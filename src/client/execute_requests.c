#include "client.h"
#include "util/files.h"
#include <dirent.h>

#define GO_TO_NEXT do {                 \
        if(exp_dir == NULL)             \
            curr = curr->next;          \
        else curr = curr->next->next;   \
    } while(0)   

static int rec_scan_dirs(const char* dirname, const char* exp_dir, int N);

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
            case 'w': {
                if(p_option)
                    printf("Sending files to server (option -w).\n");
                char* exp_dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'D'){
                    exp_dir = (char*)curr->next->data;
                    if(p_option) 
                        printf("Writing expelled files in folder %s (option -D was set).\n", exp_dir);
                } else 
                    if(p_option) printf("Deleting expelled files (option -D was not set).\n");
                
                str_long_pair_t* arg = (str_long_pair_t*)curr->data;
                char* dir = arg->dir;
                long n_files = arg->n_files;
                if(n_files == 0)
                    n_files = -1;

                int res = rec_scan_dirs(dir, exp_dir, n_files);
                if( res >= 0 ){
                    if(p_option)
                        printf("Sent %d files to server (option -w).", res);
                } else { // deal with error
                    if(p_option)
                        printf("There was an error: code %d :(\n", res);
                }

                GO_TO_NEXT;
                break;
            }

            case 'W': {
                if(p_option)
                    printf("Sending files to server (option -W).\n");
                char* exp_dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'D'){
                    exp_dir = (char*)curr->next->data;
                    if(p_option) 
                        printf("Writing expelled files in folder %s (option -D was set).\n", exp_dir);
                } else 
                    if(p_option) printf("Deleting expelled files (option -D was not set).\n");
                
                list_t* files = (list_t*)curr->data;
                node_t* file = files->head;
                while(file != NULL){
                    if( openFile(file->key, O_CREATE | O_LOCK) == -1 ){
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Open file in option -w");
                            GO_TO_NEXT;
                            continue;
                        }
                    }
                    if( writeFile(file->key, exp_dir) == -1 ) {
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Write file in option -w");
                            GO_TO_NEXT;
                            continue;
                        }
                    }
                    if( closeFile(file->key) == -1 ) {
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Close file in option -w");
                             GO_TO_NEXT;
                            continue;
                        }
                    }
                    file = file->next;
                }

                GO_TO_NEXT;
                break;
            }

            case 'r': {
                if(p_option)
                    printf("Reading files from server (option -r).\n");
                char* exp_dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'd'){
                    exp_dir = (char*)curr->next->data;
                    if(p_option)
                        printf("Writing read files in folder %s (option -d was set).\n", exp_dir);
                } else 
                    if(p_option) printf("Deleting read files (option -d was not set).\n");
                
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
                            GO_TO_NEXT;
                            continue;
                        }
                    }

                    size_n_buf_t* snb = safe_calloc(1, sizeof(size_n_buf_t));
                    if( readFile(file->key, &(snb->buf), &(snb->size)) == -1 ) {
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Write file in option -w");
                            GO_TO_NEXT;
                            continue;
                        }
                    }
                    list_push_back(to_write, file->key, snb);

                    if( closeFile(file->key) == -1 ) {
                        if(errno == ENOTRECOVERABLE) return -1;
                        else {
                            perror("Close file in option -w");
                            GO_TO_NEXT;
                            continue;
                        }
                    }
                    file = file->next;
                }

                write_list_of_files_into_dir(to_write, exp_dir);

                GO_TO_NEXT;
                break;
            }

            case 'R': {
                if(p_option) printf("Reading N files from server (option -R).\n");
                char* exp_dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'd'){
                    exp_dir = (char*)curr->next->data;
                    if(p_option)
                        printf("Writing read files in folder %s (option -d was set).\n", exp_dir);
                } else 
                    if(p_option) printf("Deleting read files (option -d was not set).\n");
                

                long N = (long)curr->data;
                if(N == 0) 
                    N = -1;
                if( readNFiles(N, exp_dir) == -1){
                    perror("Error in readNFiles");
                    GO_TO_NEXT;
                    continue;
                }

                GO_TO_NEXT;
                break;   
            }

            case 'c': {
                if(p_option) printf("Removing file from server (option -c).\n");
                
                list_t* files = (list_t*)curr->data;
                node_t* file = files->head;

                while(file != NULL){
                    if( openFile(file->key, O_LOCK) == -1){
                        perror("Error in openFile");
                        file = file->next;
                        continue;
                    }
                    if( removeFile(file->key) == -1){
                        perror("Error in removeFile");
                        file = file->next;
                        continue;
                    }
                    file = file->next;
                }

                curr = curr->next;
                break;
            }

            default:
                fprintf(stderr, "Option not implemented.\n");
                curr = curr->next;
                break;
        }
    }
    return 0;
}

static int rec_scan_dirs(const char* dirname, const char* exp_dir, int N){
    DIR* d;
    struct dirent* dir;

    if( (d = opendir(dirname)) == NULL)
        return -2;
    
    int n_reads = 0;
    int dir_len = strlen(dirname);
    errno = 0;
    printf("N = %d, n_reads = %d\n", N, n_reads);
    while( (N == -1 || n_reads < N) && (dir = readdir(d)) != NULL ){
        // skipping . and ..
        if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0){
            errno = 0;
            continue;
        }

        // creating new path: "old_dir_path/new_file"
        int new_len = dir_len + strlen(dir->d_name) + 1; // +1 because of the /
        char* new_path = safe_calloc(new_len + 1, sizeof(char));
        snprintf(new_path, new_len + 1, "%s/%s", dirname, dir->d_name);

        struct stat info;
        if( stat(new_path, &info) == -1){
            perror("Error in stat");
            free(new_path);
            return -1;
        }

        if(S_ISDIR(info.st_mode)) {
            if(p_option) printf("Recursively getting files from subdirectory %s.\n", new_path);

            int res = rec_scan_dirs(new_path, exp_dir, (N == -1 ? -1 : N - n_reads) );
            if( res >= 0 ){ // read some files!
                n_reads += res;
            } else { // do something with error
                switch(res){
                    case -2:
                        perror("Could not open subdirectory");
                        free(new_path);
                        errno = 0;
                        break;
                    default: break;
                }
            }
        } else { // found file
            if(p_option) printf("Sending file %s to server.\n", new_path);

            if( openFile(new_path, O_CREATE | O_LOCK) == -1){
                perror("Error in open file");
                free(new_path);
                errno = 0;
                continue;
            }
            if( writeFile(new_path, exp_dir) == -1 ){
                perror("Error in writing file");
                free(new_path);
                errno = 0;
                continue;
            }
            if( closeFile(new_path) == -1 ){
                perror("Error in closing file");
                free(new_path);
                errno = 0;
                continue;
            }

            n_reads++;
        }
        errno = 0;
    } if( errno != 0 ){
        closedir(d);
        return -1;
    }

    closedir(d);
    return n_reads;
}