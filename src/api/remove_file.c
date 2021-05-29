#include "api.h"
#include "api/globals.h"

#include "server-api-protocol.h"

int removeFile(const char* pathname){
    if(fd_sock == -1){
        errno = ENOTCONN;
        return -1;
    }  

    RESET_LAST_OP;

    // want to remove file
    op_code_t op_code = REMOVE_FILE;
    if( writen(fd_sock, &op_code, sizeof(op_code_t)) == -1)
        return -1;

    int len = strlen(pathname);
    if( writen(fd_sock, &len, sizeof(int)) == -1)
        return -1;
    if( writen(fd_sock, (void*)pathname, len + 1) == -1)
        return -1;
    
    int l, res;
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        return -1;
    }

    errno = convert_res_to_errno(res);
    return (res == SA_SUCCESS ? 0 : -1);
}