#ifndef _API_INTERNALS_H
#define _API_INTERNALS_H

/**
 * Returns an absolute path from a relative path.
 */
int get_realpath(const char* rel_path, char* abs_path_buf);

#endif