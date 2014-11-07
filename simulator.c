#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "moromoro.h"
#include "fpus.h"

#define MEM_SIZE  300000 // actually how much?
#define INIT_PC   0
#define INIT_SP  (MEM_SIZE / 3)

typedef union typechanger{
  uint32_t u;
  int32_t i;
  char ch[4];
  float f;
} typechanger;

//#define HALT 0x8001e000

//---------- main
int main(int argc, char*argv[]){
  // open&read program file
  if(argc<2){
    printf("too few args.");
    return 1;
  }
  FILE *fp;
  uint32_t memory[MEM_SIZE]={};
  int p_size;
  if((fp=fopen(argv[1], "rb")) == NULL){
    printf("err@opening %s",argv[1]);
    return 1;
  }
  p_size = fread(memory,sizeof(uint32_t),MEM_SIZE,fp);
  
  
  // 
  uint32_t op=0;
  uint32_t rgs[3];
  uint32_t option=0;
  int nextPC=0;
  uint32_t irg[16]={}; // int register
  uint32_t frg[16]={}; // float register
  int end=0;

  //
  int i,tmp;
  typechanger tc1,tc2,tc3;
  typechanger tcs[16]={};


  //-- debug
  int ifDebug=-1;
  int ifPrintOp=1;
  int breakpoints[10]={};
  int breakflg=0;
  char buf1[120];
  char*buf2;

  //-- options
  //---- 0:print info, 1:count op, 2:native FLU, 
  int optflgs[10]={};

  //-- count used ops
  int usedOpCounter[128]={};
  int execCounter=0;
  //-- native flu
  float n_frg[16]={};


  // initialize
  irg[0]  = 0;       // 0 register
  irg[14] = INIT_SP; // sp
  irg[15] = INIT_PC; // pc (ip)

  // option check
  for(i=0;i<argc;i++){
    // debugger
    if(!strcmp(argv[i],"-d") || !strcmp(argv[i],"--debug")){
      printf("Debugmode. input command or -h for help.\n");
      ifDebug=0;
    } else if(!strcmp(argv[i],"-p") || !strcmp(argv[i],"--printinfo")){
      optflgs[0]=1;
    } else if(!strcmp(argv[i],"-c") || !strcmp(argv[i],"--countop")){
      optflgs[1]=1;
    } else if(!strcmp(argv[i],"-n") || !strcmp(argv[i],"--nativeFPU")){
      optflgs[2]=1;
    }
  }

  
  // main loop
  while(1){
    //---- fetch
    op = change_endian(memory[irg[15]/4]);

    //---- debug
    breakflg=0;
    for(i=0;i<10;i++){
      if(breakpoints[i]!=0 && irg[15]==breakpoints[i]){
	breakflg=1; break;
      }
    }
    if(!ifDebug || breakflg){
      if(ifPrintOp){
	printf("%05d: ",irg[15]);
	//p_binary(op,32);
	print_op(op);
      } ifPrintOp = 1;

      fgets(buf1,100,stdin);
      if(!strcmp(buf1,"\n")){ ifPrintOp = 0; ifDebug = 0; continue; }
      buf2 = strtok(buf1," \n");
      if(!strcmp(buf2,"-h")){
	printf("---help---\n");
	printf("-h        : show this.\n");
	printf("print opt : print 'opt'.\n");
     	printf("   opt    : rg, irg, frg, op, breakpoint, bp.\n");
	printf("list (n)  : show surrounding +-n ops (default&min 5).\n");
	printf("step (n)  : step n (default 1).\n");
	printf("break n   : set breakpoint.\n");
        printf("delete n  : delete nth breakpoint.\n");
	printf("continue  : end debug mode.\n");
	printf("exit      : end simulator.\n");
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"print")){
	buf2 = strtok(NULL," \n");
	if(buf2==NULL || !strcmp(buf2,"rg")){
	  printf("irg[%d",irg[0]);
	  for(i=1;i<16;i++){ printf(", %d",irg[i]); }
	  if(optflgs[2]){ // native FPU
	    printf("]\nn_frg[%f",n_frg[0]);
	    for(i=1;i<16;i++){ printf(", %f",n_frg[i]); }
	  }
	  for(i=0;i<16;i++){ tcs[i].u = frg[i]; }
	  printf("]\nfrg[%f",tcs[0].f);
	  for(i=1;i<16;i++){ printf(", %f",tcs[i].f); }
	  printf("]\n");
	} else if(!strcmp(buf2,"irg")) {
	  printf("irg[%d",irg[0]); for(i=1;i<16;i++){ printf(", %d",irg[i]); } printf("]\n");
	} else if(!strcmp(buf2,"frg")) {
	  if(optflgs[2]){ // native FPU
	    printf("n_frg[%f",n_frg[0]); for(i=1;i<16;i++){ printf(", %f",n_frg[i]); } printf("]\n");
	  }
	  for(i=0;i<16;i++){ tcs[i].u = frg[i]; }
	  printf("frg[%f",tcs[0].f); for(i=1;i<16;i++){ printf(", %f",tcs[i].f); } printf("]\n");
	} else if(!strcmp(buf2,"op")) {
	  printf("%05d: ",irg[15]); print_op(op);
	} else if(!strcmp(buf2,"breakpoint") || !strcmp(buf2,"bp")) {
	  show_array(breakpoints, 10);
	} else {
	  //printf("print what?\n");
	}
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"list")){
	buf2 = strtok(NULL," \n");
	if(buf2==NULL){ tmp=5; }
	else { tmp = max(5,atoi(buf2)); }
	for(i=max(INIT_PC,irg[15]-tmp*4);i<=min(INIT_SP,irg[15]+tmp*4);i+=4){
	  printf("%05d: ",i);
	  print_op(change_endian(memory[i/4]));
	}
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"step")){
	buf2 = strtok(NULL," \n");
	if(buf2==NULL){
	  ifDebug = 1;
	} else {
	  tmp = atoi(buf2);
	  if(tmp>0){ ifDebug = atoi(buf2); }
	  else { ifDebug = 1; }
	}
      } else if(!strcmp(buf2,"break")){
	buf2 = strtok(NULL," \n");
	if(tmp=atoi(buf2)){
	  add_array(breakpoints, tmp, 10);
	  printf("set breakpoint: %d\n",tmp);
	} else { printf("invalid.\n"); }
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"delete")){
	buf2 = strtok(NULL," \n");
	if(tmp=atoi(buf2)){
	  del_array(breakpoints, tmp);
	  printf("delete breakpoint: %d\n",tmp);
	} else { printf("invalid.\n"); }
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"continue")){
	printf("program continue.\n");	
      } else if(!strcmp(buf2,"exit")){
	break;
      } else {
	printf("invalid command.\n");
	ifPrintOp = 0; ifDebug = 0;
	continue;
      }
    }
    if(ifDebug>=0){ ifDebug--; }
    //debug ----
    
    
    //-- end with halt(beq r0 r0 r15 0)
    if(op == 0x8001e000){ end = 1; }
    
    //---- countOp
    usedOpCounter[cutoutOp(op,0,6)]++;
    execCounter++;
    
    //---- decode & exec
    nextPC = irg[15] + 4;
    switch (cutoutOp(op,0,6)) { //0-6:opcode
      //--- ALU
    case 0b0000000: //add
      cutoffOp(op,rgs,&option,3);
      tc1.u = irg[rgs[1]]; tc2.u = irg[rgs[2]]
      tc3.i = tc1.i + tc2.i + utoi(option,13);
      irg[rgs[0]] = tc3.u;
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
      irg[rgs[0]] = shift_(irg[rgs[1]], cutoutOp(op,24,24), cutoutOp(op,25,26), irg[rgs[2]]);
      break;
    case 0b0010001: //shifti
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]] = shift_(irg[rgs[1]], cutoutOp(op,24,24), cutoutOp(op,25,26), utoi(cutoutOp(op,19,23),6));
      break;
      //--- FPU
    case 0b0100000: //fadd
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = fadd(frg[rgs[1]], frg[rgs[2]]);
      n_frg[rgs[0]] = n_frg[rgs[1]] + n_frg[rgs[2]];
      break;
    case 0b0100010: //fsub
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = fsub(frg[rgs[1]], frg[rgs[2]]);
      n_frg[rgs[0]] = n_frg[rgs[1]] - n_frg[rgs[2]];
      break;
    case 0b0100100: //fmul
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = fmul(frg[rgs[1]], frg[rgs[2]]);
      n_frg[rgs[0]] = n_frg[rgs[1]] * n_frg[rgs[2]];
      break;
    case 0b0100110: //fdiv  //not yet
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]] = frg[rgs[1]] / frg[rgs[2]];
      n_frg[rgs[0]] = n_frg[rgs[1]] / n_frg[rgs[2]];
      break;
    case 0b0101000: //fsqrt //not yet
      cutoffOp(op,rgs,&option,2);
      //n_frg[rgs[0]] = sqrtf(n_frg[rgs[1]]);
      break;
    case 0b0101010: //ftoi
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]] = h_floor(frg[rgs[1]]);
      //irg[rgs[0]] = floorf(n_frg[rgs[1]]);
      break;
    case 0b0101100: //itof
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]] = h_i2f(irg[rgs[1]]);
      n_frg[rgs[0]] = (int)irg[rgs[1]];
      break;
    case 0b0101110: //fneg  //not yet
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]] = -frg[rgs[1]];
      n_frg[rgs[0]] = -n_frg[rgs[1]];
      break;
    case 0b0110000: //finv  //not yet
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]] = 1 / frg[rgs[1]];
      n_frg[rgs[0]] = 1 / n_frg[rgs[1]];
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
      if(utoi(irg[rgs[0]],32) < utoi(irg[rgs[1]],32)){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
      break;
    case 0b1000011: //blti
      cutoffOp(op,rgs,&option,2);
      if(utoi(irg[rgs[0]],32) < utoi(irg[rgs[1]],32)){
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
      // bug @ NAN,-0 ?
      if(utoi(irg[rgs[0]],32) < utoi(irg[rgs[1]],32)){
	nextPC = irg[rgs[2]] + utoi(option,13);
      }
      break;
    case 0b1000111: //bflti
      cutoffOp(op,rgs,&option,2);
      if(utoi(irg[rgs[0]],32) < utoi(irg[rgs[1]],32)){
	nextPC = utoi(option,17);
      }
      break;
      //---- system
    case 0b1100000: //load
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]] = memory[irg[rgs[1]]+utoi(option,17)];
      if(rgs[0]==15){
	nextPC = irg[rgs[0]];
      }
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
    case 0b1101000: //loadr
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]] = memory[irg[rgs[1]]+irg[rgs[2]]];
      if(rgs[0]==15){
	nextPC = irg[rgs[0]]; //
      }
      break;
    case 0b1101010: //storer
      cutoffOp(op,rgs,&option,2);
      memory[irg[rgs[1]]+irg[rgs[2]]] = irg[rgs[0]];
      break;
    case 0b1101100: //floadr
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]] = memory[irg[rgs[1]]+irg[rgs[2]]];
      n_frg[rgs[0]] = memory[irg[rgs[1]]+irg[rgs[2]]];      
      break;
    case 0b1101110: //fstorer
      cutoffOp(op,rgs,&option,2);
      memory[irg[rgs[1]]+irg[rgs[2]]] = frg[rgs[0]];
      if(optflgs[2]){ memory[irg[rgs[1]]+irg[rgs[2]]] = n_frg[rgs[0]]; }
      break;
    case 0b1110000: //read
      cutoffOp(op,rgs,&option,1);
      tc1.u = irg[rgs[0]];
      fread(&tc1.ch[3], sizeof(char), 1, stdin);
      irg[rgs[0]] = tc1.u;
      break;
    case 0b1110001: //write
      cutoffOp(op,rgs,&option,1);
      tc1.u = irg[rgs[0]];
      fwrite(&tc1.ch[3], sizeof(char), 1, stdout);
      break;
    default:
      printf("invalid opration??\n");
      break;
    } // --end switch

    //---- end
    irg[15] = nextPC;
    if(end){ break; }
  }
 
  // print infos
  if(optflgs[0]){ // print info
    printf("irg[%d",irg[0]);
    for(i=1;i<16;i++){ printf(", %d",irg[i]); }
    if(optflgs[2]){ // native FPU
      printf("]\nn_frg[%f",n_frg[0]);
      for(i=1;i<16;i++){ printf(", %f",n_frg[i]); }
    }
    for(i=0;i<16;i++){ tcs[i].u = frg[i]; }
    printf("]\nfrg[%f",tcs[0].f);
    for(i=1;i<16;i++){ printf(", %f",tcs[i].f); }
    printf("]\n");
  }
  if(optflgs[1]){ // count op
    printf("%d oprations\n",execCounter);
    print_countOp(usedOpCounter);
  }
  
  //
  fclose(fp);
  return 0;
}
