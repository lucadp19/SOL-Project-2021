#include "api.h"
#include "api/globals.h"
#include "server-api-protocol.h"

int closeFile(const char* pathname){
    debug(">> Closing file!\n");
    // want to close file
    op_code_t op_code = CLOSE_FILE;
    if( writen(fd_sock, (void*)&op_code, sizeof(op_code_t)) == -1)
        return -1;
    
    // writing parameters
    int len = strlen(pathname);
    if( writen(fd_sock, &len, sizeof(int)) == -1)
        return -1;
    if( writen(fd_sock, (void*)pathname, len+1) == -1)
        return -1;
    
    // reading answer
    int err;
    int l;
    if( (l = readn(fd_sock, &err, sizeof(int))) == -1 || l == 0){
        // TODO: EBADF because bad communication ?
        errno = EBADF;
        return -1;
    } 

    if(err == -1){
        // TODO: general error -> which errno should I choose? 
        errno = EBADE;
        return -1;
    } else if(err != 0){
        errno = err;
        return -1;
    }

    // everything worked out fine :D
    errno = 0;
    return 0;
}