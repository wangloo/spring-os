#pragma once
#include <compiler.h>
#include <types.h>

#define AARCH64_SPSR_EL3h	0b1101		// el3 using sp_el3
#define AARCH64_SPSR_EL3t	0b1100		// el3 using sp_el0
#define AARCH64_SPSR_EL2h	0b1001		// el2 using sp_el2
#define AARCH64_SPSR_EL2t	0b1000		// el2 using sp_el0
#define AARCH64_SPSR_EL1h	0b0101		// el1 using sp_el1
#define AARCH64_SPSR_EL1t	0b0100		// el1 using sp_el0
#define AARCH64_SPSR_EL0t	0b0000		// el0 using sp_el0
#define AARCH64_SPSR_RW		(1 << 4)
#define AARCH64_SPSR_F		(1 << 6)
#define AARCH64_SPSR_I		(1 << 7)
#define AARCH64_SPSR_A		(1 << 8)
#define AARCH64_SPSR_D		(1 << 9)
#define AARCH64_SPSR_IL		(1 << 20)
#define AARCH64_SPSR_SS		(1 << 21)
#define AARCH64_SPSR_V		(1 << 28)
#define AARCH64_SPSR_C		(1 << 29)
#define AARCH64_SPSR_Z		(1 << 30)
#define AARCH64_SPSR_N		(1 << 31)

// ARM64 General propose Registers
struct gp_regs {
  uint64_t x0;
  uint64_t x1;
  uint64_t x2;
  uint64_t x3;
  uint64_t x4;
  uint64_t x5;
  uint64_t x6;
  uint64_t x7;
  uint64_t x8;
  uint64_t x9;
  uint64_t x10;
  uint64_t x11;
  uint64_t x12;
  uint64_t x13;
  uint64_t x14;
  uint64_t x15;
  uint64_t x16;
  uint64_t x17;
  uint64_t x18;
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t x29;
  uint64_t lr;
}__packed;

// Reserved when switching bewteen uspace and kspace
struct context {
    uint64_t sp0;     // Userspace stack pointer
    uint64_t elr;     // Saved PC
    uint64_t spsr;    // Saved processor state
    struct gp_regs gp_regs;
}__packed;




// struct cpu_context {
//   uint64_t tpidr_el0;
//   uint64_t tpidrro_el0;
//   uint64_t ttbr_el0;
// };