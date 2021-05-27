#include "util/util.h"

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

int _debug(const char* fmt, ...){
    va_list argp;
    va_start(argp, fmt);

    // fprintf(stderr, "\x1B[33m");
    vfprintf(stderr, fmt, argp);
    // fprintf(stderr, "\x1B[0m");

    va_end(argp);
    return 0;
}

void safe_pthread_mutex_lock(pthread_mutex_t* mtx){
    int err;
    if( (err = pthread_mutex_lock(mtx)) != 0){
        errno = err;
        perror("Error in pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }
}

void safe_pthread_mutex_unlock(pthread_mutex_t* mtx){
    int err;
    if( (err = pthread_mutex_unlock(mtx)) != 0){
        errno = err;
        perror("Error in pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
}

void safe_pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mtx){
    int err;
    if( (err = pthread_cond_wait(cond, mtx)) != 0){
        errno = err;
        perror("Error in pthread_cond_wait");
        exit(EXIT_FAILURE);
    }
}

void safe_pthread_cond_signal(pthread_cond_t* cond){
    int err;
    if( (err = pthread_cond_signal(cond)) != 0){
        errno = err;
        perror("Error in pthread_cond_signal");
        exit(EXIT_FAILURE);
    }
}

void safe_pthread_cond_broadcast(pthread_cond_t* cond){
    int err;
    if( (err = pthread_cond_broadcast(cond)) != 0){
        errno = err;
        perror("Error in pthread_cond_broadcast");
        exit(EXIT_FAILURE);
    }
}

void* safe_malloc(size_t size){
    void* buf;
    if( (buf = malloc(size)) == NULL){
        perror("Error in memory allocation (malloc)");
        exit(EXIT_FAILURE);
    }
    return buf;
}

void* safe_calloc(size_t nmemb, size_t size){
    void* buf;
    if( (buf = calloc(nmemb, size)) == NULL){
        perror("Error in memory allocation (calloc)");
        exit(EXIT_FAILURE);
    }
    return buf;
}

void* safe_realloc(void* ptr, size_t size){
    void* buf;
    if( (buf = realloc(ptr, size)) == NULL){
        perror("Error in memory allocation (realloc)");
        exit(EXIT_FAILURE);
    }
    return buf;
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

void pipe_init(int pipe[]){
    pipe[0] = -1;
    pipe[1] = -1;
}
