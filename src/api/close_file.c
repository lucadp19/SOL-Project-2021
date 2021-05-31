#include "api.h"
#include "api/globals.h"
#include "server-api-protocol.h"

int closeFile(const char* pathname){
    debug(">> Closing file!\n");
    if(fd_sock == -1){
        errno = ENOTCONN;
        return -1;
    }
    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }
    // last operation isn't an open file anymore
    RESET_LAST_OP;

    // want to close file
    op_code_t op_code = CLOSE_FILE;
    if( writen(fd_sock, (void*)&op_code, sizeof(op_code_t)) == -1){
        errno = EBADE;
        return -1;
    }
    
    // writing parameters
    int len = strlen(pathname);
    if( writen(fd_sock, &len, sizeof(int)) == -1){
        errno = EBADE;
        return -1;
    }
    if( writen(fd_sock, (void*)pathname, len+1) == -1){
        errno = EBADE;
        return -1;
    }
    
    // reading answer
    int res;
    int l;
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        errno = EBADE;
        return -1;
    } 
    
    errno = convert_res_to_errno(res);
    return (res == SA_SUCCESS ? 0 : -1);
}