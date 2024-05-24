#pragma once

#define max(a, b)	(a) > (b) ? (a) : (b)
#define min(a, b)	(a) < (b) ? (a) : (b)

#define align_up(addr, x)   (((addr) + ((x)-1)) & (~((typeof(addr))(x)-1)))
#define align_down(addr, x) ((addr) & (~((typeof(addr))(x)-1)))

#define nelem(x)            (sizeof(x) / sizeof(x[0]))