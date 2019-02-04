#include "my_malloc.h"
//#define DEBUG
/*
  debug
  This function will print the whole block list
 */
void debug(block_t *_head, block_t *_tail, block_t *_head_free) {
  block_t *curr = _head;
  int i = 0;
  while (curr != NULL) {
    fprintf(stderr, "%d:%ld,%d\n", i++, curr->size, curr->free);
    curr = curr->next;
  }
  fprintf(stderr, "reverse order\n");
  curr = _tail;
  while (curr != NULL) {
    fprintf(stderr, "%d:%ld,%d\n", i++, curr->size, curr->free);
    curr = curr->prev;
  }
  fprintf(stderr, "free list\n");
  curr = _head_free;
  while (curr != NULL) {
    fprintf(stderr, "%d:%ld,%d\n", i++, curr->size, curr->free);
    curr = curr->next_free;
  }
}

/*
  name:bf_find_block
  description: Using Best Fit strategy to determine the
  memory region to allocate.

  Input
  size: the size of memory requested by user
  prev: pointer pointing to  the last block in the list

  Output
  bf_block: if there is a fit block, return it; else, return NULL
 */
block_t *bf_find_block(size_t size, block_t *head_free) {
  block_t *curr = head_free, *bf_block = NULL;

  while (curr != NULL) {
    assert(curr->free == true);

    if (curr->size >= size &&
        (bf_block == NULL || curr->size < bf_block->size)) {
      bf_block = curr;
      if (curr->size == size) {
        break;
      }
    }
    curr = curr->next_free;
  }
  return bf_block;
}

/*
  add_free_block
  This function add a block into freelist

  Input
  curr:the block to be added into free list

  Output

 */
void add_free_block_lock(block_t *curr) {
  assert(curr->free == true);
  curr->next_free = head_free;
  if (head_free)
    head_free->prev_free = curr;
  curr->prev_free = NULL;
  head_free = curr;
}
void add_free_block_nolock(block_t *curr) {
  assert(curr->free == true);

  block_t *p = head_free_nolock;
  if (p == NULL) {
    head_free_nolock = curr;
    curr->next_free = NULL;
    curr->prev_free = NULL;
    return;
  }
  while (p->next_free) {
    block_t *next = p->next_free;
    if (next > curr) {
      curr->next_free = next;
      curr->prev_free = p;
      next->prev_free = curr;
      p->next_free = curr;
      break;
    }
    p = p->next_free;
  }
  if (p->next_free == NULL) {
    p->next_free = curr;
    curr->prev_free = p;
    curr->next_free = NULL;
  }
}

/*
  delete_free_block
  This function delete a block from freelist

  Input
  curr: the block to be deleted from freelist
 */
void delete_free_block_lock(block_t *curr) {
  assert(curr->free == false);
  if (curr == head_free)
    head_free = curr->next_free;
  if (curr->next_free)
    curr->next_free->prev_free = curr->prev_free;
  if (curr->prev_free)
    curr->prev_free->next_free = curr->next_free;
  curr->next_free = NULL;
  curr->prev_free = NULL;
}
void delete_free_block_nolock(block_t *curr) {
  // assert(curr->free == false);
  if (curr == head_free_nolock)
    head_free_nolock = curr->next_free;
  if (curr->next_free)
    curr->next_free->prev_free = curr->prev_free;
  if (curr->prev_free)
    curr->prev_free->next_free = curr->next_free;
  curr->next_free = NULL;
  curr->prev_free = NULL;
}

/*
  request_new_memory
  description: Using sbrk() to request new memory

  Input
  size: the memory size asked by user
  Output
  new_memory: if success, return the newly allocated memory,
  else, return NULL
 */
void *request_new_memory(size_t size) {
  void *new_memory = sbrk(size + sizeof(block_t));
  if (new_memory == (void *)-1) {
    perror("sbrk fail\n");
    return NULL;
  }
  return new_memory;
}

void *request_new_memory_lock(size_t size) {
  pthread_mutex_lock(&sbrk_lock);
  void *new_memory = sbrk(size + sizeof(block_t));
  pthread_mutex_unlock((&sbrk_lock));
  if (new_memory == (void *)-1) {
    fprintf(stderr, "sbrk fail\n");
    return NULL;
  }
  return new_memory;
}

/*
  generate_new_block
  description: generate a new block, composed of meta info block
  and required block

  Input
  size: the memory size asked by user
  Output
  new_block: if success, return the newly allocated block,
  else, return NULL
 */

block_t *generate_new_block_lock(size_t size) {
  block_t *new_block = request_new_memory_lock(size);
  if (new_block == NULL) {
    fprintf(stderr, "fail to generate new block\n");
    return NULL;
  }
  new_block->size = size;
  new_block->free = false;
  new_block->next = NULL;
  new_block->prev = tail;
  new_block->next_free = NULL;
  new_block->prev_free = NULL;
  if (tail) {
    tail->next = new_block;
  }
  tail = new_block;
  if (head == NULL) {
    head = new_block;
  }

  return new_block;
}
block_t *generate_new_block_nolock(size_t size) {
  block_t *new_block = request_new_memory_lock(size);
  if (new_block == NULL) {
    fprintf(stderr, "fail to generate new block\n");
    return NULL;
  }
  new_block->size = size;
  new_block->free = false;
  new_block->next = NULL;
  new_block->prev = tail_nolock;
  new_block->next_free = NULL;
  new_block->prev_free = NULL;
  if (tail_nolock) {
    tail_nolock->next = new_block;
  }
  tail_nolock = new_block;
  if (head_nolock == NULL) {
    head_nolock = new_block;
  }

  return new_block;
}
/*
  split
  This function split a block into two blocks

  Input
  curr: the whole block
  size: one block size(the other size is curr->size-size-sizeof(block_t)
*/
void split_lock(block_t *curr, size_t size) {
  block_t *new_block = (void *)curr + size + sizeof(block_t);
  new_block->size = curr->size - size - sizeof(block_t);
  new_block->free = true;
  new_block->next = curr->next;
  new_block->prev = curr;
  if (curr == tail)
    tail = new_block;
  if (new_block->next)
    new_block->next->prev = new_block;
  curr->next = new_block;
  curr->size = size;
  add_free_block_lock(new_block);
}

void split_nolock(block_t *curr, size_t size) {
  block_t *new_block = (void *)curr + size + sizeof(block_t);
  new_block->size = curr->size - size - sizeof(block_t);
  new_block->free = true;
  new_block->next = curr->next;
  new_block->prev = curr;
  if (curr == tail_nolock)
    tail_nolock = new_block;
  if (new_block->next)
    new_block->next->prev = new_block;
  curr->next = new_block;
  curr->size = size;
  add_free_block_nolock(new_block);
}
/*
  merge
  This function merge two adjunct freed block

  Input
  curr: the block be freed just now
 */
void merge_lock(block_t *curr) {
  block_t *next = curr->next, *prev = curr->prev;

  if (next && ((void *)curr + sizeof(block_t) + curr->size == next) &&
      next->free == true) {
    curr->size += sizeof(block_t) + next->size;
    curr->next = next->next;
    if (curr->next)
      curr->next->prev = curr;
    next->free = false;
    delete_free_block_lock(next);
    if (tail == next)
      tail = curr;
  }
  if (prev && ((void *)prev + sizeof(block_t) + prev->size == curr) &&
      prev->free == true) {
    prev->size += sizeof(block_t) + curr->size;
    prev->next = curr->next;
    if (prev->next)
      prev->next->prev = prev;
    curr->free = false;
    delete_free_block_lock(curr);
    if (tail == curr)
      tail = prev;
  }
}
void merge_nolock(block_t *curr) {
  block_t *next = curr->next_free, *prev = curr->prev_free;

  if (next && ((void *)curr + sizeof(block_t) + curr->size == next) &&
      next->free == true) {
    curr->size += sizeof(block_t) + next->size;
    curr->next = next->next;
    if (curr->next)
      curr->next->prev = curr;
    next->free = false;
    delete_free_block_nolock(next);
    if (tail_nolock == next)
      tail_nolock = curr;
  }
  if (prev && ((void *)prev + sizeof(block_t) + prev->size == curr) &&
      prev->free == true) {
    prev->size += sizeof(block_t) + curr->size;
    prev->next = curr->next;
    if (prev->next)
      prev->next->prev = prev;
    curr->free = false;
    delete_free_block_nolock(curr);
    if (tail_nolock == curr)
      tail_nolock = prev;
  }
}
/*
  fetch_block
  This function fetch a part of the block from a whole block

  Input
  curr: pointer pointing to the current block
  size: the size asked by user

  Output
  the block to use
 */
block_t *fetch_block(block_t *curr, size_t size,
                     void(delete_free_block)(block_t *),
                     void(split)(block_t *, size_t)) {
  curr->free = false;
  delete_free_block(curr);
  if (curr->size < size + sizeof(block_t) + sizeof(int))
    return curr;
  split(curr, size);
  return curr;
}

/*
  basic_malloc
  description: allocate memory requested by user

  Input
  size: the memory size asked by user
  find_block: function pointer pointing to the specific
  find block strategy
  Output
  the memory
 */
void *basic_malloc(size_t size, block_t *head_free,
                   block_t *(generate_new_block)(size_t),
                   void(delete_free_block)(block_t *),
                   void(split)(block_t *, size_t)) {

  block_t *curr = bf_find_block(size, head_free);
  if (curr == NULL) {
    curr = generate_new_block(size);
  } else {
    curr = fetch_block(curr, size, delete_free_block, split);
  }

  return curr + 1;
}

/*
  basic_free
  This function free the memory
 */
void basic_free(void *ptr, void(add_free_block)(block_t *),
                void(merge)(block_t *), block_t *_head, block_t *head_free,
                block_t *_tail) {
  if (ptr == NULL)
    return;
  assert(((block_t *)ptr - 1)->free == false);
#ifdef DEBUG
  fprintf(stderr, "before free\n");
  debug(_head, _tail, head_free);
#endif
  block_t *curr = (block_t *)ptr - 1;
  curr->free = true;
  add_free_block(curr);
#ifdef DEBUG
  fprintf(stderr, "before merge\n");
  debug(_head, _tail, head_free);
#endif
  if (merge != NULL)
    merge(curr);
#ifdef DEBUG
  fprintf(stderr, "after merge\n");
  debug(_head, _tail, head_free);
#endif
}
/*
  bf_malloc
  This function allocate memory using best fit strategy
 */
void *ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  void *curr = basic_malloc(size, head_free, generate_new_block_lock,
                            delete_free_block_lock, split_lock);
  pthread_mutex_unlock(&lock);
  return curr;
}

/*
  bf_free
  This function free the memory
 */
void ts_free_lock(void *ptr) {
  pthread_mutex_lock(&lock);
  basic_free(ptr, add_free_block_lock, merge_lock, head, head_free, tail);
  pthread_mutex_unlock(&lock);
}

void *ts_malloc_nolock(size_t size) {
  return basic_malloc(size, head_free_nolock, generate_new_block_nolock,
                      delete_free_block_nolock, split_nolock);
}

void ts_free_nolock(void *ptr) {
  basic_free(ptr, add_free_block_nolock, merge_nolock, head_nolock,
             head_free_nolock, tail_nolock);
}

unsigned long get_data_segment_size() {
  size_t total = 0;
  block_t *curr = head;
  while (curr) {
    total += curr->size + sizeof(block_t);
    curr = curr->next;
  }
  return total;
}

unsigned long get_data_segment_free_space_size() {
  size_t total = 0;
  block_t *curr = head;
  while (curr) {
    if (curr->free)
      total += curr->size + sizeof(block_t);
    curr = curr->next;
  }
  return total;
}
