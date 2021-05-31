#include "api.h"
#include "api/globals.h"

#include "server-api-protocol.h"

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname){
    if(fd_sock == -1){
        errno = ENOTCONN;
        return -1;
    }  
    if(pathname == NULL || buf == NULL){
        errno = EINVAL;
        return -1;
    }

    int l, res;

    RESET_LAST_OP;
    
    op_code_t op_code = APPEND_TO_FILE;
    if( writen(fd_sock, (void*)&op_code, sizeof(op_code_t)) == -1){
        errno = EBADE;
        return -1;
    }

    int len = strlen(pathname);
    if( writen(fd_sock, &len, sizeof(int)) == -1){
        errno = EBADE;
        return -1;
    }
    if( writen(fd_sock, (void*)pathname, len + 1) == -1){
        errno = EBADE;
        return -1;
    }
    if( writen(fd_sock, &size, sizeof(size_t)) == -1){
        errno = EBADE;
        return -1;
    }
    // if size == 0 nothing is to be written
    if( size > 0 && writen(fd_sock, buf, size) == -1){
        errno = EBADE;
        return -1;
    }

    // reading partial result
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        errno = EBADE;
        return -1;
    }
    if( res != SA_SUCCESS ){
        errno = convert_res_to_errno(res);
        return -1;
    }    

    return write_files_sent_by_server(dirname);
}