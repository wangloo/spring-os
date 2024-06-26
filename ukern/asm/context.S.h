
#define S_X0 0 /* offsetof(struct pt_regs, regs[0])	// */
#define S_X1 8 /* offsetof(struct pt_regs, regs[1])	// */
#define S_X2 16 /* offsetof(struct pt_regs, regs[2])	// */
#define S_X3 24 /* offsetof(struct pt_regs, regs[3])	// */
#define S_X4 32 /* offsetof(struct pt_regs, regs[4])	// */
#define S_X5 40 /* offsetof(struct pt_regs, regs[5])	// */
#define S_X6 48 /* offsetof(struct pt_regs, regs[6])	// */
#define S_X7 56 /* offsetof(struct pt_regs, regs[7])	// */
#define S_X8 64 /* offsetof(struct pt_regs, regs[8])	// */
#define S_X10 80 /* offsetof(struct pt_regs, regs[10])	// */
#define S_X12 96 /* offsetof(struct pt_regs, regs[12])	// */
#define S_X14 112 /* offsetof(struct pt_regs, regs[14])	// */
#define S_X16 128 /* offsetof(struct pt_regs, regs[16])	// */
#define S_X18 144 /* offsetof(struct pt_regs, regs[18])	// */
#define S_X20 160 /* offsetof(struct pt_regs, regs[20])	// */
#define S_X22 176 /* offsetof(struct pt_regs, regs[22])	// */
#define S_X24 192 /* offsetof(struct pt_regs, regs[24])	// */
#define S_X26 208 /* offsetof(struct pt_regs, regs[26])	// */
#define S_X28 224 /* offsetof(struct pt_regs, regs[28])	// */
#define S_FP 232 /* offsetof(struct pt_regs, regs[29])	// */
#define S_LR 240 /* offsetof(struct pt_regs, regs[30])	// */
#define S_FRAME_SIZE 248 /* sizeof(struct pt_regs)	// */

// Before
// [h]------------- < sp
// [l]-------------
//
// After
// [h]----x30------ 
//        ...     
// [l]----x0------- < sp
.macro SAVE_GP_REGS
   sub sp, sp, #S_FRAME_SIZE

   stp x0, x1, [sp, #16 * 0]
   stp x2, x3, [sp, #16 * 1]
   stp x4, x5, [sp, #16 * 2]
   stp x6, x7, [sp, #16 * 3]
   stp x8, x9, [sp, #16 * 4]
   stp x10, x11, [sp, #16 * 5]
   stp x12, x13, [sp, #16 * 6]
   stp x14, x15, [sp, #16 * 7]
   stp x16, x17, [sp, #16 * 8]
   stp x18, x19, [sp, #16 * 9]
   stp x20, x21, [sp, #16 * 10]
   stp x22, x23, [sp, #16 * 11]
   stp x24, x25, [sp, #16 * 12]
   stp x26, x27, [sp, #16 * 13]
   stp x28, x29, [sp, #16 * 14]
   str lr, [sp, #S_LR]
.endm

.macro LOAD_GP_REGS
  ldp x0, x1, [sp, #16 * 0]
  ldp x2, x3, [sp, #16 * 1]
  ldp x4, x5, [sp, #16 * 2]
  ldp x6, x7, [sp, #16 * 3]
  ldp x8, x9, [sp, #16 * 4]
  ldp x10, x11, [sp, #16 * 5]
  ldp x12, x13, [sp, #16 * 6]
  ldp x14, x15, [sp, #16 * 7]
  ldp x16, x17, [sp, #16 * 8]
  ldp x18, x19, [sp, #16 * 9]
  ldp x20, x21, [sp, #16 * 10]
  ldp x22, x23, [sp, #16 * 11]
  ldp x24, x25, [sp, #16 * 12]
  ldp x26, x27, [sp, #16 * 13]
  ldp x28, x29, [sp, #16 * 14]
  ldr lr, [sp, #S_LR]

  add sp, sp, #S_FRAME_SIZE
.endm

// Before
// [h]------------- < sp
// [l]-------------
//
// After
// [h]----x30------ 
//        ...
//        x0
//        spsr_el1
//        elr_el1  
//        sp_el0
// [l]----sp------- < sp
.macro SAVE_CTX
    SAVE_GP_REGS

    // Gp regs are saved, use them freely
    mrs x21, spsr_el1
    mrs x22, elr_el1
    mrs x23, sp_el0
    add x24, sp, #S_FRAME_SIZE // Restore accurate sp
    // mov x24, sp
    str x21, [sp, #-8]!
    str x22, [sp, #-8]!
    str x23, [sp, #-8]!
    str x24, [sp, #-8]!
.endm

.macro LOAD_CTX
    ldr x24, [sp], 8   // sp
    ldr x23, [sp], 8   // sp_el0
    ldr x22, [sp], 8   // elr_el1
    ldr x21, [sp], 8   // spsr_el1
    msr spsr_el1, x21
    msr elr_el1, x22
    msr sp_el0, x23
    LOAD_GP_REGS
.endm

// Save contex including exception info registers
// Before
// [h]------------- < sp
// [l]-------------
//
// After
// [h]----x30------ 
//        ...
//        x0
//        spsr_el1
//        elr_el1  
//        sp_el0
//        sp
//        esr_el1
// [l]----far_el1--- < sp
.macro SAVE_ECTX
    SAVE_CTX

  mrs x22, esr_el1
  mrs x23, far_el1
  stp x22, x23, [sp, #-16]!
.endm

.macro LOAD_ECTX
  // 只是与SAVE动作配合保证栈指针正确，这些表示异常信息的寄存器没必要恢复
  add sp, sp, #16
  LOAD_CTX
.endm


