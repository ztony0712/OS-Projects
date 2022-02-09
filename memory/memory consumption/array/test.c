#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct Meta {
    size_t size;
    int free; // 1 is free
    int tail; // 1 is tail
    struct Meta *next;
} Meta;

#define HEAP_SIZE 1024*1024*1024 // 1 MB for every HEAP
#define META_SIZE sizeof(Meta)

/*-->**********************************************/
/*----------------global variable-----------------*/

char header[HEAP_SIZE];
Meta *p = (void*)header;
/***********************************************<--*/



/*-->**********************************************/
/*----------------my_malloc-----------------------*/
void init(Meta* block_p){
    block_p->size = HEAP_SIZE - META_SIZE;
    block_p->free = 1;
    block_p->tail = 1;
}

void split(Meta* current, size_t size) {
    
    Meta* new = (void*)((void*)current+sizeof(Meta)+size);

    if (current->tail == 1)
    {
        new->tail = 1;
        current->tail = 0;
    }
    new->free = 1;
    new->size = current->size-size-sizeof(Meta);

    current->size = size;
    current->free = 0;

    new->next = current->next;
    current->next = new;

}


void add_heap (Meta* current) {
    char heap[HEAP_SIZE];
    current->next = (void*)heap;
    init(current);

}

void* my_malloc(size_t size) {
    Meta *current = p;
    Meta *block_tail = NULL;
    
    if (size == 0)
        return NULL;
    if (p->size == 0)
        init(p);

    for (; current != NULL; current = current->next) {
        if (current->free == 1)
        {
            if (current->size == size)
            {
                current->free = 0;
                add_heap(current);
                return (void*)((void*)current+sizeof(Meta));
            }
            else if (current->size > size + META_SIZE)
            {
                
                break; 
            }
            else if (size + sizeof(Meta) > HEAP_SIZE)
            {
                printf("Can't distribute enough size of memory once!");
                return NULL;
            }
        }
        if (current->next == NULL)
        {
            block_tail = current;
            current->tail = 1;
            
        }
        
    }
    

    if (current == NULL) {
        current = block_tail;
        add_heap(current); //add a heap on the tail
        current = current->next;
    }
    
    split(current, size); // split excess part into new heap
    return (void*)((void*)current+sizeof(Meta));

}
/***********************************************<--*/


/*-->**********************************************/
/*----------------my_free-------------------------*/
void merge(void) {
    Meta* current = p;
    for (; current != NULL; current = current->next) {
        if (current->free == 1 && current->next->free == 1)
        {
            if (current->tail == 0)
            {
                if (current->next->tail == 1) {
                    current->tail = 1;
                }
                current->size += current->next->size;
                current->next = current->next->next;
            }
            else
            {
                continue;
            }
        }
    }
}


void my_free(void *pointer) {
    if (pointer == NULL)
    {
        printf("Void pointer freed\n");
    }
    else
    {
        Meta* current = p;
        for (; current != NULL; current = current->next)
        {
            if ((void*)((void*)current+META_SIZE) == (void*)pointer)
            {
                current->free = 1;
                merge();
                return;
            }
        }
        printf("Void pointer freed\n");
    }
}

/***********************************************<--*/


int main(int argc, char const *argv[])
{
    char *a = (char*) my_malloc(1024*1000*1024);
    memset(a, 'a', 1024*1000*1024);
    my_free(a);
//    a = (char*) my_malloc(1024*1000*1024);
//    memset(a, 'a', 1024*1000*1024);
//    char *b = (char*) my_malloc(4);
//    memset(b, 'b', 4);
//    char *c = (char*) my_malloc(50);
//    memset(c, 'c', 50);

    printf("%p\n", header[0]);
    printf("%p\n", heap[0]);

    return 0;
}
