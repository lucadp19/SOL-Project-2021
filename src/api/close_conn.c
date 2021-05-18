#include "api.h"
#include "api/globals.h"

int closeConnection(const char* sockname){
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
        
    RESET_SOCK;
    return 0;    
}