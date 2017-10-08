#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
//for threads
#include <pthread.h>

//set the debug_flag on or off to print the debug statements
bool debug_flag = false;

pthread_mutex_t mutex_bin_8 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_bin_64 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_bin_512 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_rest = PTHREAD_MUTEX_INITIALIZER;

typedef struct node
{
  size_t size;
  struct node *next;
  int debug;
  //int magic; // For debugging, for now.
} node_f;

//size_t PAGESIZE = sysconf(_SC_PAGESIZE);
size_t PAGESIZE;
// this is a command that works like magic!
// used ot execute a function before main
// very important
_attribute_((constructor)) void get_pagesize()
{

  PAGESIZE = sysconf(_SC_PAGESIZE);
}

int META_SIZE = sizeof(struct node);

//initailly the bins are initalized to null
__thread node_f *bin_8 = NULL;
__thread node_f *bin_64 = NULL;
__thread node_f *bin_512 = NULL;

// all calls above 512 bytes are to be done using mmap
node_f *request_space_from_heap(node_f *head_node, size_t bin_size)
{

  //size_t PAGESIZE = sysconf(_SC_PAGESIZE);
  node_f *block;
  block = sbrk(0); // current location of program break
  void *request = sbrk(PAGESIZE);
  assert((void *)block == request);
  if (request == (void *)-1)
  {
    return NULL; //sbrk failed
  }
  // if the node is NULL of first request
  // I will try remove this as I am only going to request
  // when the lists are empty
  if (!head_node)
  {
    head_node = block;
  }
  int size_of_blocks = META_SIZE + bin_size;
  int no_of_blocks = PAGESIZE / size_of_blocks;
  if (debug_flag)
  {
    printf("no_of_blocks : %d\n", no_of_blocks);
  }
  node_f *current_node = block;
  current_node->debug = 1;

  for (int i = 2; i <= no_of_blocks; i++)
  {

    // This is really important!!
    // change current_node to char* so that we can do addition byte wise
    node_f *temp = (node_f *)((char *)current_node + size_of_blocks);

    current_node->next = temp;
    current_node = temp;
    current_node->debug = i;
    if (debug_flag)
    {
      printf("debug : %d\n", current_node->debug);
    }
  }
  current_node->next = NULL;

  // point it back to original place
  return head_node;
}

void *malloc(size_t size)
{
  if (size <= 8)
  {
    //pthread_mutex_t mutex_bin_8 = PTHREAD_MUTEX_INITIALIZER;
    // lock for this bin only
    pthread_mutex_lock(&mutex_bin_8);
    if (!bin_8) // first call for 8 bytes
    {
      if (debug_flag)
      {
        printf("First call for 8 bytes\n");
      }
      bin_8 = request_space_from_heap(bin_8, 8);
    }
    void space = (char)bin_8 + META_SIZE;
    bin_8->size = 8;
    bin_8 = bin_8->next;
    pthread_mutex_unlock(&mutex_bin_8);
    return space;
  }
  else if (size <= 64)
  {
    pthread_mutex_lock(&mutex_bin_64);
    if (!bin_64) // first call for 8 bytes
    {
      if (debug_flag)
      {
        printf("First call for 64 bytes\n");
      }
      bin_64 = request_space_from_heap(bin_64, 64);
    }
    void space = (char)bin_64 + META_SIZE;
    bin_64->size = 64;
    bin_64 = bin_64->next;
    pthread_mutex_unlock(&mutex_bin_64);
    return space;
  }
  else if (size <= 512)
  {
    pthread_mutex_lock(&mutex_bin_512);
    if (!bin_512) // first call for 8 bytes
    {
      if (debug_flag)
      {
        printf("First call for 512 bytes\n");
      }
      bin_512 = request_space_from_heap(bin_512, 512);
    }
    void space = (char)bin_512 + META_SIZE;
    bin_512->size = 512;
    bin_512 = bin_512->next;
    pthread_mutex_unlock(&mutex_bin_512);
    return space;
  }
  else
  {
    pthread_mutex_lock(&mutex_rest);
    node_f *new_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (new_ptr == MAP_FAILED)
    {
      perror("mmap");
      exit(EXIT_FAILURE);
    }
    void space = (char)new_ptr + META_SIZE;
    new_ptr->size = size;
    pthread_mutex_unlock(&mutex_rest);
    return space;
  }
}

// since we need this at multiple places, a function is defined
node_f *get_node_pointer(void *ptr)
{
  return (node_f *)((char *)ptr - META_SIZE);
}

// freeing the pointer according to size of the bin
void free(void *ptr)
{
  if (!ptr)
    return;

  // getting position of pointer
  node_f *position_of_ptr = get_node_pointer(ptr);
  // adding the pointer to the front
  if (position_of_ptr->size == 8)
  {
    pthread_mutex_lock(&mutex_bin_8);
    if (debug_flag)
    {
      printf("freeing 8 bytes of memory\n");
    }
    position_of_ptr->next = bin_8;
    bin_8 = position_of_ptr;
    pthread_mutex_unlock(&mutex_bin_8);
  }
  else if (position_of_ptr->size == 64)
  {
    pthread_mutex_lock(&mutex_bin_64);
    if (debug_flag)
    {
      printf("freeing 64 bytes of memory\n");
    }
    position_of_ptr->next = bin_64;
    bin_64 = position_of_ptr;
    pthread_mutex_unlock(&mutex_bin_64);
  }
  else if (position_of_ptr->size == 512)
  {
    pthread_mutex_lock(&mutex_bin_512);
    if (debug_flag)
    {
      printf("freeing 512 bytes of memory\n");
    }
    position_of_ptr->next = bin_512;
    bin_512 = position_of_ptr;
    pthread_mutex_unlock(&mutex_bin_512);
  }
  else
  {
    pthread_mutex_lock(&mutex_rest);
    if (debug_flag)
    {
      printf("freeing >512 bytes of memory\n");
    }
    //clearing all the metadata and the size was stored in
    // the structure while creating
    munmap(position_of_ptr, position_of_ptr->size);
    pthread_mutex_unlock(&mutex_rest);
  }
}

// calloc clears the memory before returning the pointer

void *calloc(size_t nmemb, size_t size)
{
  size_t total_size = nmemb * size;
  // what is overflow and how can I check for it
  void *ptr = malloc(total_size);
  // initialize all the allocated bytes to zero
  memset(ptr, 0, size);

  return ptr;
}

void *realloc(void *ptr, size_t size)
{
  if (!ptr)
  {
    //NULL ptr. realloc should act like malloc
    return malloc(size);
  }

  node_f *position_of_ptr = get_node_pointer(ptr);

  if (position_of_ptr->size >= size)
  {
    // we have enough space. Not implementing splits as
    // it will screw with the bins and and waste space
    return ptr;
  }

  // if user asks for more space, then give more space,
  // copy previous data and return the pointer to new space
  void *new_ptr = malloc(size);
  memcpy(new_ptr, ptr, position_of_ptr->size);
  free(ptr);
  return new_ptr;
}

// void main(){
//
//
//     printf("META_SIZE: %d\n", META_SIZE);
//     void* x = malloc(1);
//     void* y = malloc(1);
//     free(x);
//     free(y);
//     //void *x = request_space_from_heap(bin_64, 64);
// }