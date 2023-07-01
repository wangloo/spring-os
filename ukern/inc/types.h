#pragma once

typedef unsigned long   u64;
typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;
typedef signed long     s64;
typedef signed int      s32;
typedef signed short    s16;
typedef signed char     s8;

typedef u32   uint32_t;
typedef s32   int32_t;
typedef u16   uint16_t;
typedef s16   int16_t;
typedef u8    uint8_t;
typedef s8    int8_t;
typedef u64   uint64_t;
typedef s64   int64_t;


typedef long ssize_t;
typedef unsigned long size_t;

typedef unsigned long paddr_t;
typedef unsigned long vaddr_t;


typedef int16_t tid_t;
typedef int16_t pid_t;

typedef unsigned long uintptr_t;
typedef int irqreturn_t;

typedef int bool;

enum {
  false = 0,
  true = 1,
};

#define NULL ((void *)0)

typedef struct {
	int value;
} atomic_t;
