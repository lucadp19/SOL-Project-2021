#include "api.h"
#include "api/globals.h"

#include "server-api-protocol.h"

int readFile(const char* pathname, void** buf, size_t* size){
    debug(">> Writing file!\n");
    if(fd_sock == -1){
        errno = ENOTCONN;
        return -1;
    }

    RESET_LAST_OP;
    int len = strlen(pathname);
    op_code_t op_code = READ_FILE;

    // sending request to server
    if( writen(fd_sock, &op_code, sizeof(op_code_t)) == -1){
        errno = EBADE;
        return -1;
    }
    if( writen(fd_sock, &len, sizeof(int)) == -1){
        errno = EBADE;
        return -1;
    }
    if( writen(fd_sock, (void*)pathname, (len+1)*sizeof(char)) == -1){
        errno = EBADE;
        return -1;
    }
    
    // reading success/failure and file
    int l;
    int res;
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        errno = EBADE;
        return -1;
    }
    if( res != SA_SUCCESS ){
        errno = convert_res_to_errno(res);
        return -1;
    }
    // can read file
    if( (l = readn(fd_sock, &(*size), sizeof(size_t))) == -1 || l == 0){
        errno = EBADE;
        return -1;
    }
    debug("*size = %lu\n", *size);
    *buf = safe_malloc(*size);
    if( (l = readn(fd_sock, *buf, *size)) == -1 || l == 0){
        free(*buf);
        errno = EBADE;
        return -1;
    }

    // reading final result
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        errno = EBADE;
        return -1;
    }
    errno = convert_res_to_errno(res);
    return (res == SA_SUCCESS ? 0 : -1);
}
