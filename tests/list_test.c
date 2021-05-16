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

    printf("Pushing elements to both ends of the list.\n");
    printf("First item\n");
    if( list_push_back(list, "1", (void*)1l) == -1){
        perror("list push back node1"); return -1;
    }
    // print_list(list);
    printf("Second item\n");
    if( list_push_back(list, "2", (void*)2l) == -1){
        perror("list push back node2"); return -1;
    }
    // print_list(list);
    printf("Third item\n");
    if( list_push_front(list, "0", (void*)0l) == -1){
        perror("list push front node0"); return -1;
    }
    print_list(list);

    printf("Popping one element at a time from the back.\n");
    while(list->nelem != 0){
        const char* key;
        long val;
        list_pop_back(list, &key, (void**)&val);
        printf("New element: key = <%s>, data = <%ld>\n", key, val);
    } printf("List is empty. Deleting list.\n");

    // printf("Deleting list.\n");
    // fflush(stdout);
    list_delete(list, cleaner);
    printf("Yee! :)\n");
}

void print_list(list_t* list){
    node_t* curr = list->head;
    printf("Printing list!\n");

    printf("List length: %d\n\n", list->nelem);
    while(curr != NULL){
        printf("Found new element! ");
        if(curr->key != NULL)
            printf("Key = <%s>! ", curr->key);
        else printf("Key NULL. ");
        if(curr->data != NULL)
            printf("Data = <%ld>! ", (long)curr->data);
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