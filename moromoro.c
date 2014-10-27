#include <stdint.h>
#include <stdio.h>
#include "moromoro.h"

//-- h~t bit in 0-index
uint32_t cutoutOp(uint32_t op, int h, int t){
  return ((op<<h)>>(h+31-t));
}

//-- opcode(7) | ri..(4n) | option(rest) -> _ | rgs[i] | opt
int cutoffOp(uint32_t op,uint32_t*rgs, uint32_t*opt, int n){
  int i;
  for(i=0;i<n;i++){
    rgs[i] = cutoutOp(op,7+(4*i),10+(4*i));
  }
  *opt = cutoutOp(op,7+(4*n),31);
  return 0;
}

// big endian <-> little endian
uint32_t change_endian(uint32_t u){
  endian e;
  char temp;
  e.u = u;
  temp = e.ch[0];
  e.ch[0] = e.ch[3];
  e.ch[3] = temp;
  temp = e.ch[1];
  e.ch[1] = e.ch[2];
  e.ch[2] = temp;
  return e.u;
}

//-- uint -> int
int utoi(uint32_t u, int digit){
  if(digit<2 || digit>32) {
    printf("error@utoi invalid input.");
    return -1;
  }
  uint32_t s = (u>>(digit-1))&1;
  if(s==1){
    return -(cutoutOp(~u,33-digit,31)+1);
  } else {
    return   cutoutOp( u,33-digit,31);
  }
}

//-- print uint32_t in binary
int p_binary(uint32_t b,int digit){
  int i;
  for(i=digit-1;i>=0;i--){
    printf("%d", (b>>i) & 1);
  }
  printf("\n");
  return 0;
}

//shift u b bits
// 0:l,1:r,  00:arith,01:logic,10:rotate
uint32_t shift_(uint32_t u, int lr, int ty, int b){
  uint32_t tmp;
  if(ty==0){
    if(lr){  //arith-r
      if(u&0x80000000){ // neg
	return (u>>b)|0x80000000;
      } else {
	return u>>b;
      }
    } else { //arith-l
      if(u&0x80000000){ // neg
	return (u<<b)|0x80000000;
      } else {
	return u<<b;
      }
    }
  } else if(ty==1){
    if(lr){  //logic-r
      return u>>b;
    } else { //logic-l
      return u<<b;
    }
  } else if(ty==2){
    if(lr){  //rotate-r
      tmp = cutoutOp(u,32-b,31);
      tmp <<= 32-b;
      return tmp|(u>>b);
    } else { //rotate-l
      tmp = cutoutOp(u,0,b-1);
      return tmp|(u<<b);
    }
  }
  return 0;
}
