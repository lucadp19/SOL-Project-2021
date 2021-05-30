#include "api.h"
#include "api/globals.h"
#include "server-api-protocol.h"

int convert_res_to_errno(int res){
    switch(res){
        case SA_SUCCESS:
            return 0;
        case SA_EXISTS:
            return EEXIST;
        case SA_NO_FILE:
            return ENOENT;
        case SA_ALREADY_LOCKED:
            return EBUSY;
        case SA_NOT_LOCKED:
            return EPERM;
        case SA_TOO_BIG:
            return EFBIG;
        case SA_NO_OPEN:
            return ENOKEY;
        case SA_NOT_EMPTY:
            return EACCES;
        case SA_CLOSE:
        case SA_ERROR:
        default:
            return ENOTRECOVERABLE;
    }
}