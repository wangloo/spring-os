#include <kernel.h>
#include <kmem.h>
#include <kmon/buffer.h>

struct buffer *
alloc_buffer(unsigned long size)
{
  struct buffer *b;

  if ((b = kalloc(sizeof(*b))) == NULL) {
    return NULL;
  }
  if ((b->buf = kalloc(size)) == NULL) {
    kfree(b);
    return NULL;
  }
  memset(b->buf, 0, size); // for debug only
  b->size = size;
  b->cur_size = 0;
  return b;
}

void 
free_buffer(struct buffer **b)
{
  kfree((*b)->buf);
  kfree(*b);
  *b = 0;
}

void
buffer_clear(struct buffer *b)
{
  assert(b);
  memset(b->buf, 0, b->size);
  b->cur_size = 0;
}
  
int
buffer_is_empty(struct buffer *buf)
{
  return buf->cur_size == 0;
}

int
buffer_is_full(struct buffer *buf)
{
  return buf->cur_size == buf->size;
}

void
print_buffer(struct buffer *b)
{
  for (int i = 0; i < b->size; i++) 
    printf("%c",b->buf[i]);
  printf("\n");
}

void
print_buffer_info(struct buffer *b)
{
  printf("Buffer Info:\nCurrent Szie: %d\nMax Size: %d\n", b->cur_size, b->size);
}