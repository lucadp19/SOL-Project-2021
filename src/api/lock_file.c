#include "api.h"
#include "api/globals.h"

#include "server-api-protocol.h"

int lockFile(const char* pathname){
    errno = ENOSYS;
    return -1;
}