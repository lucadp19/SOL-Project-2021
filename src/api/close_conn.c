#include "api.h"
#include "api/globals.h"

int closeConnection(const char* sockname){
    debug("sockname = %s, socket_path = %s", sockname, socket_path);

    // wrong socket name
    if(sockname != socket_path){
        // this socket is not connected
        // TODO: maybe I should return EINVAL?
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