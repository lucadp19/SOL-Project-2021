#include "api.h"
#include "api/globals.h"

int closeConnection(const char* sockname){
    debug("sockname = %s, socket_path = %s", sockname, socket_path);

    if(sockname == NULL){
        errno = EINVAL;
        return -1;
    }

    // wrong socket name
    if(strcmp(sockname, socket_path) != 0){
        // this socket is not connected
        errno = ENOTCONN;
        return -1;
    }   

    if( close(fd_sock) == -1 ){
        RESET_SOCK;
        return -1;
    }

    debug("Closed connection!\n");
        
    RESET_SOCK;
    return 0;    
}