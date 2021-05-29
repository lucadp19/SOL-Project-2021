#include "api.h"
#include "api/globals.h"

#include "server-api-protocol.h"

int writeFile(const char* pathname, const char* dirname){
    debug(">> Writing file!\n");
    if(fd_sock == -1){
        errno = ENOTCONN;
        return -1;
    }

    int len = strlen(pathname);
    if(!last_op.is_open || !last_op.create || !last_op.lock || !last_op.success || strncmp(last_op.path, pathname, len) ){
        // invalid request!
        errno = EINVAL;
        return -1;
    }

    // reset last operation
    RESET_LAST_OP;

    debug("Reading file from disk...\n");
    // ----- READING FILE ------ //
    FILE* file;
    if( (file = fopen(pathname, "rb")) == NULL ){
        return -1;
    }

    // getting file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    if(file_size == -1){
        fclose(file);
        return -1;
    }

    void* buffer = safe_malloc(file_size);
    if (fread(buffer, 1, file_size, file) < file_size){
        if(ferror(file)){
            free(buffer);
            fclose(file);
            return -1;
        }
    }
    fclose(file);
    
    debug("File read! Now sending it to server...\n");
    // ----- WRITING TO SERVER ----- //
    op_code_t op_code = WRITE_FILE;
    // writing op_code
    if( writen(fd_sock, &op_code, sizeof(op_code_t)) == -1) {
        free(buffer);
        return -1;
    }
    // writing pathname
    if( writen(fd_sock, &len, sizeof(int)) == -1) {
        free(buffer);
        return -1;
    }
    if( writen(fd_sock, (void*)pathname, (len+1)*sizeof(char)) == -1) {
        free(buffer);
        return -1;
    }
    // writing file
    if( writen(fd_sock, &file_size, sizeof(long)) == -1) {
        free(buffer);
        return -1;
    }
    if( writen(fd_sock, buffer, file_size) == -1) {
        free(buffer);
        return -1;
    }

    free(buffer);

    debug("File sent! Reading result from server...\n");
    // ---- READING RESULT FROM SERVER ---- //

    // reading result before possible file expulsion
    int res, l;
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        // TODO: EBADF because bad communication ?
        errno = EBADF;
        return -1;
    }
    if(res != SA_SUCCESS){
        errno = convert_res_to_errno(res);
        return -1;
    }

    // reading and writing expelled files
    if( write_expelled_files(dirname) == -1)
        return -1;

    // reading final result
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        // TODO: EBADF because bad communication ?
        errno = EBADF;
        return -1;
    }

    errno = convert_res_to_errno(res);
    if(res != 0) return -1;
    return 0;
}
