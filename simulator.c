#include <stdint.h>
#include <stdio.h>

#define PROG_SIZE 1024
#define INIT_PC 0
#define INIT_MEM_ADDR 0;


//---------- fetch func
int fetchOp(uint32_t*buf,FILE fp,int pc){
  fseek(fp,pc,SEEK_SET);
  //fseek(fp,(offset-1),SEEK_CUR);
  int r = fread(buf,4,1,fp);
  return r;
}
//---------- for decode
uint32_t cutoutOp(uint32_t op, int h, int t){
  //0..h..0ret0..31-t..0
  return ((a<<h)>>(31-h-t));
}
int cutoffOp(uint32_t op,uint32_t*rgs, uint32_t opt, int n){
  int i;
  for(i=0;i<n;i++){
    rgs[i] = cutoutOp(op,7+ 4*i,10+ 4*i);
  }
  opt = cutoutOp(op,7+4*n,31);
}
//---------- for exec
int utoi(uint32_t u, int digit){
  
  return 0;
}

//---------- main
int main(int argc, char*argv){
  // open program file
  if(argc<2){
    printf("too few args.");
    return 1;
  }
  FILE fp;
  if((fp=fopen(argv[1], "r")) == NULL){
    printf("err@opening %s",argv[1]);
    return 1;
  }

  // vars
  uint32_t op;
  uint32_t opcode;
  uint32_t rgs[3];
  uint32_t option;
  int nextPC=0;
  int imm=0;
  uint32_t irg[16]={}; // int register
  uint32_t frg[16]={}; // float register
  // 
  int temp1,temp2;
  // initialize
  irg[0]  = 0;             // 0 register
  irg[14] = INIT_MEM_ADDR; // sp
  irg[15] = INIT_PC;       // pc (ip)

  // main loop
  while(true){
    //---- fetch
    fseek(fp,pc,SEEK_SET);
    //-- fseek(fp,(offset-1),SEEK_CUR);
    fread(op,4,1,fp);

    //---- decode & exec
    nextPC = irg[15] + 4;
    opcode = cutoutOp(op,0,6);
    switch (opcode) {
      //--- ALU
    case 0b0000000: //add
      cutoffOp(op,rgs,option,3); //n:rgs
      irg[rgs[0]] = irg[rgs[1]] + irg[rgs[2]] + utoi(option,13);;
      break;
    case 0b0000001: //addi
      cutoffOp(op,rgs,option,2);
      irg[rgs[0]] = irg[rgs[1]] + utoi(option,17);
      break;
    case 0b0000010: //sub
      cutoffOp(op,rgs,option,3);
      irg[rgs[0]] = irg[rgs[1]] - irg[rgs[2]];
      break;
    case 0b0000011: //subi
      cutoffOp(op,rgs,option,2);
      irg[rgs[0]] = irg[rgs[1]] - utoi(option,17);
      break;
    case 0b0000100: //not
      cutoffOp(op,rgs,option,2);
      irg[rgs[0]] = ~irg[rgs[1]];
      break;
    case 0b0000110: //and
      cutoffOp(op,rgs,option,3);
      irg[rgs[0]] = irg[rgs[1]] & irg[rgs[2]];
      break;
    case 0b0001000: //or
      cutoffOp(op,rgs,option,3);
      irg[rgs[0]] = irg[rgs[1]] | irg[rgs[2]];
      break;
    case 0b0001010: //xor
      cutoffOp(op,rgs,option,3);
      irg[rgs[0]] = irg[rgs[1]] ^ irg[rgs[2]];
      break;
    case 0b0001100: //nand
      cutoffOp(op,rgs,option,3);
      irg[rgs[0]] = ~(irg[rgs[1]] & irg[rgs[2]]);
      break;
    case 0b0001110: //nor
      cutoffOp(op,rgs,option,3);
      irg[rgs[0]] = ~(irg[rgs[1]] | irg[rgs[2]]);
      break;
    case 0b0010000: //shift
      cutoffOp(op,rgs,option,2);
      break;
      //--- FLU
    case 0b0100000: //fadd
      cutoffOp(op,rgs,option,3);
      frg[rgs[0]] = frg[rgs[1]] + frg[rgs[2]];
      break;
    case 0b0100010: //fsub
      cutoffOp(op,rgs,option,3);
      frg[rgs[0]] = frg[rgs[1]] - frg[rgs[2]];
      break;
    case 0b0100100: //fmul
      cutoffOp(op,rgs,option,3);
      frg[rgs[0]] = frg[rgs[1]] * frg[rgs[2]];
      break;
    case 0b0100110: //fdiv
      cutoffOp(op,rgs,option,3);
      frg[rgs[0]] = frg[rgs[1]] / frg[rgs[2]];
      break;
    case 0b0101000: //fsqrt
      cutoffOp(op,rgs,option,2);
      break;
    case 0b0101010: //ftoi
      cutoffOp(op,rgs,option,2);
      break;
    case 0b0101100: //itof
      cutoffOp(op,rgs,option,2);
      break;
    case 0b0101110: //fneg
      cutoffOp(op,rgs,option,2);
      frg[rgs[0]] = -frg[rgs[1]];
      break;
    case 0b0110000: //finv
      cutoffOp(op,rgs,option,2);
      frg[rgs[0]] = 1 / frg[rgs[1]];
      break;
      //--- branch
    case 0b1000000: //beq
      cutoffOp(op,rgs,option,3);
      if(irg[rgs[0]] == irg[rgs[1]]){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
      break;
    case 0b1000001: //beqi
      cutoffOp(op,rgs,option,2);
      if(irg[rgs[0]] == irg[rgs[1]]){
	nextPC = utoi(option,17);
      }
      break;
    case 0b1000010: //blt
      cutoffOp(op,rgs,option,3);
      if(irg[rgs[0]] < irg[rgs[1]]){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }      
      break;
    case 0b1000011: //blti
      cutoffOp(op,rgs,option,2);
      if(irg[rgs[0]] < irg[rgs[1]]){
	nextPC = utoi(option,17);
      }
      break;
    case 0b1000100: //bfeq
      cutoffOp(op,rgs,option,3);
      if(frg[rgs[0]] == frg[rgs[1]]){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
      break;
    case 0b1000101: //bfeqi
      cutoffOp(op,rgs,option,2);
      if(frg[rgs[0]] == frg[rgs[1]]){
	nextPC = utoi(option,17);
      }
      break;
    case 0b1000110: //bflt
      cutoffOp(op,rgs,option,3);
      if(frg[rgs[0]] < frg[rgs[1]]){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
      break;
    case 0b1000111: //bflti
      cutoffOp(op,rgs,option,2);
      if(frg[rgs[0]] < frg[rgs[1]]){
	nextPC = utoi(option,17);
      }
      break;
      //---- system
    
    default:
      break;
    }
    //---- end

  }
 
  fclose(fp);
  return 0;
}
