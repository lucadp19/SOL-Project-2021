#include "list.h"
#include <errno.h>

list_t* empty_list(){
    list_t* list;

    if( (list = (list_t*)malloc(sizeof(list_t))) == NULL){
        // malloc failed
        errno = ENOMEM;
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;

    return list;
}

void list_delete(list_t* list, void (*node_cleaner)(node_t*)){
    if(list == NULL) return;

    node_t* tmp;
    while(list->head != NULL){
        tmp = list->head;
        list->head = list->head->next;

        if(node_cleaner == NULL)
            free_node(tmp);
        else node_cleaner(tmp);
    }

    free(list);
}

int list_push_front(list_t* list, node_t* node){
    if (list == NULL || node == NULL){
        errno = EINVAL;
        return -1;
    }
 
    node->prev = NULL;
    node->next = NULL;
    
    if(list->length == 0) { // empty list
        list->head = node;
        list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->length++;

    return 0;
}

int list_push_back(list_t* list, node_t* node){
    if (list == NULL || node == NULL){
        errno = EINVAL;
        return -1;
    }
 
    node->prev = NULL;
    node->next = NULL;

    if(list->length == 0) { // empty list
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    list->length++;
    
    return 0;
}

node_t* list_pop_front(list_t* list){
    // list must not be null or empty
    if(list == NULL || list->length == 0){
        errno = EINVAL;
        return NULL;
    }

    node_t* node = list->head;
    
    // removing first element from list
    list->length--;
    list->head = list->head->next;
    if(list->length == 0) 
        list->tail = NULL;
    
    return node;
}

node_t* list_pop_back(list_t* list){
    // list must not be null or empty
    if(list == NULL || list->length == 0){
        errno = EINVAL;
        return NULL;
    }
    
    node_t* node = list->tail;
    
    // removing first element from list
    list->length--;
    list->tail = list->tail->prev;
    if(list->length == 0) 
        list->head = NULL;
    
    return node;
}