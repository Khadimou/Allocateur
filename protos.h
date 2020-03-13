#ifndef _PROTOS_H
#define _PROTOS_H

#define taille 4096


void * malloc(size_t size);
void free(void *ptr);
void *calloc(size_t n_elements,size_t n_octet);
void *realloc(void *ptr,size_t n);
void *alloc_pages(size_t s);
void *allocate(size_t size);
void *aligned_malloc(size_t align, size_t size);
void aligned_free(void * ptr);

void deallocate(void *e,size_t n);

#endif