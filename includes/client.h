#ifndef _CLIENT_H
#define _CLIENT_H

#include "util/util.h"
#include "util/node.h"
#include "util/list.h"

#include <unistd.h>
#include <getopt.h>

/**
 * A struct representing the client config options.
 */
typedef struct {
    char* socket;
    bool print_to_stdout;
} client_conf_t;

/**
 * Parses command line options given to the client program.
 * It returns 0 on success, -1 on error.
 */
int parse_options(list_t* request_q, int argc, char* argv[]);

/**
 * Prints helper message.
 */
void print_helper();
#endif