#include "api.h"

static const char* errno_to_msg(int err);

void api_perror(const char* str){
    fprintf(stderr, "%s: %s.\n", str, errno_to_msg(errno));
}

static const char* errno_to_msg(int err){
    switch (err){
        case 0:
            return "success";
        case ENOTCONN:
            return "client is not connected to server";
        case ENOTEMPTY:
            return "could not create directory";
        case ECANCELED:
            return "could not write every file into directory";
        case EINVAL:
            return "cannot write into file: last operation was not a locking-creation";
        case ENOSYS:
            return "operation is not implemented yet";
        case EIO:
            return "could not open file";
        case ENOTRECOVERABLE:
            return "fatal error in server or client application";
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