/**
 * @file kmem.h
 * @author your name (you@domain.com)
 * @brief  内核中动态申请、释放内存的接口
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <types.h>

void *kalloc(size_t size);
void kfree(void *addr);