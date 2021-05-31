#include "api.h"

static const char* errno_to_msg(int err);

void api_perror(const char* str){
    fprintf(stderr, "%s: %s.", str, errno_to_msg(errno));
}

static const char* errno_to_msg(int err){
    switch (err){
        case 0:
            return "success";
        case ENOTRECOVERABLE:
            return "fatal server error";
        case EBADE:
            return "fatal error in server-client communication";
        case EEXIST:
            return "file already exists";
        case ENOENT:
            return "file is not present in server";
        case EBUSY:
            return "file is already locked";
        case EPERM:
            return "file is not locked by client";
        case EFBIG:
            return "file is too big to fit into server";
        case ENOKEY:
            return "file has not been opened by client";
        case EACCES:
            return "file was not empty";        
        default:
            return strerror(err);
    }
}