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

    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }
    
    // want to open file
    op_code_t op_code = OPEN_FILE;
    debug("Writing opcode\n");
    if( writen(fd_sock, (void*)&op_code, sizeof(op_code_t)) == -1){
        errno = EBADE;
        return -1;
    }

    // writing parameters
    int len = strlen(pathname);
    debug("Writing len\n");
    if( writen(fd_sock, &len, sizeof(int)) == -1){ 
        errno = EBADE;
        return -1;
    }
    debug("Writing pathname");
    if( writen(fd_sock, (void*)pathname, len + 1) == -1){ 
        errno = EBADE;
        return -1;
    }
    debug("Writing flags");
    if( writen(fd_sock, &flags, sizeof(int)) == -1){ 
        errno = EBADE;
        return -1;
    }
    
    last_op.is_open = true;
    last_op.success = false;
    last_op.create  = IS_FLAG_SET(flags, O_CREATE);
    last_op.lock    = IS_FLAG_SET(flags, O_LOCK);
    last_op.path    = safe_calloc(len+1, sizeof(char));
    strncpy(last_op.path, pathname, len+1);

    // reading answer
    int res;
    int l;
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        errno = EBADE;
        return -1;
    } 
    
    if(res != SA_SUCCESS) {
       last_op.success = false;
       errno = convert_res_to_errno(res);
       return -1;
    }
    
    // everything worked out fine :D
    last_op.success = true;
    errno = 0;
    return 0;
}