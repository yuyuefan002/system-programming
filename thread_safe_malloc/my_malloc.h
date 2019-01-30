#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct _block_t {
  size_t size;
  struct _block_t *next;
  struct _block_t *prev;
  struct _block_t *next_free;
  struct _block_t *prev_free;
  bool free;
};
typedef struct _block_t block_t;
__thread block_t *head_nolock = NULL;
__thread block_t *tail_nolock = NULL;
__thread block_t *head_free_nolock = NULL;
block_t *head = NULL;
block_t *tail = NULL;
block_t *head_free = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sbrk_lock = PTHREAD_MUTEX_INITIALIZER;
bool first_sbrk = true;
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
#endif
