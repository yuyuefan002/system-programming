#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct _block_t {
  size_t size;
  struct _block_t *next;
  struct _block_t *prev;
  bool free;
};
typedef struct _block_t block_t;

block_t *head = NULL;

void *ff_malloc(size_t size);
void *bf_malloc(size_t size);
void ff_free(void *ptr);
void bf_free(void *ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
#endif
