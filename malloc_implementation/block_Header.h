#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct BlockStruct *bstruct;
struct BlockStruct
{
  size_t size;
  // struct BlockStruct *left;
  // struct BlockStruct *right;
  struct BlockStruct *next;
  struct BlockStruct *prev;
  int free;
  // char data[1];
};

#define BLOCK_SIZE sizeof(struct BlockStruct)
size_t PAGESIZE;

void *base = NULL;
__attribute__((constructor)) void get_pagesize()
{

  PAGESIZE = sysconf(_SC_PAGESIZE);
}

bstruct find_block(bstruct *last, size_t s)
{
  char buf[1024];
  bstruct b = base;

  while (b && !(b->free && b->size >= s))
  {
    snprintf(buf, 1024, "%s:%d Find block loop at addr: %p, b->size: %zu \n",
             __FILE__, __LINE__, b, b->size);
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    *last = b;
    b = b->next;
  }
  snprintf(buf, 1024, "%s:%d Find block after while b: %p\n",
           __FILE__, __LINE__, b);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  return (b);
}

bstruct extend_heap(bstruct last, size_t s)
{
  char buf[1024];
  bstruct b, new;

  b = sbrk(0);

  if (sbrk(PAGESIZE) == (void *)-1)
    return (NULL);

  b->size = s + BLOCK_SIZE;
  b->next = NULL;
  snprintf(buf, 1024, "%s:%d Extend block with base: %p and b->size: %zu\n",
           __FILE__, __LINE__, base, b->size);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  if (last)
    last->next = b;

  b->free = 0;
  new = (void *)b + b->size;
  new->size = PAGESIZE - b->size;
  new->next = NULL;
  new->prev = b;
  new->free = 1;
  b->next = new;
  snprintf(buf, 1024, "%s:%d Extend_Block at end values-> b: %p, new: %p, new->prev %p,new size: %zu, b->size: %zu \n",
           __FILE__, __LINE__, b, new, new->prev, new->size, b->size);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  if (b == base)
  {
    b->prev == NULL;
  }
  return (b);
}

void split_block(bstruct b, size_t s)
{
  char buf[1024];
  snprintf(buf, 1024, "%s:%d split_block at b: %p, b->size: %zu \n",
           __FILE__, __LINE__, b, b->size);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  bstruct new;
  new = (void *)b + BLOCK_SIZE + s;
  snprintf(buf, 1024, "%s:%d split_block b: %p, new: %p, b->size - (BLOCK_SIZE + s): %zu, b->size %zu \n",
           __FILE__, __LINE__, b, new, (BLOCK_SIZE + s), b->size);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  new->size = (size_t)(b->size - (BLOCK_SIZE + s));
  b->size = BLOCK_SIZE + s;
  b->free = 0;
  new->free = 1;
  new->next = b->next;
  b->next = new;
  new->prev = b;

  snprintf(buf, 1024, "%s:%d split_block b: %p, new: %p, new->prev: %p\n",
           __FILE__, __LINE__, b, new, new->prev);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
}

/*int valid_addr(void *p)
{
  if (base)
  {
    if (p > base &&)
  }
}*/