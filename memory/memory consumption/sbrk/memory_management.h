#ifndef MY_MALLOC_GUARD__H
#define MY_MALLOC_GUARD__H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct Meta {
    size_t size;
    int free; // 1 is free
    struct Meta *next;
    struct Meta *pre;
} Meta;

#define MB 1024 * 1024
#define META_SIZE sizeof(Meta)

/*-->**********************************************/
/*---------------functions prototype--------------*/
void split(Meta *cur, size_t size);
Meta *require_space(size_t size);
void *first_fit(size_t size);
size_t align(size_t s);
void *_malloc(size_t size);

Meta *merge(Meta *cur);
void _free(void *ptr);

void display (void);
/***********************************************<--*/
#endif
