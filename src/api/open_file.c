#include "api.h"
#include "api/globals.h"

#include "util/files.h"
#include "server-api-protocol.h"

int openFile(const char* pathname, int flags){
    debug(">> Opening file!\n");
    if(fd_sock == -1){
        errno = ENOTCONN;
        return -1;
    } 
    
    // want to open file
    op_code_t op_code = OPEN_FILE;
    if( writen(fd_sock, (void*)&op_code, sizeof(op_code_t)) == -1)
        return -1;

    // writing parameters
    int len = strlen(pathname);
    if( writen(fd_sock, &len, sizeof(int)) == -1)
        return -1;
    if( writen(fd_sock, (void*)pathname, len + 1) == -1)
        return -1;
    if( writen(fd_sock, &flags, sizeof(int)) == -1)
        return -1;
    
    last_op.is_open = true;
    last_op.success = false;
    last_op.create  = IS_FLAG_SET(flags, O_CREATE);
    last_op.lock    = IS_FLAG_SET(flags, O_LOCK);
    last_op.path    = safe_calloc(len+1, sizeof(char));
    strncpy(last_op.path, pathname, len+1);

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
    last_op.success = true;
    errno = 0;
    return 0;
}