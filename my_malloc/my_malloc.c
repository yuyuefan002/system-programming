#include "my_malloc.h"

void debug() {
  block_t *curr = head;
  int i = 0;
  while (curr != NULL) {
    fprintf(stderr, "%d:%ld,%d\n", i++, curr->size, curr->free);
    curr = curr->next;
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
  while (curr != NULL && !(curr->free == true && curr->size > size)) {
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
    if (curr->free == true && curr->size > size &&
        (bf_block == NULL || curr->size < bf_block->size)) {
      bf_block = curr;
    }
    *prev = curr;
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
  return new_block;
}

void merge(block_t *curr) {
  while (curr->next != NULL) {
    if (curr->free == true && curr->next->free == true) {
      curr->size += sizeof(block_t) + curr->next->size;
      curr->next = curr->next->next;
    } else {
      curr = curr->next;
    }
  }
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
    return head + 1;
  }
  block_t *prev = NULL;

  block_t *curr = find_block(size, &prev);

  if (curr == NULL) {

    curr = generate_new_block(size);
    prev->next = curr;
  } else {
    curr->free = false;
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
  block_t *curr = (block_t *)ptr - 1;
  curr->free = true;
#ifdef DEBUG
  fprintf(stderr, "before merge\n");
  debug();
#endif
  merge(head);
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
