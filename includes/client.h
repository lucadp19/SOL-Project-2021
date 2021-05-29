#ifndef _CLIENT_H
#define _CLIENT_H

#include "util/util.h"
#include "util/node.h"
#include "util/list.h"

#include "api.h"

#include <time.h>
#include <getopt.h>

/**
 * A struct representing the client config options.
 */
typedef struct {
    char* socket;
    bool print_to_stdout;
    long waiting_sec;
} client_conf_t;

// ----------- GLOBALS ----------- //
/**
 * Client config data.
 */
extern client_conf_t config;
/**
 * List of requests.
 */
extern list_t* request_q;

/**
 * True iff -h option was set.
 */
extern bool h_option;
/**
 * True iff -p option was set.
 */
extern bool p_option;
/**
 * True iff -f option was set.
 */
extern bool f_option;
/**
 * True iff -a option was set.
 */
extern bool a_option;

// ---------- CONSTANTS ---------- //
/**
 * Sets the time to wait before two consecutive connection attempts.
 */
#define TIME_BETWEEN_CONN 50

// ------------ TYPES ------------ //
typedef struct {
    char* dir;
    long n_files;
} str_long_pair_t;

// ---------- FUNCTIONS ---------- //
/**
 * Parses command line options given to the client program.
 * It returns 0 on success, -1 on error.
 */
int parse_options(list_t* request_list, int argc, char* argv[]);

/**
 * Checks if the parsed request list satisfies all the given requirements.
 * If not, aborts the process.
 */
void validate_options();

/** 
 * Executes the parsed request list.
 */
int execute_requests();

#endif