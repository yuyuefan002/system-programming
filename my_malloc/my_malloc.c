#include "my_malloc.h"
//#define DEBUG
/*
  debug
  This function will print the whole block list
 */
void debug() {
  block_t *curr = head;
  block_t *tail = NULL;
  int i = 0;
  while (curr != NULL) {
    fprintf(stderr, "%d:%ld,%d\n", i++, curr->size, curr->free);
    if (curr->next == NULL)
      tail = curr;
    curr = curr->next;
  }
  fprintf(stderr, "reverse order\n");
  while (tail != NULL) {
    fprintf(stderr, "%d:%ld,%d\n", i++, tail->size, tail->free);
    tail = tail->prev;
  }
}

/*
  name:ff_find_block
  description: Using First Fit strategy to determine the
  memory region to allocate.

  Input
  size: the size of memory requested by user
  prev: pointer pointing to  the last block in the list

  Output
  ff_block: if there is a fit block, return it; else, return NULL
 */
block_t *ff_find_block(size_t size, block_t **prev) {
  block_t *curr = head, *ff_block = NULL;
  while (curr != NULL && (curr->free == false || curr->size < size)) {
    *prev = curr;
    curr = curr->next;
  }
  ff_block = curr;
  return ff_block;
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
block_t *bf_find_block(size_t size, block_t **prev) {
  block_t *curr = head, *bf_block = NULL;
  while (curr != NULL) {
    if (curr->free == true && curr->size >= size &&
        (bf_block == NULL || curr->size < bf_block->size)) {
      bf_block = curr;
    }
    *prev = curr;
    curr = curr->next;
  }
  return bf_block;
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
block_t *generate_new_block(size_t size) {
  block_t *new_block = request_new_memory(size);
  if (new_block == NULL) {
    fprintf(stderr, "fail to generate new block\n");
    return NULL;
  }
  new_block->size = size;
  new_block->free = false;
  new_block->next = NULL;
  total_block_size += size + sizeof(block_t);
  return new_block;
}
/*
  merge
  This function merge two adjunct freed block

  Input
  curr: the block be freed just now
 */
void merge(block_t *curr) {
  while (curr->next && curr->next->free == true) {
    curr->size += sizeof(block_t) + curr->next->size;
    curr->next = curr->next->next;
    if (curr->next)
      curr->next->prev = curr;
  }
  while (curr->prev && curr->prev->free == true) {
    block_t *temp = curr->prev;
    temp->size += sizeof(block_t) + curr->size;
    temp->next = curr->next;
    if (temp->next)
      temp->next->prev = temp;
    curr = temp;
  }
}

/*
  split
  This function split a whole freed space into one space for use and a small
  free space

  Input
  curr: pointer pointing to the current block
  size: the size asked by user
 */
void split(block_t *curr, size_t size) {
  if (curr->size < size + sizeof(block_t))
    return;
  block_t *new_block = (void *)curr + size + sizeof(block_t);
  new_block->size = curr->size - size - sizeof(block_t);
  new_block->free = true;
  new_block->next = curr->next;
  new_block->prev = curr;
  if (new_block->next)
    new_block->next->prev = new_block;
  curr->next = new_block;
  curr->size = size;
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
void *basic_malloc(size_t size, block_t *(find_block)(size_t, block_t **)) {
  if (head == NULL) {
    head = generate_new_block(size);
    head->prev = NULL;
    return head + 1;
  }
  block_t *prev = NULL;
  block_t *curr = find_block(size, &prev);

  if (curr == NULL) {
    curr = generate_new_block(size);
    prev->next = curr;
    curr->prev = prev;
  } else {
    split(curr, size);
    curr->free = false;
    total_free_block_size -= (curr->size + sizeof(block_t));
  }
  return curr + 1;
}

/*
  basic_free
  This function free the memory
 */
void basic_free(void *ptr) {
  if (ptr == NULL)
    return;
#ifdef DEBUG
  fprintf(stderr, "before free\n");
  debug();
#endif
  block_t *curr = (block_t *)ptr - 1;
  curr->free = true;
  total_free_block_size += curr->size + sizeof(block_t);
#ifdef DEBUG
  fprintf(stderr, "before merge\n");
  debug();
#endif
  merge(curr);
#ifdef DEBUG
  fprintf(stderr, "after merge\n");
  debug();
#endif
}

/*
  ff_malloc
  This function allocate memory using first fit strategy
 */
void *ff_malloc(size_t size) { return basic_malloc(size, ff_find_block); }

/*
  ff_free
  This function free the memory
 */
void ff_free(void *ptr) { basic_free(ptr); }

/*
  bf_malloc
  This function allocate memory using best fit strategy
 */
void *bf_malloc(size_t size) { return basic_malloc(size, bf_find_block); }

/*
  bf_free
  This function free the memory
 */
void bf_free(void *ptr) { basic_free(ptr); }

unsigned long get_data_segment_size() { return total_block_size; }

unsigned long get_data_segment_free_space_size() {
  return total_free_block_size;
}
