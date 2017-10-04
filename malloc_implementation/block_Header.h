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
  // snprintf(buf, 1024, "%s:%d base in find _block %p\n",
  //          __FILE__, __LINE__, base);
  // write(STDOUT_FILENO, buf, strlen(buf) + 1));
  bstruct b = base;

  while (b && !(b->free && b->size >= s))
  {
    snprintf(buf, 1024, "%s:%d eNTERED  in hte loop  link %p, size %zu \n",
             __FILE__, __LINE__, b, b->size);
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    *last = b;
    b = b->next;
  }
  snprintf(buf, 1024, "%s:%d Inside find_block? AND B %p\n",
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

  b->size = s + BLOCK_SIZE + 8;
  b->next = NULL;
  snprintf(buf, 1024, "%s:%d Extend block with base %p and size %zu\n",
           __FILE__, __LINE__, base, b->size);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  if (last)
    last->next = b;

  b->free = 0;
  new = (void *)b + b->size;
  // snprintf(buf, 1024, "%s:%d Extend block EXPERIMENTATION!!! %p and new %p\n",
  //          __FILE__, __LINE__, base, new);
  // write(STDOUT_FILENO, buf, strlen(buf) + 1);
  new->size = PAGESIZE - b->size;
  new->next = NULL;
  new->free = 1;
  b->next = new;
  snprintf(buf, 1024, "%s:%d extend_block values-> b %p, new %p,new size %zu, b->size %zu \n",
           __FILE__, __LINE__, b, new, new->size, b->size);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  return (b);
}

void split_block(bstruct b, size_t s)
{
  char buf[1024];
  snprintf(buf, 1024, "%s:%d split_block part 1 b %p, b->size %zu \n",
           __FILE__, __LINE__, b, b->size);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  bstruct new;
  new = (void *)b + BLOCK_SIZE + s + 8;
  snprintf(buf, 1024, "%s:%d split_block b %p, new %p, b->size - (BLOCK_SIZE + s) %zu, b->size %zu \n",
           __FILE__, __LINE__, b, new, (BLOCK_SIZE + s + 8), b->size);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  new->size = (size_t)(b->size - (BLOCK_SIZE + s + 8));
  b->size = BLOCK_SIZE + s;
  b->free = 0;
  new->free = 1;
  // new = b->data + s;
  // new->size = b->size - s - BLOCK_SIZE;
  // new->next = b->next;
  // new->free = 1;
  // b->size = s;
  b->next = new;
  new->next = NULL;
}