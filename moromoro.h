#ifndef _MOROMORO_H_
#define _MOROMORO_H_

#include <stdint.h>

uint32_t cutoutOp(uint32_t, int, int);
int cutoffOp(uint32_t, uint32_t*, uint32_t*, int);

typedef union endian{
  uint32_t u;
  char ch[4];
} endian;

uint32_t change_endian(uint32_t);
int utoi(uint32_t, int);
int p_binary(uint32_t, int);
void print_op(uint32_t);
uint32_t shift_(uint32_t, int, int, int);

int max(int, int);
int min(int, int);

int add_array(int*, int, int);
void del_array(int*, int);
void show_array(int*, int);

void print_countOp(int*);

#endif
