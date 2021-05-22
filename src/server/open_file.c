#include "server.h"
#include "util/files.h"

int open_file(long fd_client){
    char* pathname = NULL;
    int pathname_len;
    int flags;

    readn(fd_client, &pathname_len, sizeof(int));

    debug("pathname_len = %d\n", pathname_len);

    pathname = calloc((pathname_len + 1), sizeof(char));
    readn(fd_client, (void*)pathname, pathname_len+1);

    debug("pathname = <%s>\n", pathname);

    readn(fd_client, &flags, sizeof(int));
    debug("flag = %d\n", flags);
    if(IS_FLAG_SET(flags, O_CREATE))
        debug("O_CREATE is set\n");
    else debug("O_CREATE is not set\n");
    if(IS_FLAG_SET(flags, O_LOCK))
        debug("O_LOCK is set\n");
    else debug("O_LOCK is not set\n");

    return 0;
}