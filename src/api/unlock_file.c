#include "api.h"
#include "api/globals.h"

#include "server-api-protocol.h"

int unlockFile(const char* pathname){
    errno = ENOSYS;
    return -1;
}