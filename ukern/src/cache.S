#include <asm/asm_macro.S.h>

.global inv_dcache_all_boot
.global flush_dcache_all_boot
.global flush_cache_all_boot

asmfunc inv_dcache_all_boot
  // From the ARM ARMv8-A Architecture Reference Manual
  dmb ish                   // ensure all prior inner-shareable accesses have been observed
  mrs x0, CLIDR_EL1
  and w3, w0, #0x07000000   // get 2 x level of coherence
  lsr w3, w3, #23
  cbz w3, finished
  mov w10, #0               // w10 = 2 x cache level
  mov w8, #1                // w8 = constant 0b1
loop_level:
  add w2, w10, w10, lsr #1  // calculate 3 x cache level
  lsr w1, w0, w2            // extract 3-bit cache type for this level
  and w1, w1, #0x7
  cmp w1, #2
  b.lt  next_level            // no data or unified cache at this level
  msr CSSELR_EL1, x10       // select this cache level
  isb           // synchronize change of csselr
  mrs x1, CCSIDR_EL1        // read ccsidr
  and w2, w1, #7            // w2 = log2(linelen)-4
  add w2, w2, #4            // w2 = log2(linelen)
  ubfx  w4, w1, #3, #10       // w4 = max way number, right aligned
  clz w5, w4                // w5 = 32-log2(ways), bit position of way in dc operand
  lsl w9, w4, w5            // w9 = max way number, aligned to position in dc operand
  lsl w16, w8, w5           // w16 = amount to decrement way number per iteration
loop_way:
  ubfx  w7, w1, #13, #15      // w7 = max set number, right aligned
  lsl w7, w7, w2            // w7 = max set number, aligned to position in dc operand
  lsl w17, w8, w2           // w17 = amount to decrement set number per iteration
loop_set:
  orr w11, w10, w9          // w11 = combine way number and cache number ...
  orr w11, w11, w7          // ... and set number for dc operand
  dc  isw, x11              // do data cache invalidate by set and way
  subs  w7, w7, w17           // decrement set number
  b.ge  loop_set
  subs  x9, x9, x16           // decrement way number
  b.ge  loop_way
next_level:
  add w10, w10, #2          // increment 2 x cache level
  cmp w3, w10
  b.gt  loop_level
  dsb sy                    // ensure completion of previous cache maintenance operation
  isb
finished:
  ret
endfunc inv_dcache_all_boot

asmfunc flush_dcache_all_boot
  dmb sy        // ensure ordering with previous memory accesses
  mrs x0, clidr_el1     // read clidr
  and x3, x0, #0x7000000    // extract loc from clidr
  lsr x3, x3, #23     // left align loc bit field
  cbz x3, finish      // if loc is 0, then no need to clean
  mov x10, #0       // start clean at cache level 0
loop4:
  add x2, x10, x10, lsr #1    // work out 3x current cache level
  lsr x1, x0, x2      // extract cache type bits from clidr
  and x1, x1, #7      // mask of the bits for current cache only
  cmp x1, #2        // see what cache we have at this level
  b.lt  skip        // skip if no cache, or just i-cache
  mrs x9, daif
  msr daifset, #2
  msr csselr_el1, x10     // select current cache level in csselr
  isb         // isb to sych the new cssr&csidr
  mrs x1, ccsidr_el1      // read the new ccsidr
  msr daif, x9
  and x2, x1, #7      // extract the length of the cache lines
  add x2, x2, #4      // add 4 (line length offset)
  mov x4, #0x3ff
  and x4, x4, x1, lsr #3    // find maximum number on the way size
  clz w5, w4        // find bit position of way size increment
  mov x7, #0x7fff
  and x7, x7, x1, lsr #13   // extract max number of the index size
loop2:
  mov x9, x4        // create working copy of max way size
loop3:
  lsl x6, x9, x5
  orr x11, x10, x6      // factor way and cache number into x11
  lsl x6, x7, x2
  orr x11, x11, x6      // factor index number into x11
  dc  cisw, x11     // clean & invalidate by set/way
  subs  x9, x9, #1      // decrement the way
  b.ge  loop3
  subs  x7, x7, #1      // decrement the index
  b.ge  loop2
skip:
  add x10, x10, #2      // increment cache number
  cmp x3, x10
  b.gt  loop4
finish:
  mov x10, #0       // swith back to cache level 0
  msr csselr_el1, x10     // select current cache level in csselr
  dsb sy
  isb
  ret
endfunc flush_dcache_all_boot

/*
 *  flush_cache_all()
 *
 *  Flush the entire cache system.  The data cache flush is now achieved
 *  using atomic clean / invalidates working outwards from L1 cache. This
 *  is done using Set/Way based cache maintainance instructions.  The
 *  instruction cache can still be invalidated back to the point of
 *  unification in a single instruction.
 */
asmfunc flush_cache_all_boot
  mov x12, x30
  bl  flush_dcache_all_boot
  mov x0, #0
  ic  ialluis       // I+BTB cache invalidate
  ret x12
endfunc flush_cache_all_boot

