#include <stdint.h>
#include <stdio.h>
#include "moromoro.h"

#include "fpu_.h"

#define MEM_SIZE  300000
#define INIT_PC   0
#define INIT_SP  (MEM_SIZE / 3)

//#define HALT 0x8001e000

//---------- main
int main(int argc, char*argv[]){
  // open&read program file
  if(argc<2){
    printf("too few args.");
    return 1;
  }
  FILE *fp;
  uint32_t memory[MEM_SIZE];
  int p_size;
  if((fp=fopen(argv[1], "rb")) == NULL){
    printf("err@opening %s",argv[1]);
    return 1;
  }
  p_size = fread(memory,sizeof(uint32_t),MEM_SIZE,fp);
  
  
  // vars
  uint32_t op=0;
  uint32_t rgs[3];
  uint32_t option=0;
  int nextPC=0;
  uint32_t irg[16]={}; // int register
  uint32_t frg[16]={}; // float register
  //
  int isDebug=0;
  int end=0;
  //
  int i;
  // initialize
  irg[0]  = 0;       // 0 register
  irg[14] = INIT_SP; // sp
  irg[15] = INIT_PC; // pc (ip)
  // debug mode
  if(0){
    
  }
  
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
      irg[rgs[0]] = shift_(irg[rgs[1]],
			   cutoutOp(op,24,24),
			   cutoutOp(op,25,26),
			   irg[rgs[2]]+utoi(cutoutOp(op,19,23),6));
      break;
      //--- FLU
    case 0b0100000: //fadd
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = fadd(frg[rgs[1]], frg[rgs[2]]);
      break;
    case 0b0100010: //fsub
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = fsub(frg[rgs[1]], frg[rgs[2]]);
      break;
    case 0b0100100: //fmul
      cutoffOp(op,rgs,&option,3);
      //frg[rgs[0]] = fmul(frg[rgs[1]], frg[rgs[2]]);
      break;
    case 0b0100110: //fdiv  //not yet
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = frg[rgs[1]] / frg[rgs[2]];
      break;
    case 0b0101000: //fsqrt //not yet
      cutoffOp(op,rgs,&option,2);
      break;
    case 0b0101010: //ftoi
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]] = h_floor(frg[rgs[1]]);
      break;
    case 0b0101100: //itof
      cutoffOp(op,rgs,&option,2);
      //frg[rgs[0]] = h_i2f(irg[rgs[1]]);
      break;
    case 0b0101110: //fneg  //not yet
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]] = -frg[rgs[1]];
      break;
    case 0b0110000: //finv  //not yet
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
      if(irg[rgs[0]] < irg[rgs[1]]){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
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
    case 0b1100000: //load
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]] = memory[irg[rgs[1]]+utoi(option,17)];
      break;
    case 0b1100010: //store
      cutoffOp(op,rgs,&option,2);
      memory[irg[rgs[1]]+utoi(option,17)] = irg[rgs[0]];
      break;
    case 0b1100100: //fload
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]] = memory[irg[rgs[1]]+utoi(option,17)];
      break;
    case 0b1100110: //fstore
      cutoffOp(op,rgs,&option,2);
      memory[irg[rgs[1]]+utoi(option,17)] = frg[rgs[0]];
      break;
      /*
	case 0b: //read
	cutoffOp(op,rgs,&option,1);
	break;
	case 0b: //write
	break;
       */
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
    } // --end switch

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
