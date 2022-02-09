#include "memory_management.h"

/*-->**********************************************/
/*----------------global variable-----------------*/

Meta *p = NULL;
/***********************************************<--*/



/*-->**********************************************/
/*----------------_malloc-----------------------*/

void split(Meta *cur, size_t size) {
    Meta *new = (void *)((void *)cur + META_SIZE + size);

    // init new meta and resize cur meta
    new->free = 1;
    new->size = cur->size - size - META_SIZE;
    cur->size = size;

    // insert new meta into meta linked list
    new->next = cur->next;
    cur->next = new;
    if (new->next != NULL)
        new->next->pre = new;
    new->pre = cur;
}

Meta *require_space(size_t size) {
    Meta *result = NULL;

    Meta *bk = (Meta *)sbrk(0);
    if (sbrk(size + META_SIZE) == (void *)-1) {
        // fail to melloc and return NULL
        result = NULL;
        printf("No enough memory to malloc...\n");
        fflush(stdout);
    } else {
        result = bk;
        result->size = size;
        result->free = 0;
        printf("Allocate memory by requiring new space \n");
        fflush(stdout);
    }
    return result;
}

void *first_fit(size_t size) {
    Meta *cur = p;
    Meta *tail = NULL;

    for (; cur != NULL; cur = cur->next) {
        // find free and eligible block
        // in distributed memory
        if (cur->free == 1) {
            if (cur->size == size+sizeof(Meta)) {
                cur->free = 0;
                printf("Allocate memory with an exact fitted block\n");
                fflush(stdout);
                return (void *)((void *)cur + META_SIZE);
            } else if (cur->size > size + META_SIZE) {
                cur->free = 0;
                split(cur, size); // split excess part into new heap
                printf("Allocate memory by spliting existing block\n");
                fflush(stdout);
                return (void *)((void *)cur + META_SIZE);
            }
        }

        // record the last block
        // if there is no avaliable block
        if (cur->next == NULL)
            tail = cur;
    }

    // require a piece of new memory block
    // add the new meta to linked meta block list
    Meta *new = require_space(size);
    if (new == NULL)
        return NULL;
    tail->next = new;
    new->pre = tail;
    tail = tail->next;
    return (void *)((void *)tail + META_SIZE);
}

// align input size to a multiple of 8
size_t align(size_t s) {
    if ((s & 0x7) == 0)
        return s;
    return ((s >> 3) + 1) << 3;
}

void *_malloc(size_t size) {
    fflush(stdin);
    fflush(stdout);

    if (size <= 0) {
        printf("Invalid size input...\n");
        return NULL;
    }

    size = align(size);
    void *result = NULL;

    // init p when it is NULL
    // otherwise, use first fit to distribute memory
    if (p == NULL) {
        p = require_space(size);
        if (p == NULL)
            return NULL;
        result = (void *)((void *)p + META_SIZE);
    } else
        result = first_fit(size);

    if (result != NULL)
        printf("%zu bytes of memory allocated\n", size);

    return result;
}
/***********************************************<--*/

/*-->**********************************************/
/*----------------_free-------------------------*/
Meta *merge(Meta *cur) {
    // try to merge with pre block
    // when pre block exists
    if (cur->pre != NULL) {
        if (cur->pre->free == 1) {
            cur->pre->size += cur->size + META_SIZE;
            cur->pre->next = cur->next;
            cur = cur->pre;
        }
    }

    // try to merge with next block
    // when next block exists
    if (cur->next != NULL && cur->next->free == 1) {
        cur->size += cur->next->size + META_SIZE;
        cur->next = cur->next->next;
    }

    return cur;
}

void _free(void *ptr) {
    fflush(stdin);
    fflush(stdout);
    if (ptr != NULL) {
        Meta *cur = p;
        for (; cur != NULL; cur = cur->next) {
            if ((void *)((void *)cur + META_SIZE) == (void *)ptr) {
                // set current block to free and try to merge
                cur->free = 1;
                cur = merge(cur);

                // give memory back to system
                if (cur->next == NULL) {
                    if (cur->pre != NULL)
                        cur->pre->next = NULL;
                    else
                        p = NULL;
                    printf("%zu bytes of memory returned\n", cur->size);
                    fflush(stdout);
                    brk(cur);
                    return;
                }
                printf("No memory returned\n");
                fflush(stdout);
                return;
            }
        }

        // fail to find target pointer
        // no operation will be performed
        printf("Void pointer freed...\n");
        fflush(stdout);
    }
}

void display (void) {
    printf("=>------------<=\n");
    Meta* cur = p;
    for (int i = 0; cur != NULL; cur = cur->next) {
        printf("Block %d\n", i);
        if (cur->free == 1)
            printf("free *\n");
        else
            printf("Busy\n");
        printf("size: %d bytes\n", cur->size);
        printf("----------------\n");
        i++;
    }
}

/***********************************************<--*/

int main(int argc, char const *argv[]) {
    size_t size = 20;

    printf("Press enter to continue program...");
    
    getchar();
    char *a = (char *)_malloc(size*MB);
    memset(a, 'a', size*MB);
    printf("Allocate 20 MB char* type of memo to a\n");
    display();
    
    getchar();
    int *b = (int *)_malloc(size*MB*sizeof(int));
    memset(b, 2, size*MB*sizeof(int));
    printf("Allocate 80 MB int* type of memo to b\n");
    display();

    getchar();
    float *c = (float *)_malloc(size*MB*sizeof(float));
    memset(c, 3.0, size*MB*sizeof(float));
    printf("Allocate 80 MB float* type of memo to c\n");
    display();

    getchar();
    _free(a);
    printf("Free memo of a\n");
    display();

    getchar();
    _free(c);
    printf("Free memo of c\n");
    display();

    getchar();
    char* d = (char *)_malloc(size/2*MB);
    memset(d, 'd', size/2*MB);
    printf("Allocate 10 MB char* type of memo to d\n");
    display();

    getchar();
    char* e = (char *)_malloc(10485728-sizeof(Meta));
    memset(e, 'e', 10485728-sizeof(Meta));
    printf("Allocate 10 MB char* type of memo to e\n");
    display();

    getchar();
    printf("Try to input 0 to _malloc\n");
    char* f = (char *)_malloc(0);
    display();

    getchar();
    return 0;
}
