#include <stdio.h>
#include <unistd.h>

typedef struct header_block
{
  short current_state;
  short size;
  struct header_block *next;
  struct header_block *prev;
} hblock;

__thread void *heap;

__thread hblock *freelist[9];

void print_freelist()
{
  int size = 16;
  for (int i = 0; i < 9; i++)
  {
    fprintf(stderr, "[%d] -> %d: %p\n", i, size, (void *)freelist[i]);
    size *= 2;
  }
}
/* Initialize the heap and freelist */
void init_heap()
{
  int p = 1;
  fprintf(stderr, "#######starting malloc %d\n", p);
  heap = sbrk(sysconf(_SC_PAGESIZE));
  freelist[8] = (hblock *)heap;
  freelist[8]->current_state = 0;
  freelist[8]->size = 4096;
  freelist[8]->next = NULL;
  freelist[8]->prev = NULL;

  print_freelist();
}

int get_pos(size_t needed)
{
  int pos = 0;
  int memory = 16;
  while (memory < needed)
  {
    memory *= 2;
    pos++;
  }

  return pos;
}

void *offset_pointer(hblock *ptr, int offset)
{
  char *offset_ptr = (char *)ptr;
  if (offset)
    return offset_ptr + sizeof(hblock);
  else
    return offset_ptr - sizeof(hblock);
}

void print_block(hblock *block)
{
  fprintf(stderr, "Printing block data\n");
  fprintf(stderr, "address: %p\n", (void *)block);
  fprintf(stderr, "current_sate: %d\n", block->current_state);
  fprintf(stderr, "size: %d\n", block->size);
  fprintf(stderr, "next: %p\n", (void *)block->next);
  fprintf(stderr, "prev: %p\n", (void *)block->prev);
  fprintf(stderr, "\n");
}

__thread int position;
void *malloc(size_t size)
{
  fprintf(stderr, "size %zu\n", size);

  int needed = size + sizeof(hblock);

  if (needed > 4096)
    return NULL;

  if (!heap)
    init_heap();
  if (!heap)
    return NULL;

  position = get_pos(needed);
  int pos = position;
  fprintf(stderr, "pos : %d\n", pos);
  print_freelist();

  hblock *front, *next;

  if (freelist[pos])
  {
    fprintf(stderr, "Yes, pos exists %p\n", freelist[pos]);
    front = freelist[pos];
    next = front->next;

    front->current_state = 1;
    front->next = NULL;
    front->prev = NULL;
    fprintf(stderr, "inside there is a block %d\n", pos);

    if (next)
    {
      next->prev = NULL;
      freelist[pos] = next;
    }
    else
    {
      freelist[pos] = NULL;
    }

    return offset_pointer(front, 1);
  }

  fprintf(stderr, "get pos value%d\n", pos);
  int available = pos;
  while (!freelist[available])
  {
    available++;
  }
  fprintf(stderr, "***%d\n", available);

  if (available == 9)
  {
    hblock *new_heap = sbrk(sysconf(_SC_PAGESIZE));

    if (!new_heap)
      return NULL;

    new_heap->current_state = 0;
    new_heap->size = 4096;
    new_heap->next = NULL;
    new_heap->prev = NULL;

    if (freelist[8])
      freelist[8]->next = new_heap;
    else
      freelist[8] = new_heap;
    available--;
  }
  print_freelist();
  hblock *current, *new;

  fprintf(stderr, "available before while %d\n", available);
  while (available != pos)
  {
    current = freelist[available];
    current->size /= 2; // split in half
    fprintf(stderr, "%d\n", current->size);

    new = (hblock *)((char *)current + current->size);
    new->size = current->size;

    fprintf(stderr, "available before freelist%d\n", available);

    if (freelist && freelist[available]->next)
    {
      freelist[available] = freelist[available]->next;
      freelist[available]->prev = NULL;
    }
    else
    {
      freelist[available] = NULL;
    }

    // Decrement available
    --available;

    current->next = new;
    new->prev = current;
    freelist[available] = current;
    print_block(new);
    print_freelist();
  }

  hblock *ret_meta = freelist[pos];

  if (freelist[pos]->next)
  {
    freelist[pos] = freelist[pos]->next;
    freelist[pos]->prev = NULL;
  }
  else
  {
    freelist[pos] = NULL;
  }

  ret_meta->next = NULL;
  ret_meta->current_state = 1;

  return offset_pointer(ret_meta, 1);
}