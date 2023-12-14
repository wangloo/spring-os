

// Dynamic array buffer
struct buffer {
  char *buf;
  unsigned long size;
  unsigned long cur_size;
};


struct buffer *
alloc_buffer(unsigned long size);
void 
free_buffer(struct buffer **buf);
void
buffer_clear(struct buffer *buf);
int
buffer_is_empty(struct buffer *buf);
int
buffer_is_full(struct buffer *buf);


// Debug
void
print_buffer(struct buffer *b);
void
print_buffer_info(struct buffer *b);