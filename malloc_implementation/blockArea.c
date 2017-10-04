#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "block_Header.h"

/*typedef struct BlockStruct
{
  size_t size;
  // struct BlockStruct *left;
  // struct BlockStruct *right;
  struct BlockStruct *next;
  int free;
} BlockStruct;

void *base = NULL;*/

// typedef BlockStruct bs;
bstruct find_block(bstruct *last, size_t s)
{
  bstruct b = base;

  while (b && !(b->free && b->size >= s))
  {
    *last = b;
    b = b->next;
  }
  return (b);
}

bstruct extend_heap(bstruct last, size_t s)
{
  char buf[1024];
  snprintf(buf, 1024, "%s:%d Tell me, did you reach here? \n",
           __FILE__, __LINE__);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  bstruct b;

  b = sbrk(0);

  if (sbrk(BLOCK_SIZE + s) == (void *)-1)
    return (NULL);

  b->size = s;
  b->next = NULL;

  if (last)
    last->next = b;

  b->free = 0;
  return (b);
}

void split_block(bstruct b, size_t s)
{
  bstruct new;
  // new = b->data + s;
  new->size = b->size - s - BLOCK_SIZE;
  new->next = b->next;
  new->free = 1;
  b->size = s;
  b->next = new;
}