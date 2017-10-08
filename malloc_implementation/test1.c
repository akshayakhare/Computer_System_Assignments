#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "block_Header.h"

// void *base = NULL;
int main(int argc, char **argv)
{
  size_t size = 12;
  size_t size2 = 80;
  size_t size3 = 12;
  void *mem = malloc(size);
  // printf("holalalalalalalala\n")
  fprintf(stderr, "MALLOCD mem Size %zu, address %p\n", size, mem);
  fprintf(stderr, "******************OVER*************\n");
  // printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  void *mem2 = malloc(size2);
  fprintf(stderr, "MALLOCD mem2 Size %zu,address %p\n", size2, mem2);
  fprintf(stderr, "*****************OVER*************\n");
  // // printf("Successfully malloc'd %zu bytes at addr %p\n", size2, mem2);
  void *mem3 = malloc(size3);
  fprintf(stderr, "MALLOCD mem3 Size %zu,address %p\n", size3, mem3);
  fprintf(stderr, "*****************OVER*************\n");
  void *mem4 = malloc(size3);
  fprintf(stderr, "MALLOCD Size %zu,address %p\n", size3, mem4);
  fprintf(stderr, "*****************OVER*************\n");
  void *mem5 = malloc(size3);
  fprintf(stderr, "MALLOCD Size %zu,address %p\n", size3, mem5);
  fprintf(stderr, "*****************OVER*************\n");

  void *mem6 = malloc(size2);
  fprintf(stderr, "MALLOCD Size %zu,address %p\n", size2, mem6);
  fprintf(stderr, "*****************OVER*************\n");
  // void *mem4 = malloc(size3);
  // printf("Successfully malloc'd %zu bytes at addr %p\n", size3, mem3);

  // mem = malloc(size);
  assert(mem != NULL);
  free(mem);
  free(mem3);
  // free(mem);
  // printf("Successfully free'd %zu bytes from addr %p\n", size, mem);
  return 0;
}