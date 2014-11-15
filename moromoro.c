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
    fprintf(stderr, "error@utoi invalid input.");
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
    fprintf(stderr, "%d", (b>>i) & 1);
  }
  fprintf(stderr, "\n");
  return 0;
}
int p_hexadecimal(uint32_t b){
  fprintf(stderr, "%x\n",b);
  return 0;
}

//-- print opration in assembli
void print_op(uint32_t op){
  uint32_t rgs[3];
  uint32_t option=0;
  switch(cutoutOp(op,0,6)){
    //--- ALU
  case 0b0000000: //add
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "add r%d r%d r%d %d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b0000001: //addi
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "addi r%d r%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b0000010: //sub
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "sub r%d r%d r%d\n",rgs[0],rgs[1],rgs[2]); 
    break;
  case 0b0000011: //subi
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "subi r%d r%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b0000100: //not
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "not r%d r%d\n",rgs[0],rgs[1]);
    break;
  case 0b0000110: //and
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "and r%d r%d r%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0001000: //or
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "or r%d r%d r%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0001010: //xor
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "xor r%d r%d r%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0001100: //nand
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "nand r%d r%d r%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0001110: //nor
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "nor r%d r%d r%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0010000: //shift
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "shift r%d r%d r%d",rgs[0],rgs[1],rgs[2]);
    if(cutoutOp(op,24,24)){
      fprintf(stderr, " r");
    } else { fprintf(stderr, " l"); }
    if(cutoutOp(op,25,26)==0){
      fprintf(stderr, "-arith\n");
    } else if(cutoutOp(op,25,26)==1){
      fprintf(stderr, "-logic\n");
    } else {
      fprintf(stderr, "-rotate\n");
    }
    break;
  case 0b0010001: //shifti
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "shifti r%d r%d %d",rgs[0],rgs[1],utoi(cutoutOp(op,19,23),6));
    if(cutoutOp(op,24,24)){
      fprintf(stderr, " r");
    } else { fprintf(stderr, " l"); }
    if(cutoutOp(op,25,26)==0){
      fprintf(stderr, "-arith\n");
    } else if(cutoutOp(op,25,26)==1){
      fprintf(stderr, "-logic\n");
    } else {
      fprintf(stderr, "-rotate\n");
    }
    break;
    //--- FLU
  case 0b0100000: //fadd
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "fadd f%d f%d f%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0100010: //fsub
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "fsub f%d f%d f%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0100100: //fmul
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "fmul f%d f%d f%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0100110: //fdiv
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "fdiv f%d f%d f%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0101000: //fsqrt
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "fsqrt f%d f%d\n",rgs[0],rgs[1]);
    break;
  case 0b0101010: //ftoi
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "ftoi r%d f%d\n",rgs[0],rgs[1]);
    break;
  case 0b0101100: //itof
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "itof f%d r%d\n",rgs[0],rgs[1]);
    break;
  case 0b0101110: //fneg
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "fadd f%d f%d\n",rgs[0],rgs[1]);
    break;
  case 0b0110000: //finv
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "finv f%d f%d\n",rgs[0],rgs[1]);
    break;
    //--- branch
  case 0b1000000: //beq
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "beq r%d r%d r%d %d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b1000001: //beqi
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "beqi r%d r%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1000010: //blt
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "blt r%d r%d r%d %d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b1000011: //blti
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "blti r%d r%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1000100: //bfeq
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "bfeq f%d f%d r%d %d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b1000101: //bfeqi
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "bfeqi f%d f%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1000110: //bflt
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "bflt f%d f%d r%d %d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b1000111: //bflti
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "bflti f%d f%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
    //---- system
  case 0b1100000: //load
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "load r%d r%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1100010: //store
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "store r%d r%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1100100: //fload
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "fload f%d r%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1100110: //fstore
    cutoffOp(op,rgs,&option,2);
    fprintf(stderr, "fstore f%d r%d %d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1101000: //loadr
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "loadr r%d r%d r%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b1101010: //storer
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "storer r%d r%d r%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b1101100: //floadr
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "floadr f%d r%d r%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b1101110: //fstorer
    cutoffOp(op,rgs,&option,3);
    fprintf(stderr, "fstorer f%d r%d r%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b1110000: //read
    cutoffOp(op,rgs,&option,1);
    fprintf(stderr, "read r%d\n",rgs[0]);
    break;
  case 0b1110001: //write
    cutoffOp(op,rgs,&option,1);
    fprintf(stderr, "write r%d\n",rgs[0]);
    break;
  default:
    fprintf(stderr, "invalid op: ");
    p_binary(op,32);
    //p_hexadecimal(op);
    break;
  }
  return;
}

//shift u b bits
// 0:l,1:r,  00:arith,01:logic,10:rotate
uint32_t shift_(uint32_t u, int lr, int ty, int b){
  int i;
  uint32_t tmp;
  if(ty==0){
    if(lr){  //arith-r
      if(u&0x80000000){ // neg
	tmp = u;
	for(i=0;i<b;i++){ tmp = (tmp>>1)|0x80000000; }
	return tmp;
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

int max(int a, int b){ return a>b?a:b; }
int min(int a, int b){ return a<b?a:b; }

// for debugger
int add_array(int*ar, int b, int ln){
  int i,flg=0;
  for(i=0;i<ln;i++){
    if(ar[i]==0){
      ar[i]=b; flg=1; break;
    }
  }
  if(flg){ return 0; }
  else   { return 1; }
}
void del_array(int*ar, int b){
  ar[b-1] = 0;
  return;
}
void show_array(int*ar, int ln){
  int i,flg=0;
  for(i=0;i<ln;i++){
    if(ar[i]){
      fprintf(stderr, "%d: break@ %d\n",i+1,ar[i]);
      flg=1;
    }
  }
  if(!flg){
    fprintf(stderr, "no breakpoints.\n");
  }
  return;
}
// for debugger ----

//---- for op counter
void print_countOp(int*count){
  int flg=1,mid,i;
  while(flg){
    mid=0;
    for(i=0;i<128;i++){
      mid = count[mid]>count[i] ? mid : i;   
    }
    if(count[mid]>0){
      switch (mid){
      case 0b0000000: fprintf(stderr, "add    :"); break;
      case 0b0000001: fprintf(stderr, "addi   :"); break;
      case 0b0000010: fprintf(stderr, "sub    :"); break;
      case 0b0000011: fprintf(stderr, "subi   :"); break;
      case 0b0000100: fprintf(stderr, "not    :"); break;
      case 0b0000110: fprintf(stderr, "and    :"); break;
      case 0b0001000: fprintf(stderr, "or     :"); break;
      case 0b0001010: fprintf(stderr, "xor    :"); break;
      case 0b0001100: fprintf(stderr, "nand   :"); break;
      case 0b0001110: fprintf(stderr, "nor    :"); break;
      case 0b0010000: fprintf(stderr, "shift  :"); break;
      case 0b0010001: fprintf(stderr, "shifti :"); break;
      case 0b0100000: fprintf(stderr, "fadd   :"); break;
      case 0b0100010: fprintf(stderr, "fsub   :"); break;
      case 0b0100100: fprintf(stderr, "fmul   :"); break;
      case 0b0100110: fprintf(stderr, "fdiv   :"); break;
      case 0b0101000: fprintf(stderr, "fsqrt  :"); break;
      case 0b0101010: fprintf(stderr, "ftoi   :"); break;
      case 0b0101100: fprintf(stderr, "itof   :"); break;
      case 0b0101110: fprintf(stderr, "fneg   :"); break;
      case 0b0110000: fprintf(stderr, "finv   :"); break;
      case 0b1000000: fprintf(stderr, "beq    :"); break;
      case 0b1000001: fprintf(stderr, "beqi   :"); break;
      case 0b1000010: fprintf(stderr, "blt    :"); break;
      case 0b1000011: fprintf(stderr, "blti   :"); break;
      case 0b1000100: fprintf(stderr, "bfeq   :"); break;
      case 0b1000101: fprintf(stderr, "bfeqi  :"); break;
      case 0b1000110: fprintf(stderr, "bflt   :"); break;
      case 0b1000111: fprintf(stderr, "bflti  :"); break;
      case 0b1100000: fprintf(stderr, "load   :"); break;
      case 0b1100010: fprintf(stderr, "store  :"); break;
      case 0b1100100: fprintf(stderr, "fload  :"); break;
      case 0b1100110: fprintf(stderr, "fstore :"); break;
      case 0b1110000: fprintf(stderr, "read   :"); break;
      case 0b1110001: fprintf(stderr, "write  :"); break;
      default:        fprintf(stderr, "invalid:"); break;
      }
      fprintf(stderr, " %d times.\n",count[mid]);
      count[mid] = 0;
    } else {
      flg = 0;
    }
  }

  return;
}

//for op counter ----
