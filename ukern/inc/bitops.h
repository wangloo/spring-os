#pragma once

void set_bit(int nr, unsigned long *p);
void clear_bit(int nr, unsigned long *p);
void change_bit(int nr, unsigned long *p);
int test_bit(int nr, unsigned long *p);
int test_and_set_bit(int nr, unsigned long *p);
int test_and_clear_bit(int nr, unsigned long *p);
int test_and_change_bit(int nr, unsigned long *p);