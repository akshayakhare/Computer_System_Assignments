#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "block_Header.h"

// void *base = NULL;
int main(int argc, char **argv)
{
  size_t size = 12;
  size_t size2 = 100;
  size_t size3 = 20;
  void *mem = malloc(size);
  // printf("holalalalalalalala\n");
  // printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  void *mem2 = malloc(size2);
  // // printf("Successfully malloc'd %zu bytes at addr %p\n", size2, mem2);
  void *mem3 = malloc(size3);
  // void *mem4 = malloc(size3);
  // printf("Successfully malloc'd %zu bytes at addr %p\n", size3, mem3);

  // mem = malloc(size);
  assert(mem != NULL);
  // free(mem);
  // printf("Successfully free'd %zu bytes from addr %p\n", size, mem);
  return 0;
}