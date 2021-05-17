#include "util.h"

int str_to_long(const char* str, long* num_ptr){
    if(str == NULL || strlen(str) == 0 || num_ptr == NULL){
        errno = EINVAL;
        return -1;
    }
    // setting errno to zero at the beginning
    errno = 0;

    char* e = NULL;
    long val = strtol(str, &e, 10);

    if(errno == ERANGE) return -2;

    if (e != NULL && *e == (char)0) {
        *num_ptr = val;
        return 0; 
    }

    // not a number
    errno = EINVAL;
    return -3;
}

int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;

    while(left > 0) {
        if( (r=read((int)fd, bufptr, left)) == -1) {
            if (errno == EINTR) 
                continue;
            return -1;
        }
        if (r == 0) 
            return 0;   // EOF
        left    -= r;
        bufptr  += r;
    }

    return size;
}

int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;

    while(left > 0) {
        if( (r=write((int)fd, bufptr, left)) == -1) {
            if (errno == EINTR) 
                continue;
            return -1;
        }
        if (r == 0) 
            return 0;  
        left    -= r;
        bufptr  += r;
    }

    return 1;
}

