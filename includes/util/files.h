#ifndef _FILES_H
#define _FILES_H

#include "util/util.h"
#include "util/list.h"

/** An elementary struct for a buffer, containing its size and the data. */
typedef struct {
    size_t size;
    void* buf;
} size_n_buf_t;

/** Taken a path to a file (absolute or relative),
 * strips away everything but the basename and returns it.
 * If there are no slashes ('/') in pathname, returns pathname.
 */
const char* remove_path_from_name(const char* pathname);

/** 
 * Recursively creates all the directories necessary to create dirname.
 * On success returns -1, on error sets errno and returns -1.
 * Possible values for errno are 
 *  * [ENAMETOOLONG] if the path is too long, 
 *  * any of the errors returned by mkdir (man 2 mkdir).
 */
int mkdir_p(const char* dirname);

/**
 * Given a list of files, writes them all into directory dir.
 * Returns
 *      - on success: a positive integer representing the number of files actually written;
 *      - on error -1 and sets errno.
 * The possible values for errno are those set by mkdir_p.
 */
int write_list_of_files_into_dir(list_t* files, const char* dir);

/** Given a node containing a size_n_buf, frees the buffer and the node. */
void free_node_size_n_buf(node_t* node);

/**
 * Given a pathname returns in size the size of the file with the given name.
 * Returns 0 on success, -1 on error.
 */
int get_file_size(const char* pathname, off_t* size);

#endif