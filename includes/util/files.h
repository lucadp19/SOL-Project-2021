#ifndef _FILES_H
#define _FILES_H

#include "util/util.h"
#include "util/list.h"

typedef struct {
    size_t size;
    void* buf;
} size_n_buf_t;

const char* remove_path_from_name(const char* pathname);
int mkdir_p(const char* dirname);
int write_list_of_files_into_dir(list_t* files, const char* dir);
void free_node_size_n_buf(node_t* node);

#endif