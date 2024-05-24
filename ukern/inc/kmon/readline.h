
// Buffer for readline
struct linebuf {
  struct buffer *lbuf; // text left to the cursor
  struct buffer *rbuf; // text right to the cursor
  int cursor_pos;
};

struct linebuf *
alloc_linebuf(void);
void
free_linebuf(struct linebuf **lb);
void
linebuf_clear(struct linebuf *lb);
int
linebuf_to_str(struct linebuf *lb, char *str);

int
linebuf_cursor_inc(struct linebuf *lb);
int
linebuf_cursor_dec(struct linebuf *lb);
void
linebuf_cursor_end(struct linebuf *lb);
int
linebuf_insert(struct linebuf *lb, char c);
int
linebuf_del(struct linebuf *lb);

void
init_readline(void);