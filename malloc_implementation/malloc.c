#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

void initialize_heap();
void print_list();
int get_available_index(int list_pos);
typedef struct BlockHeader bheader;
struct BlockHeader
{
  size_t size;
  int filled;
  struct BlockHeader *next;
  struct BlockHeader *prev;
};

#define BLOCK_SIZE sizeof(struct BlockHeader)
size_t PAGESIZE;

bheader *base = NULL;

// We take size as 8 because we start with 32, 64, 128 till 4096
bheader *freelist[8];

//2^5 is 32 that's why we need to add 5 to the array index of freelists
//elements, enabling them to handle the base 2 arithmetic
int LOG_OFFSET_HANDLER = 5;

__attribute__((constructor)) void get_pagesize()
{
  PAGESIZE = sysconf(_SC_PAGESIZE);
}

void *malloc(size_t size)
{
  char buf[1024];
  size_t header_size = sizeof(bheader);
  size_t actual_size = header_size + size;

  if (base == NULL)
  {
    initialize_heap();
  }

  // If size too big to handle, return NULL
  if (actual_size > PAGESIZE)
  {
    return NULL;
  }

  int list_pos = log2(actual_size) - LOG_OFFSET_HANDLER + 1;

  snprintf(buf, 1024, "%s:%d size is %zu and list_pos is %d \n",
           __FILE__, __LINE__, actual_size, list_pos);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  bheader *temp, *temp_pair;
  if (freelist[list_pos] != NULL)
  {
    temp = freelist[list_pos];
    snprintf(buf, 1024, "%s:%d Free list has value temp->size %zu %d %p\n",
             __FILE__, __LINE__, temp->size, temp->filled, temp->next);
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    while (temp->filled != 0 && temp->next != NULL)
      temp = temp->next;
    if (temp && temp->filled == 0)
    {
      temp->filled = 1;
      print_list();
      return temp;
    }
    //DO SOMETHING
  }

  //Find the appropriate block to start breaking
  int list_check = get_available_index(list_pos);

  while (list_check != list_pos)
  {
    snprintf(buf, 1024, "\nValue ->?%p %d\n", temp_pair, list_check);
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    temp = freelist[list_check];
    temp->size = temp->size / 2;
    temp->filled = 0;
    temp_pair = (bheader *)((char *)temp + temp->size);
    temp_pair->size = temp->size;
    temp_pair->filled = 0;

    // The case for the whole block, when block is complete 4096 bytes
    if (freelist[list_check]->next == NULL)
    {
      snprintf(buf, 1024, "Should be reaching here once!!! %d\n", list_check);
      write(STDOUT_FILENO, buf, strlen(buf) + 1);
      freelist[list_check] = NULL;
    }
    else
    {
      freelist[list_check]->prev = freelist[list_check];
      freelist[list_check] = freelist[list_check]->next;
    }
    temp->next = temp_pair;
    temp_pair->prev = temp;
    list_check = list_check - 1;
    freelist[list_check] = temp;
    print_list();
  }

  // Now we are at the point where we found the right block, and we
  // need to assign it.

  temp = freelist[list_pos];
  while (temp->filled != 0 || temp->next != NULL)
    temp = temp->next;
  bheader *actual_block = temp;
  actual_block->filled = 1;
  snprintf(buf, 1024, "freelist[]: pos %d, %p %p %zu %d\n",
           list_pos, actual_block->next, actual_block->prev, actual_block->size,
           actual_block->filled);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  if (freelist[list_pos]->next == NULL)
  {
    freelist[list_pos] = NULL;
  }
  // else
  // {
  //   freelist[list_pos]->prev = freelist[list_pos];
  //   freelist[list_pos] = freelist[list_pos]->next;
  // }

  // actual_block->next = NULL;
  // actual_block->filled = 1;

  print_list();

  snprintf(buf, 1024, "-----------------Done-------------\n", list_pos);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  return actual_block;
}

int get_available_index(int pos)
{
  int i;
  for (i = pos; i <= 8; i++)
  {
    if (freelist[i] != NULL)
      return i;
  }
  return i;
}

//Prints the  whole list, line by line
void print_list()
{
  char buf[1024];
  snprintf(buf, 1024, "\nindex :      range: Address \n");
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  int val = 32;

  for (int i = 0; i < 8; i++)
  {
    // snprintf(buf, 1024, "freelist[%d]: %d: %p %zu %p %p %d\n", i, val,
    //          freelist[i], freelist[i]->size, freelist[i]->next,
    //          freelist[i]->prev, freelist[i]->filled);
    snprintf(buf, 1024, "freelist[%d]: %d: %p  \n", i, val,
             freelist[i]);
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    val = val * 2;
  }
}

//Should be initialized in the first call
void initialize_heap()
{
  base = (bheader *)sbrk(PAGESIZE);
  freelist[7] = base;
  freelist[7]->size = PAGESIZE;
  freelist[7]->filled = 0;
  freelist[7]->next = NULL;
  freelist[7]->prev = NULL;
}