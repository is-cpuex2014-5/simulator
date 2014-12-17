#include <stdio.h>
#include <string.h>
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

//-- regard uint(lower digit bits) as int
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
int p_binary(uint32_t b, FILE*fp){
  int i;
  for(i=31;i>=0;i--){
    fprintf(fp, "%d", (b>>i) & 1);
  }
  fprintf(fp, "\n");
  return 0;
}
int p_hexadecimal(uint32_t b){
  fprintf(stderr, "%x\n",b);
  return 0;
}

//-- print opration in assembli
void print_op(uint32_t op){
  disassembl(op,stderr);
  return;
}

void disassembl(uint32_t op, FILE*fp){
  uint32_t rgs[3];
  uint32_t option=0;
  uint32_t lr;
  switch(cutoutOp(op,0,6)){
    //--- ALU
  case 0b0000000: //add
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "add\tr%d\tr%d\tr%d\t%d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b0000001: //addi
    cutoffOp(op,rgs,&option,2);
    if(((op>>16)&1) == 0)
      fprintf(fp, "addil\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(option,16));
    else
      fprintf(fp, "addih\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(option,16));
    break;
  case 0b0000010: //sub
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "sub\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]); 
    break;
  case 0b0000011: //subi
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "subi\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b0000100: //not
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "not\tr%d\tr%d\n",rgs[0],rgs[1]);
    break;
  case 0b0000110: //and
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "and\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0001000: //or
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "or\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0001010: //xor
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "xor\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0001100: //nand
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "nand\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0001110: //nor
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "nor\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0010000: //shift
    cutoffOp(op,rgs,&option,3);
    lr = cutoutOp(op,24,24);
    if(cutoutOp(op,25,26)==0){
      if(lr){ fprintf(fp, "sra\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]); }
      else  { fprintf(fp, "sla\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]); }
    } else if(cutoutOp(op,25,26)==1){
      if(lr){ fprintf(fp, "srl\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]); }
      else  { fprintf(fp, "sll\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]); }
    } else {
      if(lr){ fprintf(fp, "srr\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]); }
      else  { fprintf(fp, "slr\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]); }
    }
    break;
  case 0b0010001: //shifti
    cutoffOp(op,rgs,&option,3);
    lr = cutoutOp(op,24,24);
    if(cutoutOp(op,25,26)==0){
      if(lr){ fprintf(fp, "srai\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(cutoutOp(op,19,23),6)); }
      else  { fprintf(fp, "slai\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(cutoutOp(op,19,23),6)); }
    } else if(cutoutOp(op,25,26)==1){
      if(lr){ fprintf(fp, "srli\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(cutoutOp(op,19,23),6)); }
      else  { fprintf(fp, "slli\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(cutoutOp(op,19,23),6)); }
    } else {
      if(lr){ fprintf(fp, "srri\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(cutoutOp(op,19,23),6)); }
      else  { fprintf(fp, "slri\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(cutoutOp(op,19,23),6)); }
    }
    break;
    //--- FLU
  case 0b0100000: //fadd
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "fadd\tf%d\tf%d\tf%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0100010: //fsub
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "fsub\tf%d\tf%d\tf%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0100100: //fmul
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "fmul\tf%d\tf%d\tf%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0100110: //fdiv
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "fdiv\tf%d\tf%d\tf%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b0101000: //fsqrt
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "fsqrt\tf%d\tf%d\n",rgs[0],rgs[1]);
    break;
  case 0b0101010: //ftoi
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "ftoi\tr%d\tf%d\n",rgs[0],rgs[1]);
    break;
  case 0b0101100: //itof
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "itof\tf%d\tr%d\n",rgs[0],rgs[1]);
    break;
  case 0b0101110: //fneg
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "fadd\tf%d\tf%d\n",rgs[0],rgs[1]);
    break;
  case 0b0110000: //finv
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "finv\tf%d\tf%d\n",rgs[0],rgs[1]);
    break;
    //--- branch
  case 0b1000000: //beq
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "beq\tr%d\tr%d\tr%d\t%d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b1000001: //beqi
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "beqi\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1000010: //blt
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "blt\tr%d\tr%d\tr%d\t%d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b1000011: //blti
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "blti\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1000100: //bfeq
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "bfeq\tf%d\tf%d\tr%d\t%d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b1000101: //bfeqi
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "bfeqi\tf%d\tf%d\t%d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1000110: //bflt
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "bflt\tf%d\tf%d\tr%d\t%d\n",rgs[0],rgs[1],rgs[2],utoi(option,13));
    break;
  case 0b1000111: //bflti
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "bflti\tf%d\tf%d\t%d\n",rgs[0],rgs[1],utoi(option,17));
    break;
    //---- system
  case 0b1100000: //load
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "load\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1100010: //store
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "store\tr%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1100100: //fload
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "fload\tf%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1100110: //fstore
    cutoffOp(op,rgs,&option,2);
    fprintf(fp, "fstore\tf%d\tr%d\t%d\n",rgs[0],rgs[1],utoi(option,17));
    break;
  case 0b1101000: //loadr
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "loadr\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b1101010: //storer
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "storer\tr%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b1101100: //floadr
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "floadr\tf%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b1101110: //fstorer
    cutoffOp(op,rgs,&option,3);
    fprintf(fp, "fstorer\tf%d\tr%d\tr%d\n",rgs[0],rgs[1],rgs[2]);
    break;
  case 0b1110000: //read
    cutoffOp(op,rgs,&option,1);
    fprintf(fp, "read\tr%d\n",rgs[0]);
    break;
  case 0b1110001: //write
    cutoffOp(op,rgs,&option,1);
    fprintf(fp, "write\tr%d\n",rgs[0]);
    break;
  default:
    fprintf(fp, "invalid op: ");
    p_binary(op,fp);
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
void print_countOp(long long int*count){
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
      case 0b1101000: fprintf(stderr, "loadr  :"); break;
      case 0b1101010: fprintf(stderr, "storer :"); break;
      case 0b1101100: fprintf(stderr, "floadr :"); break;
      case 0b1101110: fprintf(stderr, "fstorer:"); break;
      case 0b1110000: fprintf(stderr, "read   :"); break;
      case 0b1110001: fprintf(stderr, "write  :"); break;
      default:        fprintf(stderr, "invalid:"); break;
      }
      fprintf(stderr, " %lld times.\n",count[mid]);
      count[mid] = 0;
    } else {
      flg = 0;
    }
  }

  return;
}

//for op counter ----
