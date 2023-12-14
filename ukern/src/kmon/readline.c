#include <kernel.h>
#include <console.h>
#include <kmem.h>
#include <kmon/buffer.h>
#include <kmon/readline.h>

#define BUFLEN 512
static char linestr[BUFLEN];
static int init_ok = 0;
static struct linebuf *readlbuf = 0;


#define cursorforwd(x) printf("\033[%dC", (x))
#define cursorbackwd(x) printf("\033[%dD", (x))
static inline
backspace(int x)
{
  for (int i = 0; i < x; i++)
    console_puts("\b \b", 3);
}


#define EOF -1
// Asynchronous recv, define EOF
static int
getch(char *ch)
{
  char c;

  if ((c = console_getc()) == 0) 
    return EOF;

  *ch = c;
  return 0;
}

  
#define KEY_ESCAPE  0x001b
enum editorKey
{
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

static int
readkey(void)
{
  char c;
  
  // Not sure: async or sync?
  getch(&c);
  // while (getch(&c) == EOF) {}

  if (c == KEY_ESCAPE) {
    char seq[3];

    if (getch(&seq[0]) == EOF) return KEY_ESCAPE;
    if (getch(&seq[1]) == EOF) return KEY_ESCAPE;
    
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (getch(&seq[2]) == EOF) return KEY_ESCAPE;
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }
    }
    
    return KEY_ESCAPE;
  } else {
    return c;
  }
}


void
init_readline(void)
{
  if (init_ok)
    return;
  
  readlbuf = alloc_linebuf();
  if (!readlbuf) {
    LOG_ERROR("Error on init linebuf\n");
    return ;
  }
  init_ok = 1;
}
  
// Return null-terminated line str
char *
readline(const char *prompt)
{
  // char c;
  int c;

  init_readline();
  printf("%s", prompt);
  while (1) {
    c = readkey();

    if (c == '\b' || c == '\x7f') { // Backspace
      linebuf_del(readlbuf);
    } else if (c == '\n' || c == '\r') { // End of line
      console_putc('\n');
      linebuf_to_str(readlbuf, linestr);
      break;
    } else if (c == ARROW_LEFT) {
      linebuf_cursor_dec(readlbuf);
    } else if (c == ARROW_RIGHT) {
      linebuf_cursor_inc(readlbuf);
    } else if (c == ARROW_UP) {
    } else if (c == ARROW_DOWN) {
    } else if (c >= ' ') {
      linebuf_insert(readlbuf, c);
    } else {  // Error
      LOG_ERROR("Errno in getchar, c = 0x%x\n",c);
      break;
    }

    // printf("==> Input %c\n", c);
    // printf("cursor_pos: %d\n", readlbuf->cursor_pos);
    // print_buffer_info(readlbuf->lbuf);
    // print_buffer(readlbuf->lbuf);
    // print_buffer_info(readlbuf->rbuf);
    // print_buffer(readlbuf->rbuf);
  }
  linebuf_clear(readlbuf);
  return linestr;
}

struct linebuf *
alloc_linebuf(void)
{
  struct linebuf *lb;

  if ((lb = kalloc(sizeof(*lb))) == NULL)
    return NULL;

  lb->lbuf = alloc_buffer(128);
  lb->rbuf = alloc_buffer(128);
  if (!lb->lbuf || !lb->rbuf) {
    kfree(lb);
    return NULL;
  }
  lb->cursor_pos = 0;
  return lb;
}

void
free_linebuf(struct linebuf **lb)
{
  free_buffer(&((*lb)->rbuf));
  free_buffer(&((*lb)->lbuf));
  *lb = 0;
}

void
linebuf_clear(struct linebuf *lb)
{
  buffer_clear(lb->lbuf);
  buffer_clear(lb->rbuf);
  lb->cursor_pos = 0;
}

int
linebuf_to_str(struct linebuf *lb, char *str)
{
  int size;
  
  size = lb->lbuf->cur_size + lb->rbuf->cur_size;
  assert(size < BUFLEN);

  memcpy(str, lb->lbuf->buf, lb->lbuf->cur_size);
  memcpy(str+lb->lbuf->cur_size, lb->rbuf->buf, lb->rbuf->cur_size);
  str[size] = 0;
  return 0;
}

int
linebuf_cursor_inc(struct linebuf *lb)
{
  if (lb->cursor_pos < lb->rbuf->cur_size + lb->lbuf->cur_size) {
    lb->cursor_pos += 1;
    cursorforwd(1);
  }
  return 0;
}
int
linebuf_cursor_dec(struct linebuf *lb)
{
  if (lb->cursor_pos > 0) {
    lb->cursor_pos -= 1;
    cursorbackwd(1);
  }
  return 0;
}

// Cursor inc/dec only move lb->cursor_pos,
// sync lbuf and rbuf to lb->cursor_pos
static void
linebuf_sync(struct linebuf *lb)
{
  int i, off;

  off = lb->lbuf->cur_size - lb->cursor_pos;
  if (off > 0) { // cursor left shift
    for (i = lb->rbuf->cur_size-1; i >= 0; i--) 
      lb->rbuf->buf[i+off] = lb->rbuf->buf[i];
    for (i = 0; i < off; i++) 
      lb->rbuf->buf[i] = lb->lbuf->buf[lb->cursor_pos+i];

    lb->rbuf->cur_size += off;
    lb->lbuf->cur_size -= off;
  } else if (off < 0) { // cursor has right shifted
    off = -off;
    for (i = 0; i < off; i++) 
      lb->lbuf->buf[lb->lbuf->cur_size+i] = lb->rbuf->buf[i];
    for (i = 0; i < lb->rbuf->cur_size-off; i++) 
      lb->rbuf->buf[i] = lb->rbuf->buf[i+off];

    lb->rbuf->cur_size -= off;
    lb->lbuf->cur_size += off;
  }
}

int
linebuf_insert(struct linebuf *lb, char c)
{
  int i;

  linebuf_sync(lb);
  assert(lb->lbuf->cur_size == lb->cursor_pos);

  lb->lbuf->buf[lb->lbuf->cur_size] = c;
  lb->lbuf->cur_size += 1;
  lb->cursor_pos += 1;

  // Show changed content on terminal
  // aabb|cc(Insert 'd') -> aabbd|c -> aabbcdd| -> aabbd|cc
  console_putc(c);
  for (i = 0; i < lb->rbuf->cur_size; i++) 
    console_putc(lb->rbuf->buf[i]);
  if (lb->rbuf->cur_size > 0)
    cursorbackwd(lb->rbuf->cur_size);
    
  return 0;
}

int
linebuf_del(struct linebuf *lb)
{
  int i;

  linebuf_sync(lb);
  assert(lb->lbuf->cur_size == lb->cursor_pos);

  if (lb->cursor_pos > 0) {
    lb->cursor_pos -= 1;
    lb->lbuf->cur_size -= 1;

    // Show changed content on terminal
    // aabb|cc(Backspace) -> aab| -> aabcc| -> aab|cc
    if (lb->rbuf->cur_size > 0) cursorforwd(lb->rbuf->cur_size);
    backspace(lb->rbuf->cur_size + 1);

    for (i = 0; i < lb->rbuf->cur_size; i++)
      console_putc(lb->rbuf->buf[i]);

    if (lb->rbuf->cur_size > 0) cursorbackwd(lb->rbuf->cur_size);
  }
  return 0;
}