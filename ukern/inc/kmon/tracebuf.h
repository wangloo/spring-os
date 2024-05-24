
struct tracebuf {
  char *data;
  int read;
  int write;
  int items;
  int cur_size;
  int size;
};

struct tracebuf *
alloc_tracebuf(size_t size);
void
free_tracebuf(struct tracebuf **tb);

int
tracebuf_insert(struct tracebuf *tb, char *s);

int 
tracebuf_to_str(struct tracebuf *tb, char **s);

void
tracebuf_clear(struct tracebuf *tb);
void
print_tracebuf(struct tracebuf *tb);
void
print_tracebuf_raw(struct tracebuf *tb);
void
print_tracebuf_info(struct tracebuf *tb);
