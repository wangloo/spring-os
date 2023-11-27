typedef int (*init_call)(void);


// This sections are defined to put different kinds of
// function pointer, and call them at right time
#define __init_0	__section(".__init_func_0")
#define __init_1	__section(".__init_func_1")
#define __init_2	__section(".__init_func_2")
#define __init_3	__section(".__init_func_3")
#define __init_4	__section(".__init_func_4")
#define __init_5	__section(".__init_func_5")
#define __init_6	__section(".__init_func_6")
#define __init_7	__section(".__init_func_7")
#define __init_8	__section(".__init_func_8")
#define __init_9	__section(".__init_func_9")

extern unsigned char __init_func_0_start;
extern unsigned char __init_func_1_start;
extern unsigned char __init_func_2_start;
extern unsigned char __init_func_3_start;
extern unsigned char __init_func_4_start;
extern unsigned char __init_func_5_start;
extern unsigned char __init_func_6_start;
extern unsigned char __init_func_7_start;
extern unsigned char __init_func_8_start;
extern unsigned char __init_func_9_start;
extern unsigned char __init_func_end;
extern void log_init(void);


#define __define_initcall(fn, id)	\
	static init_call __init_call_##fn __used __init_##id = fn

#define early_initcall(fn)     __define_initcall(fn, 0)
#define uspace_initcall(fn)    __define_initcall(fn, 5)


void init_uspace(void);