#pragma once

#include <ctx.h>

struct _esr_t {
  uint32_t iss : 25;
  uint32_t il  : 1;
  uint32_t ec  : 6;
  uint32_t res : 32;
} __packed;

typedef struct _esr_t esr_t;

struct _exception_ctx {
  u64 sp_elx;
  u64 sctlr;
  u64 far;
  esr_t esr;
  gp_regs gregs;
};

typedef struct _exception_ctx exception_ctx;

