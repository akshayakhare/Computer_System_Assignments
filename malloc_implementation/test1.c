#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "block_Header.h"

// void *base = NULL;
int main(int argc, char **argv)
{
  size_t size = 4000;
  size_t size2 = 4;
  size_t size3 = 4000;
  void *mem = malloc(size);
  // printf("holalalalalalalala\n");
  void *mem2 = malloc(size2);
  void *mem3 = malloc(size3);
  // void *mem4 = malloc(size3);
  // printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  // printf("Successfully malloc'd %zu bytes at addr %p\n", size2, mem2);

  // mem = malloc(size);
  assert(mem != NULL);
  // free(mem);
  // printf("Successfully free'd %zu bytes from addr %p\n", size, mem);
  return 0;
}