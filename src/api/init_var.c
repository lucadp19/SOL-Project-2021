#include "api.h"
#include "api/globals.h"

long fd_sock = -1;
const char* socket_path = NULL;

last_op_t last_op = {
    .is_open = false, 
    .lock    = false, 
    .create  = false, 
    .success = false,
    .path    = NULL 
};