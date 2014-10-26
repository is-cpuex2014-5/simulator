#include <stdint.h>
#include <stdio.h>

#define INIT_PC        0
#define MEM_SIZE  300000
#define INIT_MEM_ADDR (MEM_SIZE / 3)

//#define HALT 0x8001e000

//---------- for decode
uint32_t cutoutOp(uint32_t op, int h, int t){
  //-- h~t bit in 0-index
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

typedef union{
  uint32_t u;
  char ch[4];
} endian;
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

//---------- main
int main(int argc, char*argv[]){
  // open&read program file
  if(argc<2){
    printf("too few args.");
    return 1;
  }
  FILE *fp;
  uint32_t memory[PROG_SIZE];
  int p_size;
  if((fp=fopen(argv[1], "rb")) == NULL){
    printf("err@opening %s",argv[1]);
    return 1;
  }
  p_size = fread(memory,sizeof(uint32_t),PROG_SIZE,fp);


  // vars
  uint32_t op=0;
  uint32_t rgs[3];
  uint32_t option=0;
  int nextPC=0;
  int imm=0;
  uint32_t irg[16]={}; // int register
  uint32_t frg[16]={}; // float register
  //
  int isDebug=0;
  int end=0;
  //
  int i;
  int t1,t2;
  uint32_t ut1,ut2;
  // initialize
  irg[0]  = 0;             // 0 register
  irg[14] = INIT_MEM_ADDR; // sp
  irg[15] = INIT_PC;       // pc (ip)
  
  // main loop
  while(1){
    //---- fetch
    op = change_endian(memory[irg[15]/4]);

    //-- end with halt(beq r0 r0 r15 0)
    if(op == 0x8001e000){ end = 1; }

    //---- decode & exec
    nextPC = irg[15] + 4;
    switch (cutoutOp(op,0,6)) { //0-6:opcode
      //--- ALU
    case 0b0000000: //add
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]] = irg[rgs[1]] + irg[rgs[2]] + utoi(option,13);
      break;
    case 0b0000001: //addi
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]] = irg[rgs[1]] + utoi(option,17);
      break;
    case 0b0000010: //sub
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]] = irg[rgs[1]] - irg[rgs[2]];
      break;
    case 0b0000011: //subi
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]] = irg[rgs[1]] - utoi(option,17);
      break;
    case 0b0000100: //not
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]] = ~irg[rgs[1]];
      break;
    case 0b0000110: //and
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]] = irg[rgs[1]] & irg[rgs[2]];
      break;
    case 0b0001000: //or
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]] = irg[rgs[1]] | irg[rgs[2]];
      break;
    case 0b0001010: //xor
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]] = irg[rgs[1]] ^ irg[rgs[2]];
      break;
    case 0b0001100: //nand
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]] = ~(irg[rgs[1]] & irg[rgs[2]]);
      break;
    case 0b0001110: //nor
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]] = ~(irg[rgs[1]] | irg[rgs[2]]);
      break;
    case 0b0010000: //shift
      cutoffOp(op,rgs,&option,3);
      imm = utoi(cutoutOp(op,19,23),6);
      ut1 = cutoutOp(op,24,24);
      ut2 = cutoutOp(op,25,26);
      if(ut2==0){
	
      } else if(ut2==1){

      } else if(ut2==3){

      }
      irg[rgs[0]] = irg[rgs[1]] << (irg[rgs[2]] + imm);
      break;
      //--- FLU
    case 0b0100000: //fadd
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = frg[rgs[1]] + frg[rgs[2]];
      break;
    case 0b0100010: //fsub
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = frg[rgs[1]] - frg[rgs[2]];
      break;
    case 0b0100100: //fmul
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = frg[rgs[1]] * frg[rgs[2]];
      break;
    case 0b0100110: //fdiv
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = frg[rgs[1]] / frg[rgs[2]];
      break;
    case 0b0101000: //fsqrt
      cutoffOp(op,rgs,&option,2);
      break;
    case 0b0101010: //ftoi
      cutoffOp(op,rgs,&option,2);
      break;
    case 0b0101100: //itof
      cutoffOp(op,rgs,&option,2);
      break;
    case 0b0101110: //fneg
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]] = -frg[rgs[1]];
      break;
    case 0b0110000: //finv
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]] = 1 / frg[rgs[1]];
      break;
      //--- branch
    case 0b1000000: //beq
      cutoffOp(op,rgs,&option,3);
      if(irg[rgs[0]] == irg[rgs[1]]){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
      break;
    case 0b1000001: //beqi
      cutoffOp(op,rgs,&option,2);
      if(irg[rgs[0]] == irg[rgs[1]]){
	nextPC = utoi(option,17);
      }
      break;
    case 0b1000010: //blt
      cutoffOp(op,rgs,&option,3);
      //printf("%d < %d ??\n",irg[rgs[0]],irg[rgs[1]]);
      if(irg[rgs[0]] < irg[rgs[1]]){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
      //printf("next: %d\n",nextPC);
      break;
    case 0b1000011: //blti
      cutoffOp(op,rgs,&option,2);
      if(irg[rgs[0]] < irg[rgs[1]]){
	nextPC = utoi(option,17);
      }
      break;
    case 0b1000100: //bfeq
      cutoffOp(op,rgs,&option,3);
      if(frg[rgs[0]] == frg[rgs[1]]){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
      break;
    case 0b1000101: //bfeqi
      cutoffOp(op,rgs,&option,2);
      if(frg[rgs[0]] == frg[rgs[1]]){
	nextPC = utoi(option,17);
      }
      break;
    case 0b1000110: //bflt
      cutoffOp(op,rgs,&option,3);
      if(frg[rgs[0]] < frg[rgs[1]]){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
      break;
    case 0b1000111: //bflti
      cutoffOp(op,rgs,&option,2);
      if(frg[rgs[0]] < frg[rgs[1]]){
	nextPC = utoi(option,17);
      }
      break;
      //---- system
    case 0b1100000: //store
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]] = memory[irg[rgs[1]]+utoi(option,17)];
      break;
    case 0b1100010: //load
      cutoffOp(op,rgs,&option,2);
      memory[irg[rgs[1]]+utoi(option,17)] = irg[rgs[0]];
      break;
    case 0b1100100: //fstore
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]] = memory[irg[rgs[1]]+utoi(option,17)];
      break;
    case 0b1100110: //fload
      cutoffOp(op,rgs,&option,2);
      memory[irg[rgs[1]]+utoi(option,17)] = frg[rgs[0]];
      break;
      //---- gomi
    case 0b1111110 : //testcode write
      cutoffOp(op,rgs,&option,1);
      printf("irg%d = %d\n",rgs[0],irg[rgs[0]]);
      break;
    case 0b1111111 : //testcode fwrite
      cutoffOp(op,rgs,&option,1);
      printf("frg%d = %d\n",rgs[0],frg[rgs[0]]);
      break;
    case 0b1111010 : //testcode allwrite
      printf("irg[%d",irg[0]);
      for(i=1;i<16;i++){
	printf(", %d",irg[i]);
      }
      printf("]\n");
      break;
    case 0b1111011 : //testcode fallwrite
      printf("frg[%d",irg[0]);
      for(i=1;i<16;i++){
	printf(", %d",irg[i]);
      }
      printf("]\n");
      break;
    default:
      break;
    }

    //---- end    
    irg[15] = nextPC;
    if(end){ break; }
  }
 
  //
  printf("irg[%d",irg[0]);
  for(i=1;i<16;i++){ printf(", %d",irg[i]); }
  printf("]\n");
  printf("frg[%d",irg[0]);
  for(i=1;i<16;i++){ printf(", %d",irg[i]); }
  printf("]\n");
  //
  fclose(fp);
  return 0;
}
