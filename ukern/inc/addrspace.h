#pragma once


#define ptov(addr)	((unsigned long)addr)
#define vtop(addr)	((unsigned long)addr)
#define __va(va)	((unsigned long)(va))
#define is_kva(va)	((unsigned long)va >= 4096)