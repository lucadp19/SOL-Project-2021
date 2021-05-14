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