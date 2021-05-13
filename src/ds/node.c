#include "node.h"
#include <errno.h>

node_t* create_node(const char* key, void* data){
    node_t* node;

    if( (node = (node_t*)malloc(sizeof(node_t))) == NULL){
        // malloc failed
        errno = ENOMEM;
        return NULL;
    }

    node->key = key;
    node->data = data;
    node->prev = NULL;
    node->next = NULL;

    return node;
}

void free_node(node_t* node){
    if(node != NULL) {
        free((void*)node->key);
        free(node->data);
        free(node);
    }
}