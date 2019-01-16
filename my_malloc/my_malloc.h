#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct _block_t {
  size_t size;
  struct _block_t *next;
  bool free;
};
typedef struct _block_t block_t;

block_t *head = NULL;

void *ff_malloc(size_t size);
void *bf_malloc(size_t size);
void ff_free(void *ptr);
void bf_free(void *ptr);

#endif
