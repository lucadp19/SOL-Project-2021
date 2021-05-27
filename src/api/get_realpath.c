#include "api/internals.h"

#include <limits.h>
#include <stdlib.h>

int get_realpath(const char* rel_path, char* abs_path){
    char* res = realpath(rel_path, abs_path);
    if(res == NULL)
        return -1;
    
    return 0;
}