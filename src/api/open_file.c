#include "api.h"
#include "api/globals.h"
#include "server-api-protocol.h"

int openFile(const char* pathname, int flags){
    debug("Writing to server! fd_sock = %ld\n", fd_sock);

    op_code_t op_code = OPEN_FILE;
    if( writen(fd_sock, (void*)&op_code, sizeof(op_code_t)) == -1)
        return -1;
    

    int len = strlen(pathname);
    if( writen(fd_sock, &len, sizeof(int)) == -1)
        return -1;
    if( writen(fd_sock, (void*)pathname, len + 1) == -1)
        return -1;
    if( writen(fd_sock, &flags, sizeof(int)) == -1)
        return -1;

    return 0;
}