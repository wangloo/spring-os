/* 64-bit bitmap */
#pragma once
#include <utils.h>
#include <utils_arm64.h>
#include <size.h>
#include <types.h>

#define BITMAP_FULL        ((u64)~0)
#define BITMAP_EMPTY       ((u64)0)
#define BITMAP_SIZE(count) (div_round_up((count), WORD_BITS) * sizeof(long))

typedef u64 bitmap_t;

static inline void bitmap_set_bit(bitmap_t *bitmap, u64 index)
{
  unsigned int wi = index / WORD_BITS;
  unsigned int bi = index % WORD_BITS;
  bitmap[wi] |= BIT(bi);
}

static inline void bitmap_clear_bit(bitmap_t *bitmap, u64 index)
{
  unsigned int wi = index / WORD_BITS;
  unsigned int bi = index % WORD_BITS;
  bitmap[wi] &= ~BIT(bi);
}

static inline void bitmap_set_bits(bitmap_t *bitmap, u64 index, int len)
{
  for (int i = 0; i < len; i++)
    bitmap_set_bit(bitmap, index+i);
}

static inline void bitmap_clear_bits(bitmap_t *bitmap, u64 index, int len)
{
  for (int i = 0; i < len; i++)
    bitmap_clear_bit(bitmap, index+i);
}

static inline int bitmap_find_first_0(bitmap_t *bitmap, u64 size)
{
  u64 wsize = size / sizeof(long);
  u64 val, i;
  int pos = 0;

  for (i = 0; i < wsize; i++) {
    if (bitmap[i] != BITMAP_FULL) {
      val = bitmap[i];
      while ((val & 0x1) != 0) {
        val >>= 1;
        pos++;
      }
      break;
    }
    pos += WORD_BITS;
  }
  if (i == wsize) 
    pos = -1; // Not found
  return pos;
}

static inline int 
bitmap_find_next_0(bitmap_t *bitmap, u64 size, u64 start)
{
  u64 wsize = size / sizeof(long);
  u64 wi, bi, word, i;
  int pos;
  
  wi = start / WORD_BITS;
  bi = start % WORD_BITS;

  for (i = wi, pos = start; i < wsize; i++) {
    word = bitmap[i];
    if (word == BITMAP_FULL) {
      pos += WORD_BITS;
      continue;
    }
    if (i == wi && (~(s64)(word >> bi)) == 0) {
      pos += WORD_BITS;
      continue;
    }
    if (i == wi) 
      word >>= bi;
    while (word & 0x1) {
      word >>= 1;
      pos++;
    }
    return pos;
  }
  return -1; // not found
}


static inline int 
bitmap_find_first_1(const bitmap_t *bitmap, u64 size)
{
  u64 wsize = size / sizeof(long);
  int pos = 0;
  u64 i;

  for (i = 0; i < wsize; i++) {
    if (bitmap[i] != BITMAP_EMPTY) {
      pos += ctz(bitmap[i]);
      break;
    }
    pos += WORD_BITS;
  }
  if (i == wsize) 
    pos = -1; // Not found
  return pos;
}

static inline int 
bitmap_find_next_1(const bitmap_t *bitmap, u64 size, u64 start)
{
  u64 wsize = size / sizeof(long);
  u64 wi, bi, word, i;
  int pos;
  
  wi = start / WORD_BITS;
  bi = start % WORD_BITS;

  for (i = wi, pos = start; i < wsize; i++) {
    word = bitmap[i];
    if (word == BITMAP_EMPTY) {
      pos += WORD_BITS;
      continue;
    }
    if (i == wi && (word >>= bi) == 0) {
      pos += WORD_BITS;
      continue;
    }

    pos += ctz(word);
    return pos;
  }
  return -1; // not found
}

// count: number of continuous bit0
static inline int 
bitmap_find_next_0_area(bitmap_t *bitmap, u64 size, u64 start, int count)
{
  u64 wsize = size / sizeof(long);
  u64 end, i;
  int pos;

again:
  pos = bitmap_find_next_0(bitmap, size, start);
  if (pos == -1)
    return -1;

  end = pos + count;
  if (end >= wsize*WORD_BITS)
    return -1; 

  i = bitmap_find_next_1(bitmap, size, start);
  if (i < end) {
    start = i+1;
    goto again;
  }
  return pos;
}


static inline int bitmap_is_empty(const bitmap_t *bitmap, u64 size)
{
  u64 wsize = size / sizeof(long);
  u64 i;
  for (i = 0; i < wsize; i++)
    if (bitmap[i] != BITMAP_EMPTY) 
      return 0;

  return 1;
}

// size: bytes         TODO: bits
static inline void bitmap_init(bitmap_t *bitmap, u64 size, u64 value)
{
  u64 wsize = size / sizeof(long);

  // assert(value == BITMAP_FULL || value == BITMAP_EMPTY);

  for (u64 i = 0; i < wsize; i++) 
    bitmap[i] = value;
}


#define bitmap_for_each_set_bit(bit, addr, size) \
	for ((bit) = bitmap_find_first_1((addr), (size));		\
	     (bit) > 0 && (bit) < (size);					\
	     (bit) = bitmap_find_next_1((addr), (size), (bit) + 1))

#define bitmap_for_each_clear_bit(bit, addr, size) \
	for ((bit) = bitmap_find_first_0((addr), (size));	\
	     (bit) > 0 && (bit) < (size);				\
	     (bit) = bitmap_find_next_0((addr), (size), (bit) + 1))
