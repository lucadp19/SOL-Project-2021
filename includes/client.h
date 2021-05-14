#ifndef _CLIENT_H
#define _CLIENT_H

#include "util.h"
#include "node.h"
#include "list.h"

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