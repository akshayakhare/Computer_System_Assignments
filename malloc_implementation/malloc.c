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

// typedef struct ListHeader lheader;

// struct Listheader
// {
//   int filled;
//   bheader *blockheader;
// }

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

  bheader *temp, *temp_pair, *temp_handler;
  if (freelist[list_pos] != NULL)
  {
    temp = freelist[list_pos];
    while (temp->filled != 0 && temp->next != NULL)
      temp = temp->next;

    snprintf(buf, 1024, "%s:%d Free list has value temp->size %zu %d %p\n",
             __FILE__, __LINE__, temp->size, temp->filled, temp->next);
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    if (temp && temp->filled == 0)
    {
      temp->filled = 1;
      print_list();
      snprintf(buf, 1024, "-----------------Done-------------\n", list_pos);
      write(STDOUT_FILENO, buf, strlen(buf) + 1);
      return temp;
    }
    //DO SOMETHING
  }

  //Find the appropriate block to start breaking
  int list_check = get_available_index(list_pos);

  snprintf(buf, 1024, "%s:%d list check is %d \n",
           __FILE__, __LINE__, list_check);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  if (list_check == 8)
  {
    bheader *new_block = sbrk(PAGESIZE);
    if (!new_block)
      return NULL;

    new_block->filled = 1;
    new_block->size = 4096;
    new_block->next = NULL;
    new_block->prev = NULL;
    if (freelist[7])
    {
      temp = freelist[7];
      while (temp->next != NULL)
        temp = temp->next;
      temp->next = new_block;
    }

    else
      freelist[7] = new_block;
    print_list();
    snprintf(buf, 1024, "-----------------Done-------------\n", list_pos);
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    return new_block;
  }

  while (list_check != list_pos)
  {
    snprintf(buf, 1024, "\nValue ->?%p %d\n", temp_pair, list_check);
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    bheader *temp_ptr = freelist[list_check];
    // while (temp_ptr->next != NULL /*&& temp_ptr->filled != 0*/)
    // {
    //   temp_ptr = temp_ptr->next;
    // }
    // temp = temp_ptr;
    temp = freelist[list_check];
    temp->size = temp->size / 2;
    temp->filled = 0;
    temp_pair = (bheader *)((char *)temp + temp->size);
    temp_pair->size = temp->size;
    temp_pair->filled = 0;

    snprintf(buf, 1024, "PRiNT IT before!!!\n");
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    print_list();
    // The case for the whole block, when block is complete 4096 bytes
    if (freelist[list_check]->next == NULL /*&& list_check == 7*/)
    {
      // snprintf(buf, 1024, "Should be reaching here once!!! %d\n", list_check);
      // write(STDOUT_FILENO, buf, strlen(buf) + 1);
      freelist[list_check] = NULL;
    }
    else
    {
      // freelist[list_check]->prev = freelist[list_check];
      // temp_handler = freelist[list_check];
      // while (temp_handler->next != NULL)
      // {
      //   temp_handler = temp_handler->next;
      // }
      // temp_handler->next =
      freelist[list_check] = freelist[list_check]->next;
      // snprintf(buf, 1024, "freelist orig!!! %p\n", freelist[list_check]->next);
      // write(STDOUT_FILENO, buf, strlen(buf) + 1);
      freelist[list_check]->next = NULL;
      freelist[list_check]->prev = NULL;
      // freelist[list_check]->next = freelist[list_check];
    }
    snprintf(buf, 1024, "PRiNT IT!!!\n");
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    print_list();
    temp->next = temp_pair;
    // temp_pair->prev = temp;
    // temp_pair->prev = NULL; experimenting
    temp_pair->prev = temp;
    list_check = list_check - 1;

    temp_handler = freelist[list_check];
    while (temp_handler && temp_handler->next != NULL)
      temp_handler = temp_handler->next;
    if (!temp_handler)
    {
      temp_handler = temp;
      freelist[list_check] = temp_handler;
    }
    else
    {
      temp_handler->next = temp;
      temp->prev = temp_handler;
    }
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

// int get_available_index(int pos)
// {
//   int i;
//   for (i = pos; i <= 8; i++)
//   {
//     if (freelist[i] != NULL)
//       return i;
//   }
//   return i;
// }

int get_available_index(int pos)
{
  int i;
  bheader *get_temp;
  for (i = pos; i < 8; i++)
  {
    get_temp = freelist[i];
    if (get_temp == NULL)
      continue;
    while (get_temp != NULL && get_temp->filled != 0)
      get_temp = get_temp->next;
    if (get_temp != NULL && get_temp->filled == 0)
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
    snprintf(buf, 1024, "freelist[%d]: %d: %p :", i, val,
             freelist[i]);
    write(STDOUT_FILENO, buf, strlen(buf) + 1);

    if (freelist[i])
    {
      bheader *temp = freelist[i];
      while (temp != NULL)
      {
        snprintf(buf, 1024, " %p n:%p p:%p %d -->",
                 temp, temp->next,
                 temp->prev, temp->filled);
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
        temp = temp->next;
      }
    }
    snprintf(buf, 1024, "\n");
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

/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////
/////////////////////////////

void *free(void *mem)
{
  //check for valid address
  if (base)
  {
    // fprintf(stderr, "here???/\n%p %p ", base, mem);
    if (mem < base || mem > sbrk(0))
    {
      fprintf(stderr, "ghanta chalega\n");
      return 0;
    }
  }

  bheader *temp;
  for (int i = 0; i <= 7; i++)
  {
    temp = freelist[i];
    while (temp != NULL)
    {
      if (temp == (bheader *)mem)
      {
        temp->filled = 0;
        print_list();
        merge(i, temp);
        return;
      }
      temp = temp->next;
    }
  }
}

void merge(int i, bheader *node)
{
  size_t size = freelist[i]->size;
  bheader *previous = node->prev;
  bheader *next = node->next;
  bheader *buddy;
  bheader *temp_buddy;
  bheader *buddy_left;
  bheader *buddy_right;

  fprintf(stderr, "previous %p, division %d\n", previous, ((size_t)node % 128));
  if ((size_t)previous % freelist[i + 1]->size)
  {
    buddy_left = previous;
    buddy_right = node;
  }
  else if ((size_t)next % freelist[i + 1]->size)
  {
    buddy_left = node;
    buddy_right = next;
  }
  else
  {
    return;
  }

  //Handles the current list
  if (buddy_left->filled == 0 && buddy_right->filled == 0)
  {
    if (buddy_left->prev == NULL && buddy_right->next == NULL)
      freelist[i] = NULL;
    else if (buddy_left->prev == NULL)
    {
      freelist[i] = buddy_right->next;
      freelist[i]->prev = NULL;
    }
    else if (buddy_right->next == NULL)
    {
      buddy_left->prev->next = NULL;
    }
    else
    {

      buddy_right->next->prev = buddy_left->prev;
      buddy_left->prev->next = buddy_right->next;
    }
  }

  buddy = buddy_left;
  buddy->size = buddy->size * 2;
  buddy->filled = 0;
  //After merging, handles next list
  //First find the appropriate place
  temp_buddy = freelist[i + 1];
  if (temp_buddy == NULL)
  {
    freelist[i + 1] = buddy;
    buddy->next = NULL;
    buddy->prev = NULL;
  }
  while (temp_buddy->next != NULL && temp_buddy < buddy)
  {
    temp_buddy = temp_buddy->next;
  }
  fprintf(stderr, "is temp+buddy printing %p %p %d\n", temp_buddy, buddy,
          (size_t)temp_buddy > (size_t)buddy);
  if (temp_buddy->next == NULL && (size_t)temp_buddy > (size_t)buddy)
  {

    buddy->next = temp_buddy;
    buddy->prev = temp_buddy->prev;
    if (temp_buddy->prev == NULL)
    {

      freelist[i + 1] = buddy;
    }
    temp_buddy->prev = buddy;
  }
  else
  {
    fprintf(stderr, "Entering 2\n");
    buddy->prev = temp_buddy;
    buddy->next = temp_buddy->next;
    if (temp_buddy->next != NULL)
      temp_buddy->next->prev = buddy;
    temp_buddy->next = buddy;
    // buddy->prev = temp_buddy;
    // buddy->next = NULL;
  }
  print_list();
}