#include "client.h"
#include "util/files.h"
#include <dirent.h>

#define GO_TO_NEXT do {                 \
        if(exp_dir == NULL)             \
            curr = curr->next;          \
        else curr = curr->next->next;   \
    } while(0)   

#define CHECK_IF_FATAL_ERR \
    if( errno == ENOTRECOVERABLE || errno == EBADE ) { \
        fprintf(stderr, "Fatal error in server-client communication. Aborting.\n"); \
        exit(EXIT_FAILURE); \
    }

static int rec_scan_dirs(const char* dirname, const char* exp_dir, int N);
static void clean_node_buf(node_t* node);

void execute_requests(){
    node_t* curr = request_q->head;
    struct timespec wait_time;
    set_timespec_from_msec(0, &wait_time);

    while(curr != NULL){
        // if it's a time, set it and go on
        if(curr->key[0] == 't'){
            if( config.print_to_stdout ) printf("\n> Setting waiting time to %ld msec (option -t).\n", (long)curr->data);
            set_timespec_from_msec((long)curr->data, &wait_time);
            curr = curr->next;
            continue;
        } else nanosleep(&wait_time, NULL); // else wait for the time interval
        
        switch(curr->key[0]){
            case 'w': {
                if( config.print_to_stdout )
                    printf("\n> Sending files to server (option -w).\n");
                char* exp_dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'D'){
                    exp_dir = (char*)curr->next->data;
                    if( config.print_to_stdout ) 
                        printf("\tWriting expelled files in folder %s (option -D was set).\n", exp_dir);
                } else 
                    if( config.print_to_stdout ) printf("\tDeleting expelled files (option -D was not set).\n");
                
                str_long_pair_t* arg = (str_long_pair_t*)curr->data;
                char* dir = arg->dir;
                long n_files = arg->n_files;
                if(n_files == 0)
                    n_files = -1;

                int res = rec_scan_dirs(dir, exp_dir, n_files);
                if( res >= 0 ){
                    if( config.print_to_stdout )
                        printf("> Sent %d files to server (option -w).\n", res);
                } else { // deal with error
                    if( config.print_to_stdout )
                        printf("> There was an error: code %d :(\n", res);
                }

                GO_TO_NEXT;
                break;
            }

            case 'W': {
                if( config.print_to_stdout )
                    printf("\n> Sending files to server (option -W).\n");
                char* exp_dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'D'){
                    exp_dir = (char*)curr->next->data;
                    if( config.print_to_stdout ) 
                        printf("\tWriting expelled files in folder %s (option -D was set).\n", exp_dir);
                } else 
                    if( config.print_to_stdout ) printf("\tDeleting expelled files (option -D was not set).\n");
                
                list_t* files = (list_t*)curr->data;
                node_t* file = files->head;

                while(file != NULL){
                    if( config.print_to_stdout ) printf("\tWriting file %s.\n", file->key);

                    // opening
                    if( openFile(file->key, O_CREATE | O_LOCK) == -1 ){
                        api_perror("\tOpen file in option -W");
                        CHECK_IF_FATAL_ERR;

                        fprintf(stderr, "\tWriting file %s failed. Going to next file.\n\n", file->key);
                        file = file->next;
                        continue;
                    }

                    // writing
                    if( writeFile(file->key, exp_dir) == -1 ) {
                        api_perror("\tWrite file in option -W");
                        CHECK_IF_FATAL_ERR;

                        if( config.print_to_stdout ) printf("\tWriting file %s failed. Going to next file.\n\n", file->key);
                        file = file->next;
                        continue;
                    }

                    // closing
                    if( closeFile(file->key) == -1 ) {
                        api_perror("\tClose file in option -W");
                        CHECK_IF_FATAL_ERR;

                        if( config.print_to_stdout ) printf("\tWriting file %s failed. Going to next file.\n\n", file->key);
                        file = file->next;
                        continue;
                    }

                    if( config.print_to_stdout )
                        printf("\tSuccessfully written file %s into server.", file->key);
                    
                    // writing no. bytes written
                    off_t size;
                    if( config.print_to_stdout ) {
                        if(get_file_size(file->key, &size) != -1)
                            printf(" %ld bytes were written.\n", size);
                        else printf(" Couldn't get the number of written bytes.\n");
                    }

                    file = file->next;
                }

                if( config.print_to_stdout )
                    printf("> Completed writing files into server.\n");
                GO_TO_NEXT;
                break;
            }

            case 'r': {
                if( config.print_to_stdout )
                    printf("\n> Reading files from server (option -r).\n");
                char* exp_dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'd'){
                    exp_dir = (char*)curr->next->data;
                    if( config.print_to_stdout )
                        printf("\tWriting read files in folder %s (option -d was set).\n", exp_dir);
                } else 
                    if( config.print_to_stdout ) printf("\tDeleting read files (option -d was not set).\n");
                
                list_t* files = (list_t*)curr->data;
                node_t* file = files->head;

                list_t* to_write;
                if((to_write = empty_list()) == NULL){
                    perror("Error in creating list");
                    fprintf(stderr, "Aborting.\n");
                    exit(EXIT_FAILURE);
                }

                while(file != NULL){
                    if( openFile(file->key, O_NOFLAG) == -1 ){
                        api_perror("\tOpen file in option -r");
                        CHECK_IF_FATAL_ERR;
                        
                        if( config.print_to_stdout ) printf("\tReading file %s failed. Going to next file.\n\n", file->key);
                        file = file->next;
                        continue;
                    }

                    size_n_buf_t* snb = safe_calloc(1, sizeof(size_n_buf_t));
                    if( readFile(file->key, &(snb->buf), &(snb->size)) == -1 ) {
                        api_perror("\tRead file in option -r");
                        CHECK_IF_FATAL_ERR;

                        if( config.print_to_stdout ) printf("\tReading file %s failed. Going to next file.\n\n", file->key);
                        file = file->next;
                        continue;
                    }
                    if( list_push_back(to_write, file->key, snb) == -1){
                        fprintf(stderr, "Error in adding node to list. Aborting.\n");
                        exit(EXIT_FAILURE);
                    }

                    if( closeFile(file->key) == -1 ) {
                        api_perror("\tClose file in option -r");
                        CHECK_IF_FATAL_ERR;

                        if( config.print_to_stdout ) printf("\tWriting file %s failed. Going to next file.\n\n", file->key);
                        file = file->next;
                        continue;
                    }

                    if( config.print_to_stdout ) printf("\tSuccessfully read file %s. Read %lu bytes.\n", file->key, snb->size);
                    file = file->next;
                }

                int err;
                if( (err = write_list_of_files_into_dir(to_write, exp_dir)) < to_write->nelem) { 
                    // I've written less files than I should have
                    if(err == -1){
                        perror("\tError in writing files into directory");
                    } else {
                        fprintf(
                            stderr, "\tWritten %d files into %s instead of %d: %d writes failed.\n", 
                            err, exp_dir, to_write->nelem, err-to_write->nelem
                        );
                    }
                } else {
                    if( config.print_to_stdout )
                        printf("> Completed reading files from server.\n");
                }

                list_delete(&to_write, clean_node_buf);
                GO_TO_NEXT;
                break;
            }

            case 'R': {
                if( config.print_to_stdout ) printf("\n> Reading N files from server (option -R).\n");
                char* exp_dir = NULL;
                if(curr->next != NULL && curr->next->key[0] == 'd'){
                    exp_dir = (char*)curr->next->data;
                    if( config.print_to_stdout )
                        printf("\tWriting read files in folder %s (option -d was set).\n", exp_dir);
                } else 
                    if( config.print_to_stdout ) printf("\tDeleting read files (option -d was not set).\n");
                

                long N = (long)curr->data;
                int n_files;
                if(N == 0) 
                    N = -1;
                if( (n_files = readNFiles(N, exp_dir)) == -1){
                    api_perror("\tError in readNFiles (option -R)");
                    CHECK_IF_FATAL_ERR;

                    if( config.print_to_stdout )
                        printf("> Reading N files failed.\n");
                } else {
                    if( config.print_to_stdout )
                        printf("> Read %d files from server.\n", n_files);
                }

                if( config.print_to_stdout ) printf("> Completed reading N files.\n");
                GO_TO_NEXT;
                break;   
            }

            case 'c': {
                if( config.print_to_stdout ) printf("\n> Removing files from server (option -c).\n");
                
                list_t* files = (list_t*)curr->data;
                node_t* file = files->head;

                while(file != NULL){
                    if( config.print_to_stdout ) printf("\tRemoving file %s.\n", file->key);
                    
                    if( openFile(file->key, O_LOCK) == -1){
                        api_perror("\tOpen file in option -c");
                        CHECK_IF_FATAL_ERR;

                        if( config.print_to_stdout ) printf("\tRemoving file %s failed. Going to next file.\n\n", file->key);
                        file = file->next;
                        continue;
                    }
                    if( removeFile(file->key) == -1){
                        api_perror("\tRemove file in option -c");
                        CHECK_IF_FATAL_ERR;
                        
                        if( config.print_to_stdout ) printf("\tRemoving file %s failed. Going to next file.\n\n", file->key);
                        file = file->next;
                        continue;
                    }

                    if( config.print_to_stdout ) printf("\tSuccessfully removed file %s from server.\n", file->key);
                    file = file->next;
                }

                if( config.print_to_stdout ) printf("> Completed removal of files from server.\n");
                curr = curr->next;
                break;
            }

            default:
                fprintf(stderr, "\n> Option -%c not implemented.\n", curr->key[0]);
                curr = curr->next;
                break;
        }
    }
}

static int rec_scan_dirs(const char* dirname, const char* exp_dir, int N){
    DIR* d;
    struct dirent* dir;

    if( config.print_to_stdout ) printf("\t> Opening directory %s.\n", dirname);
    if( (d = opendir(dirname)) == NULL){
        if( config.print_to_stdout ) printf("\t> Could not open directory %s: %s.\n", dirname, strerror(errno));
        return -1;
    }
    
    int n_reads = 0;
    int dir_len = strlen(dirname);
    errno = 0;
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

        if( config.print_to_stdout )
            printf("\tExamining file %s.\n", new_path);

        struct stat info;
        if( stat(new_path, &info) == -1){
            perror("\tError in stat");
            free(new_path);
            return -3;
        }

        if(S_ISDIR(info.st_mode)) {
            if( config.print_to_stdout ) printf("\tFile %s is a subdirectory: recursively getting files from subdirectory.\n\n", new_path);

            int res = rec_scan_dirs(new_path, exp_dir, (N == -1 ? -1 : N - n_reads) );
            if( res >= 0 ){ // read some files!
                n_reads += res;
                if( config.print_to_stdout ) printf("\tSuccessfully read %d files from subdirectory %s.\n", n_reads, new_path);
            } else { // do something with error
                if( config.print_to_stdout ) printf("\tCouldn't read files from subdirectory %s.\n", new_path);
            }
        } else { // found file
            if( config.print_to_stdout ) printf("\tSending file %s to server.\n", new_path);

            if( openFile(new_path, O_CREATE | O_LOCK) == -1){
                api_perror("\tError in open file");
                CHECK_IF_FATAL_ERR;

                if( config.print_to_stdout ) printf("\tCouldn't send file %s to server. Going to next file.\n\n", new_path);
                free(new_path);
                errno = 0;
                continue;
            }

            if( writeFile(new_path, exp_dir) == -1 ){
                api_perror("\tError in writing file");
                CHECK_IF_FATAL_ERR;

                if( config.print_to_stdout ) printf("\tCouldn't send file %s to server. Going to next file.\n\n", new_path);
                free(new_path);
                errno = 0;
                continue;
            }

            if( closeFile(new_path) == -1 ){
                api_perror("Error in closing file");
                CHECK_IF_FATAL_ERR;

                if( config.print_to_stdout ) printf("\tCouldn't send file %s to server. Going to next file.\n\n", new_path);
                free(new_path);
                errno = 0;
                continue;
            }

            if( config.print_to_stdout ) printf("\tSuccesfully written file %s to server.", new_path);

            // writing no. bytes written
            off_t size;
            if( config.print_to_stdout ) {
                if(get_file_size(new_path, &size) != -1)
                    printf(" %ld bytes were written.\n", size);
                else printf(" Couldn't get the number of written bytes.\n");
            }
            n_reads++;
        }
        free(new_path);
        printf("\tGoing to next file.\n\n");
        errno = 0;
    } if( errno != 0 ){
        perror("\t> Errror in getting files from directory");
        closedir(d);
        return -1;
    }

    
    if( config.print_to_stdout ) printf("\t> Completed work in directory %s.\n", dirname);
    closedir(d);
    return n_reads;
}

static void clean_node_buf(node_t* node){
    if(node == NULL) return;
    if(node->data != NULL) {
        size_n_buf_t* snb = (size_n_buf_t*)node->data;
        if(snb->buf != NULL) free(snb->buf);
        free(snb);
    }
    free(node);
}