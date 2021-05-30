#include "api.h"
#include "api/globals.h"

#include "server-api-protocol.h"

int readNFiles(int N, const char* dirname){
    debug(">> Read N files!\n");
    if(fd_sock == -1){
        errno = ENOTCONN;
        return -1;
    }

    RESET_LAST_OP;
    op_code_t op_code = READ_N_FILES;

    // sending request to server
    if( writen(fd_sock, &op_code, sizeof(op_code_t)) == -1){
        errno = EBADE;
        return -1;
    }
    if( writen(fd_sock, &N, sizeof(int)) == -1){
        errno = EBADE;
        return -1;
    }
    
    // reading success/failure
    int l, res;
    if( (l = readn(fd_sock, &res, sizeof(int))) == -1 || l == 0){
        errno = EBADE;
        return -1;
    } if( res != SA_SUCCESS ){
        errno = convert_res_to_errno(res);
        return -1;    
    }

    // can read files
    if( write_files_sent_by_server(dirname) == -1){
        return -1;
    }

    return 0;
}
