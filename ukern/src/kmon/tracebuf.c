#include <kernel.h>
#include <kmem.h>
#include <kmon/tracebuf.h>

struct tracebuf *
alloc_tracebuf(size_t size)
{
  struct tracebuf *tb;

  if ((tb = kalloc(sizeof(*tb))) == NULL) {
    return NULL;
  }

  if ((tb->data = kalloc(size)) == NULL) {
    kfree(tb);
    return NULL;
  }
  tb->items = 0;
  tb->read = 0;
  tb->write = 0;
  tb->size = size;
  return tb;
}

void
free_tracebuf(struct tracebuf **tb);

#define roundi(i, size) ((i) % (size))
static int
tracebuf_release_item(struct tracebuf *tb)
{
  int read = tb->read;
  int i, res=0;
  
  for (i = read;;i=roundi(i+1, tb->size)) {
    if (tb->data[i] == 0x0) {
        int j,k,l;
        j = roundi(i+1,tb->size);
        k = roundi(j+1, tb->size);
        l = roundi(k+1, tb->size);
    // LOG_DEBUG("i = %d, j: %d, k: %d\n", i, j, k);
    // LOG_DEBUG("data[j]: 0x%x, data[k]: 0x%x, data[l]: 0x%x\n",
              // tb->data[j], tb->data[k], tb->data[l]);
        if (tb->data[j] == 0x01 &&
            tb->data[k] == 0x02 &&
            tb->data[l] == 0x03) {
          tb->read = roundi(l+1, tb->size);
          tb->cur_size -= res;
          LOG_DEBUG("Release one item, i = %d\n", i);
          return res;
        }
    }
    res += 1;
  }
}

// Return the size have write in
int
tracebuf_insert(struct tracebuf *tb, char *s)
{
  int free, len, tail;
  char *tmps;

  len = strlen(s);
  if (!len || len+4 > tb->size)
    return -1; 
  
  tmps = kalloc(len+4);
  memcpy(tmps, s, len);
  tmps[len++] = 0x0;
  tmps[len++] = 0x1;
  tmps[len++] = 0x2;
  tmps[len++] = 0x3;
    

  if (tb->write >= tb->read) {
    free = tb->size - tb->write + tb->read;
  } else {
    free = tb->read - tb->write;
  }

  LOG_DEBUG("free: %d\n", free);
  while (free < len) {
    // Release an item
    free += tracebuf_release_item(tb);
  }
  
  if (len < tb->size-tb->write) {
    memcpy(tb->data+tb->write, tmps, len);
    tb->write += len;
  } else {
    tail = tb->size - tb->write;
    memcpy(tb->data+tb->write, tmps, tail);
    memcpy(tb->data, tmps+tail, len-tail);
    tb->write = len-tail;
  }
  tb->cur_size += len;
  return len;
}
    

static int
tracebuf_next_item(struct tracebuf *tb, int pos);


// Allocate inside, free outside
// Return length of str
int 
tracebuf_to_str(struct tracebuf *tb, char **s)
{
  int read, res=0;
  char *str;

  if (!tb->cur_size)
    return 0;

  if ((str = kalloc(tb->cur_size)) == NULL) {
    LOG_ERROR("No more memory\n");
    return 0;
  }

  read = tb->read;
  while (read != tb->write) {
    if (tb->data[read] == 0x0 || tb->data[read] == 0x1 ||
        tb->data[read] == 0x2 || tb->data[read] == 0x3) {
    } else {
      str[res++] = tb->data[read];
    }
    read = roundi(read+1, tb->size);
  }

  str[res] = 0;
  *s = str;
  return res;
}
    


void
tracebuf_clear(struct tracebuf *tb)
{
  memset(tb->data, 0, tb->size);
  tb->write = 0;
  tb->read = 0;
  tb->items = 0;
}

// Debug funcs
void
print_tracebuf(struct tracebuf *tb)
{
  for (int i = 0; i < tb->size; i++) 
    printf("%c", tb->data[i]);
  printf("\n");
}

void
print_tracebuf_raw(struct tracebuf *tb)
{
  for (int i = 0; i < tb->size; i++) {
    printf("%2x ", tb->data[i]);
    if ((i+1) % 8 == 0) {
      printf("\n");
    }
  }
}

void
print_tracebuf_info(struct tracebuf *tb)
{
  printf("Tracebuf Info:\nRead: %d, Write: %d, Items: %d, cur_size: %d\n", 
                              tb->read, tb->write, tb->items, tb->cur_size);
}
