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
block_t *bf_find_block(size_t size, block_t *_head_free) {
  block_t *curr = _head_free, *bf_block = NULL;

  while (curr != NULL) {
    assert(curr->free == true);

    if (curr->size >= size &&
        (bf_block == NULL || curr->size < bf_block->size)) {
      bf_block = curr;
      if (curr->size == size)
        break;
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
void add_free_block(block_t *curr, block_t *_head_free) {
  assert(curr->free == true);
  curr->next_free = _head_free;
  if (_head_free)
    _head_free->prev_free = curr;
  curr->prev_free = NULL;
  _head_free = curr;
}

/*
  delete_free_block
  This function delete a block from freelist

  Input
  curr: the block to be deleted from freelist
 */
void delete_free_block(block_t *curr, block_t *_head_free) {
  assert(curr->free == false);
  if (curr == _head_free)
    _head_free = curr->next_free;
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
block_t *generate_new_block(size_t size, block_t *_head, block_t *_tail) {
  block_t *new_block = request_new_memory_lock(size);
  if (new_block == NULL) {
    fprintf(stderr, "fail to generate new block\n");
    return NULL;
  }
  new_block->size = size;
  new_block->free = false;
  new_block->next = NULL;
  new_block->prev = _tail;
  new_block->next_free = NULL;
  new_block->prev_free = NULL;
  if (_tail) {
    _tail->next = new_block;
  }
  _tail = new_block;
  if (_head == NULL) {
    _head = new_block;
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
void split(block_t *curr, size_t size, block_t *_tail, block_t *_head_free) {
  block_t *new_block = (void *)curr + size + sizeof(block_t);
  new_block->size = curr->size - size - sizeof(block_t);
  new_block->free = true;
  new_block->next = curr->next;
  new_block->prev = curr;
  if (curr == _tail)
    _tail = new_block;
  if (new_block->next)
    new_block->next->prev = new_block;
  curr->next = new_block;
  curr->size = size;
  add_free_block(new_block, _head_free);
}
/*
  merge
  This function merge two adjunct freed block

  Input
  curr: the block be freed just now
 */
void merge(block_t *curr, block_t *_head_free, block_t *_tail) {
  block_t *next = curr->next, *prev = curr->prev;

  if (next && ((void *)curr + sizeof(block_t) + curr->size == next) &&
      next->free == true) {
    curr->size += sizeof(block_t) + next->size;
    curr->next = next->next;
    if (curr->next)
      curr->next->prev = curr;
    next->free = false;
    delete_free_block(next, _head_free);
    if (_tail == next)
      _tail = curr;
  }
  if (prev && ((void *)prev + sizeof(block_t) + prev->size == curr) &&
      prev->free == true) {
    prev->size += sizeof(block_t) + curr->size;
    prev->next = curr->next;
    if (prev->next)
      prev->next->prev = prev;
    curr->free = false;
    delete_free_block(curr, _head_free);
    if (_tail == curr)
      _tail = prev;
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
block_t *fetch_block(block_t *curr, size_t size, block_t *_tail,
                     block_t *_head_free) {
  curr->free = false;
  delete_free_block(curr, _head_free);
  if (curr->size < size + sizeof(block_t) + sizeof(int))
    return curr;
  split(curr, size, _tail, _head_free);
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
void *basic_malloc(size_t size, block_t *_head, block_t *_tail,
                   block_t *_head_free) {

  block_t *curr = bf_find_block(size, _head);
  if (curr == NULL) {
    curr = generate_new_block(size, _head, _tail);
  } else {
    curr = fetch_block(curr, size, _tail, _head_free);
  }

  return curr + 1;
}

/*
  basic_free
  This function free the memory
 */
void basic_free(void *ptr, block_t *_head_free, block_t *_tail) {
  if (ptr == NULL)
    return;
  assert(((block_t *)ptr - 1)->free == false);
#ifdef DEBUG
  fprintf(stderr, "before free\n");
  debug();
#endif
  block_t *curr = (block_t *)ptr - 1;
  curr->free = true;
  add_free_block(curr, _head_free);
#ifdef DEBUG
  fprintf(stderr, "before merge\n");
  debug();
#endif
  merge(curr, _head_free, _tail);
#ifdef DEBUG
  fprintf(stderr, "after merge\n");
  debug();
#endif
}
/*
  bf_malloc
  This function allocate memory using best fit strategy
 */
void *ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  void *curr = basic_malloc(size, head, tail, head_free);
  pthread_mutex_unlock(&lock);
  return curr;
}

/*
  bf_free
  This function free the memory
 */
void ts_free_lock(void *ptr) {
  pthread_mutex_lock(&lock);
  basic_free(ptr, head_free, tail);
  pthread_mutex_unlock(&lock);
}

void *ts_malloc_nolock(size_t size) {
  return basic_malloc(size, head_nolock, tail_nolock, head_free_nolock);
}

void ts_free_nolock(void *ptr) {
  basic_free(ptr, head_free_nolock, tail_nolock);
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
