#include "util.h"
#include "list.h"
#include "node.h"

void print_list(list_t*);
void cleaner(node_t*);

int main(){
    list_t* list;

    printf("Creating linked list.\n");
    if( (list = empty_list()) == NULL){
        perror("list");
        return -1;
    }

    printf("Creating nodes.\n");
    node_t *node1;
    node_t *node2;
    node_t *node0;

    if( (node1 = create_node("1", (void*)"1")) == NULL){
        perror("node1"); return -1;
    }
    if( (node2 = create_node("2", (void*)"2")) == NULL){
        perror("node2"); return -1;
    }
    if( (node0 = create_node("0", (void*)"0")) == NULL){
        perror("node0"); return -1;
    }

    printf("Pushing elements to both ends of the list.\n");
    printf("First item\n");
    if( list_push_back(list, node1) == -1){
        perror("list push back node1"); return -1;
    }
    // print_list(list);
    printf("Second item\n");
    if( list_push_back(list, node2) == -1){
        perror("list push back node2"); return -1;
    }
    // print_list(list);
    printf("Third item\n");
    if( list_push_front(list, node0) == -1){
        perror("list push front node0"); return -1;
    }
    // print_list(list);

    printf("Deleting list.\n");
    fflush(stdout);
    list_delete(list, cleaner);

    printf("Yee! :)\n");
}

void print_list(list_t* list){
    node_t* curr = list->head;
    printf("Printing list!\n");

    printf("List length: %d\n\n", list->length);
    while(curr != NULL){
        printf("Found new element! ");
        if(curr->key != NULL)
            printf("Key not NULL! ");
        else printf("Key NULL. ");
        if(curr->data != NULL)
            printf("Data not NULL! ");
        else printf("Data NULL. ");
        if(curr->prev != NULL)
            printf("Prev not NULL! ");
        else printf("Prev NULL. ");
        if(curr->next != NULL)
            printf("Next not NULL! ");
        else printf("Next NULL. ");
        printf("\n");

        curr = curr->next;
    }
    printf("End of list!\n");
}

void cleaner(node_t* to_del){
    free(to_del);
}