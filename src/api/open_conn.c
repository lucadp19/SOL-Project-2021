#include "api.h"
#include "api/globals.h"


int openConnection(const char* sockname, int msec, const struct timespec abstime){
    // already connected
    if(socket_path != NULL || fd_sock != -1){
        // already connected!
        errno = EISCONN;
        return -1;
    }

    // invalid arguments
    if(sockname == NULL || msec < 0) {
        errno = EINVAL;
        return -1;
    }

    // socket creation
    if( (fd_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        return -1;
    
    // initializing address for connection
    struct sockaddr_un sock_addr;
    memset(&sock_addr, '0', sizeof(sock_addr));
    strncpy(sock_addr.sun_path, sockname, strlen(sockname) + 1);
    sock_addr.sun_family = AF_UNIX;

    // setting waiting time
    struct timespec wait_time;
    // no need to check because msec > 0 and &wait_time != NULL 
    set_timespec_from_msec(msec, &wait_time);

    // setting current time
    struct timespec curr_time;
    clock_gettime(CLOCK_REALTIME, &curr_time);

    // trying to connect
    int err = -1;
    while( (err = connect(fd_sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr))) == -1
            && curr_time.tv_sec < abstime.tv_sec ){
        debug("connect didn't succeed, trying again...\n");
            
        if( nanosleep(&wait_time, NULL) == -1){
            RESET_SOCK;
            return -1;
        }
        if( clock_gettime(CLOCK_REALTIME, &curr_time) == -1){
            RESET_SOCK;
            return -1;        
        }
    }

    if(err == -1) {
        debug("Could not connect to server. :(\n");

        RESET_SOCK;
        errno = ETIMEDOUT;
        return -1;
    }

    debug("Connected! :D\n");

    socket_path = sockname;
    return 0;
}
