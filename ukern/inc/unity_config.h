#include <console.h>

typedef void (*ut_call)(void);
#define __ut_call __section(".__ut_call")
#define __define_ut_call(fn)	\
	static ut_call __ut_call_##fn __used __ut_call = fn


extern unsigned char __start_ut_call;
extern unsigned char __stop_ut_call_used;

#define UNITY_OUTPUT_CHAR(a) console_putc(a)
#define UNITY_OUTPUT_COLOR
#define UNITY_EXCLUDE_SETJMP_H