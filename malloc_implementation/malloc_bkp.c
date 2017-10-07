#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "block_Header.h"
// #include "blockArea.o"

#define align8(x) (((((x)-1) >> 3) << 3) + 8)

typedef struct MallocHeader
{
  size_t size;
} MallocHeader;

void *calloc(size_t nmemb, size_t size)
{
  return NULL;
}

// void *buddyAlloc(size_t size)
// {
//   memset();
// }

void *malloc(size_t size)
{
  bstruct bs, current;
  char buf[1024];
  if (size <= 0)
  {
    printf("Size cannot be zero or less \n");
    exit(1);
  }
  // TODO: Validate size. I guess done?
  size_t s = align8(size);
  snprintf(buf, 1024, "%s:%d Beginning of malloc-> base: %p %d %d\n",
           __FILE__, __LINE__, base, PAGESIZE, BLOCK_SIZE);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  if (base)
  {
    current = base;
    bs = find_block(&current, s);
    // snprintf(buf, 1024, "%s:%d bs is %p, size  \n",
    //          __FILE__, __LINE__, bs);
    // write(STDOUT_FILENO, buf, strlen(buf) + 1);
    if (bs)
    {
      snprintf(buf, 1024, "%s:%d bs SIZE IS  is %zu, block suze is %d  and size %zu, and diff %d \n",
               __FILE__, __LINE__, (bs->size), (BLOCK_SIZE), s, ((int)bs->size - ((int)BLOCK_SIZE + (int)s)));
      write(STDOUT_FILENO, buf, strlen(buf) + 1);
      //we split
      if (((int)bs->size - ((int)BLOCK_SIZE + (int)s)) >= ((int)BLOCK_SIZE))
      {
        split_block(bs, s);
      }
      else
      {
        bs = extend_heap(base, s);
        if (!bs)
          return (NULL);
      }
      // bs->free = 0;
    }
    else
    {
      bs = extend_heap(base, s);
      if (!bs)
        return (NULL);
    }
  }
  else
  {
    // snprintf(buf, 1024, "%s:%d I just need to see something \n",
    //          __FILE__, __LINE__);
    // write(STDOUT_FILENO, buf, strlen(buf) + 1);
    bs = extend_heap(NULL, s);
    // snprintf(buf, 1024, "%s:%d Error before this \n",
    //          __FILE__, __LINE__);
    // write(STDOUT_FILENO, buf, strlen(buf) + 1);
    if (!bs)
      return NULL;
    base = bs;
  }
  return bs;

  void *block;
  block = sbrk(0);

  if (sbrk(s + sizeof(bstruct)) == (void *)-1)
    return NULL;
  // char buf2[1024];
  // size_t allocSize = size + sizeof(MallocHeader);

  snprintf(buf, 1024, "%s:%d block all... %p and other things %ld and size %ld and also %ld \n",
           __FILE__, __LINE__, block, sizeof(block), s, s + sizeof(bstruct));
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  return block;

  // void *ret = mmap(block, allocSize, PROT_READ | PROT_WRITE,
  //                  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  // assert(ret != MAP_FAILED);

  // // We can't use printf here because printf internally calls `malloc` and thus
  // // we'll get into an infinite recursion leading to a segfault.
  // // Instead, we first write the message into a string and then use the `write`
  // // system call to display it on the console.
  // snprintf(buf2, 1024, "%s:%d malloc(%zu): Allocated %zu bytes at %p\n",
  //          __FILE__, __LINE__, size, allocSize, ret);
  // write(STDOUT_FILENO, buf2, strlen(buf2) + 1);

  // MallocHeader *hdr = (MallocHeader *)ret;
  // hdr->size = allocSize;

  // return ret + sizeof(MallocHeader);
}

void free(void *ptr)
{
  MallocHeader *hdr = ptr - sizeof(MallocHeader);
  // We can't use printf here because printf internally calls `malloc` and thus
  // we'll get into an infinite recursion leading to a segfault.
  // Instead, we first write the message into a string and then use the `write`
  // system call to display it on the console.
  char buf[1024];
  snprintf(buf, 1024, "%s:%d free(%p): Freeing %zu bytes from %p\n",
           __FILE__, __LINE__, ptr, hdr->size, hdr);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  munmap(hdr, hdr->size);
}

void *realloc(void *ptr, size_t size)
{
  // Allocate new memory (if needed) and copy the bits from old location to new.

  return NULL;
}
