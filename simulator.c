#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>

#include "moromoro.h"
#include "fpu.h"

#define MEM_SIZE  300000 // actually how much?
#define INIT_PC   0
#define INIT_SP  (MEM_SIZE   / 3)
#define INIT_HP  (MEM_SIZE*2 / 3)
//#define HALT 0x8001e000

typedef union typechanger{
  uint32_t u;
  int32_t i;
  char ch[4];
  float f;
} typechanger;

uint32_t memory[MEM_SIZE]={};

//---------- main
int main(int argc, char*argv[]){
  // open&read program file
  if(argc<2){
    fprintf(stderr, "too few args.");
    return 1;
  }
  FILE *fp;
  int p_size;
  int i;
  if((fp=fopen(argv[1], "rb")) == NULL){
    fprintf(stderr, "err@opening %s\n",argv[1]);
    return 1;
  }
  p_size = fread(memory,sizeof(uint32_t),MEM_SIZE,fp);
  for(i = 0; i<p_size; i++) memory[i] = change_endian(memory[i]);


  // vars
  uint32_t op=0;
  uint32_t rgs[3];
  uint32_t option=0;
  int nextPC=0;
  typechanger irg[16]={}; // int register
  typechanger frg[16]={}; // float register
  FILE *input, *output;

  //
  int tmp;

  //-- debug
  int ifDebug=-1;
  int ifPrintOp=1;
  int breakpoints[10]={};
  int breakflg=0;
  char buf1[120];
  char*buf2;

  //-- options
  //---- 0:print rg, 1:print total exec, 2:count op,
  //---- 3:native FPU, 4:exist debug input,
  int optflgs[10]={};

  //-- count used ops
  int usedOpCounter[128]={};
  int execCounter=0;
  //-- native flu
  typechanger n_frg[16]={};


  // initialize
  irg[0].i  = 0;       // 0 register
  irg[13].i = INIT_HP; // heap pointer
  irg[14].i = INIT_SP; // sp
  irg[15].i = INIT_PC; // pc (ip)
  input  = stdin;      // from where get input
  output = stdout;     //

  //---- option check
  int c;
  int optionIndex;
  static struct option long_options[]={
    {"countop",   no_argument,       0, 'c'},
    {"debug",     optional_argument, 0, 'd'},
    {"nativeFPU", optional_argument, 0, 'n'},
    {"printinfo", no_argument,       0, 'p'},
    {0,           0,                 0, 0}
  };
  while(1){
    c = getopt_long_only(argc,argv,"cd::n::p",
			 long_options, &optionIndex);
    if(c<0){ break; }
    switch(c){
    case 0: break;
    case 'c':  // countop
      optflgs[1] = optflgs[2] = 1;
      break;
    case 'd':  // debug
      fprintf(stderr, "Debugmode.\n");
      if(optarg){
	fprintf(stderr, "get input from: %s\n", optarg);
	if((input=fopen(optarg,"rb"))==NULL){
	  fprintf(stderr, "err@opening %s\n", optarg);
	  return 1;
	}
	optflgs[4]=1;
      }
      fprintf(stderr, "input command or -h for help.\n");
      ifDebug=0;
      break;
    case 'n':  // nativeFPU
      optflgs[3] = 1;
      if(optarg){
	fprintf(stderr, "%s\n", optarg);
	fprintf(stderr, "opt-arg is not yet implemented.\n");
      }
      break;
    case 'p':  // printinfo
      optflgs[0] = optflgs[1] = 1;
      break;
    case '?':
      fprintf(stderr, "? unknown option: %c ?\n",c);
      break;
    }
  }
  // option check end ----
  
  // main loop
  while(1){
    //---- fetch
    op = memory[irg[15].i/4];

    //---- debug
    breakflg=0;
    for(i=0;i<10;i++){
      if(breakpoints[i]!=0 && irg[15].i==breakpoints[i]){
	breakflg=1; break;
      }
    }
    if(!ifDebug || breakflg){
      if(ifPrintOp){
	fprintf(stderr, "%05d: ",irg[15].i);
	//p_binary(op,32);
	print_op(op);
      } ifPrintOp = 1;

      fgets(buf1,100,stdin);
      if(!strcmp(buf1,"\n")){ ifPrintOp = 0; ifDebug = 0; continue; }
      buf2 = strtok(buf1," \n");
      if(!strcmp(buf2,"-h")){
	fprintf(stderr, "---help---\n");
	fprintf(stderr, "-h        : show this.\n");
	fprintf(stderr, "print opt : print 'opt'.\n");
     	fprintf(stderr, "   opt    : rg, irg, frg, op, breakpoint(bp), exectime(et).\n");
	fprintf(stderr, "list (n)  : show surrounding +-n ops (default&min 5).\n");
	fprintf(stderr, "step (n)  : step n (default 1).\n");
	fprintf(stderr, "break n   : set breakpoint.\n");
        fprintf(stderr, "delete n  : delete nth breakpoint. check print bp\n");
	fprintf(stderr, "continue  : jump to next breakpoint or end.\n");
	fprintf(stderr, "exit      : end simulator.\n");
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"print") || !strcmp(buf2,"p")){
	buf2 = strtok(NULL," \n");
	if(buf2==NULL || !strcmp(buf2,"rg")){
	  fprintf(stderr, "irg[%d",irg[0].i);
	  for(i=1;i<16;i++){ fprintf(stderr, ", %d",irg[i].i); }
	  if(optflgs[3]){ // native FPU
	    fprintf(stderr, "]\nn_frg[%f",n_frg[0].f);
	    for(i=1;i<16;i++){ fprintf(stderr, ", %f",n_frg[i].f); }
	  }
	  fprintf(stderr, "]\nfrg[%f",frg[0].f);
	  for(i=1;i<16;i++){ fprintf(stderr, ", %f",frg[i].f); }
	  fprintf(stderr, "]\n");
	} else if(!strcmp(buf2,"irg")) {
	  fprintf(stderr, "irg[%d",irg[0].i); for(i=1;i<16;i++){ fprintf(stderr, ", %d",irg[i].i); } fprintf(stderr, "]\n");
	} else if(!strcmp(buf2,"frg")) {
	  if(optflgs[3]){ // native FPU
	    fprintf(stderr, "n_frg[%f",n_frg[0].f); for(i=1;i<16;i++){ fprintf(stderr, ", %f",n_frg[i].f); } fprintf(stderr, "]\n");
	  }
	  fprintf(stderr, "frg[%f",frg[0].f); for(i=1;i<16;i++){ fprintf(stderr, ", %f",frg[i].f); } fprintf(stderr, "]\n");
	} else if(!strcmp(buf2,"op")) {
	  fprintf(stderr, "PC: %05d\n",irg[15].i); print_op(op);
	} else if(!strcmp(buf2,"breakpoint") || !strcmp(buf2,"bp")) {
	  show_array(breakpoints, 10);
	} else if(!strcmp(buf2,"exectime") || !strcmp(buf2,"et")) {
	  fprintf(stderr, "%d time.\n",execCounter);
	} else {
	  //fprintf(stderr, "print what?\n");
	}
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"list") || !strcmp(buf2,"l")){
	buf2 = strtok(NULL," \n");
	if(buf2==NULL){ tmp=5; }
	else { tmp = max(5,atoi(buf2)); }
	for(i=max(INIT_PC,irg[15].i-tmp*4);i<=min(INIT_HP,irg[15].i+tmp*4);i+=4){
	  fprintf(stderr, "%05d: ",i);
	  print_op(memory[i/4]);
	}
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"step") || !strcmp(buf2,"s")){
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
	if((tmp=atoi(buf2))){
	  add_array(breakpoints, tmp, 10);
	  fprintf(stderr, "set breakpoint: %d\n",tmp);
	} else { fprintf(stderr, "invalid.\n"); }
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"delete")){
	buf2 = strtok(NULL," \n");
	if((tmp=atoi(buf2))){
	  del_array(breakpoints, tmp);
	  fprintf(stderr, "delete breakpoint: %d\n",tmp);
	} else { fprintf(stderr, "invalid.\n"); }
	ifPrintOp = 0; ifDebug = 0;
	continue;
      } else if(!strcmp(buf2,"continue") || !strcmp(buf2,"c")){
	fprintf(stderr, "program continue.\n");	
      } else if(!strcmp(buf2,"exit")){
	break;
      } else {
	fprintf(stderr, "invalid command.\n");
	ifPrintOp = 0; ifDebug = 0;
	continue;
      }
    }
    if(ifDebug>=0){ ifDebug--; }
    // debug end ----

    // end with halt(beq r0 r0 r15 0)
    if(op == 0x8001e000){ break; }
    
    //-- countOp
    usedOpCounter[cutoutOp(op,0,6)]++;
    execCounter++;
    
    //---- decode & exec
    nextPC = irg[15].i + 4;

    switch (cutoutOp(op,0,6)) { //0-6:opcode
      //--- ALU
    case 0b0000000: //add
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].i = irg[rgs[1]].i + irg[rgs[2]].i + utoi(option,17);
      break;
    case 0b0000001: //addi
      cutoffOp(op,rgs,&option,2);
      if(((op>>16)&1) == 0)
	irg[rgs[0]].i = irg[rgs[1]].i + utoi(option,16);
      else
	irg[rgs[0]].i = irg[rgs[1]].i + (utoi(option,16)<<16);
      break;
    case 0b0000010: //sub
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].i = irg[rgs[1]].i - irg[rgs[2]].i;
      break;
    case 0b0000011: //subi
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]].i = irg[rgs[1]].i - utoi(option,17);
      break;
    case 0b0000100: //not
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]].u = ~irg[rgs[1]].u;
      break;
    case 0b0000110: //and
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].u = irg[rgs[1]].u & irg[rgs[2]].u;
      break;
    case 0b0001000: //or
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].u = irg[rgs[1]].u | irg[rgs[2]].u;
      break;
    case 0b0001010: //xor
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].u = irg[rgs[1]].u ^ irg[rgs[2]].u;
      break;
    case 0b0001100: //nand
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].u = ~(irg[rgs[1]].u & irg[rgs[2]].u);
      break;
    case 0b0001110: //nor
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].u = ~(irg[rgs[1]].u | irg[rgs[2]].u);
      break;
    case 0b0010000: //shift
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].u = shift_(irg[rgs[1]].u, cutoutOp(op,24,24), cutoutOp(op,25,26), irg[rgs[2]].u);
      break;
    case 0b0010001: //shifti
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].u = shift_(irg[rgs[1]].u, cutoutOp(op,24,24), cutoutOp(op,25,26), utoi(cutoutOp(op,19,23),6));
      break;
      //--- FPU
    case 0b0100000: //fadd
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]].u = fadd(frg[rgs[1]].u, frg[rgs[2]].u);
      n_frg[rgs[0]].f = n_frg[rgs[1]].f + n_frg[rgs[2]].f;
      break;
    case 0b0100010: //fsub
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]].u = fsub(frg[rgs[1]].u, frg[rgs[2]].u);
      n_frg[rgs[0]].f = n_frg[rgs[1]].f - n_frg[rgs[2]].f;
      break;
    case 0b0100100: //fmul
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]].u = fmul(frg[rgs[1]].u, frg[rgs[2]].u);
      n_frg[rgs[0]].f = n_frg[rgs[1]].f * n_frg[rgs[2]].f;
      break;
    case 0b0100110: //fdiv
      cutoffOp(op,rgs,&option,3);
      frg[rgs[0]].u = fdiv(frg[rgs[1]].u, frg[rgs[2]].u);
      n_frg[rgs[0]].f = n_frg[rgs[1]].f / n_frg[rgs[2]].f;
      break;
    case 0b0101000: //fsqrt
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]].u = fsqrt(frg[rgs[1]].u);
      n_frg[rgs[0]].f = sqrtf(n_frg[rgs[1]].f);
      break;
    case 0b0101010: //ftoi
      cutoffOp(op,rgs,&option,2);
      irg[rgs[0]].u = h_floor(frg[rgs[1]].u);
      irg[rgs[0]].f = floorf(n_frg[rgs[1]].f);
      break;
    case 0b0101100: //itof
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]].u = h_i2f(irg[rgs[1]].u);
      n_frg[rgs[0]].f = (float)irg[rgs[1]].i;
      break;
    case 0b0101110: //fneg  //not yet
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]].f = -frg[rgs[1]].f;
      n_frg[rgs[0]].f = -n_frg[rgs[1]].f;
      break;
    case 0b0110000: //finv
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]].u = finv(frg[rgs[1]].u);
      n_frg[rgs[0]].f = 1 / n_frg[rgs[1]].f;
      break;
      //--- branch
    case 0b1000000: //beq
      cutoffOp(op,rgs,&option,3);
      if(irg[rgs[0]].i == irg[rgs[1]].i){
	nextPC = irg[rgs[2]].i + utoi(option,13);
      }
      break;
    case 0b1000001: //beqi
      cutoffOp(op,rgs,&option,2);
      if(irg[rgs[0]].i == irg[rgs[1]].i){
	nextPC = utoi(option,17);
      }
      break;
    case 0b1000010: //blt
      cutoffOp(op,rgs,&option,3);
      if(irg[rgs[0]].i < irg[rgs[1]].i){
	nextPC = irg[rgs[2]].i + utoi(option,13);
      }
      break;
    case 0b1000011: //blti
      cutoffOp(op,rgs,&option,2);
      if(irg[rgs[0]].i < irg[rgs[1]].i){
	nextPC = utoi(option,17);
      }
      break;
    case 0b1000100: //bfeq
      cutoffOp(op,rgs,&option,3);
      if(!optflgs[3]){
	if(feq(frg[rgs[0]].u, frg[rgs[1]].u)){
	  nextPC = irg[rgs[2]].i + utoi(option,13);
	}
      } else {
	if(n_frg[rgs[0]].f == n_frg[rgs[1]].f){
	  nextPC = irg[rgs[2]].i + utoi(option,13);
	}
      }
      break;
    case 0b1000101: //bfeqi
      cutoffOp(op,rgs,&option,2);
      if(!optflgs[3]){
	if(feq(frg[rgs[0]].u, frg[rgs[1]].u)){
	  nextPC = utoi(option,17);
	}
      } else {	
	if(n_frg[rgs[0]].f == n_frg[rgs[1]].f){
	  nextPC = utoi(option,17);
	}
      }
      break;
    case 0b1000110: //bflt
      cutoffOp(op,rgs,&option,3);
      if(!optflgs[3]){
	if(flt(frg[rgs[0]].u, frg[rgs[1]].u)){
	  nextPC = irg[rgs[2]].i + utoi(option,13);
	}
      } else {
	if(n_frg[rgs[0]].f < n_frg[rgs[1]].f){
	  nextPC = irg[rgs[2]].i + utoi(option,13);
	}
      }
      break;
    case 0b1000111: //bflti
      cutoffOp(op,rgs,&option,2);
      if(!optflgs[3]){
	if(flt(frg[rgs[0]].u, frg[rgs[1]].u)){
	  nextPC = utoi(option,17);
	}
      } else {
	if(n_frg[rgs[0]].f < n_frg[rgs[1]].f){
	  nextPC = utoi(option,17);
	}
      }
      break;
      //---- system
    case 0b1100000: //load
      cutoffOp(op,rgs,&option,2);

      irg[rgs[0]].u = memory[(irg[rgs[1]].i + utoi(option,17))/4];
      if(rgs[0]==15){
	nextPC = irg[rgs[0]].i;
      }
      break;
    case 0b1100010: //store
      cutoffOp(op,rgs,&option,2);
      memory[(irg[rgs[1]].i + utoi(option,17))/4] = irg[rgs[0]].u;
      break;
    case 0b1100100: //fload
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]].u = memory[(irg[rgs[1]].i + utoi(option,17))/4];
      n_frg[rgs[0]].u = memory[(irg[rgs[1]].i + utoi(option,17))/4];
      break;
    case 0b1100110: //fstore
      cutoffOp(op,rgs,&option,2);
      memory[(irg[rgs[1]].i + utoi(option,17))/4] = frg[rgs[0]].u;
      if(optflgs[3]){
	memory[(irg[rgs[1]].i + utoi(option,17))/4] = n_frg[rgs[0]].u;
      }
      break;
    case 0b1101000: //loadr
      cutoffOp(op,rgs,&option,3);
      irg[rgs[0]].u = memory[(irg[rgs[1]].i + irg[rgs[2]].i)/4];
      if(rgs[0]==15){
	nextPC = irg[rgs[0]].i; //
      }
      break;
    case 0b1101010: //storer
      cutoffOp(op,rgs,&option,2);
      memory[(irg[rgs[1]].i + irg[rgs[2]].i)/4] = irg[rgs[0]].u;
      break;
    case 0b1101100: //floadr
      cutoffOp(op,rgs,&option,2);
      frg[rgs[0]].u   = memory[(irg[rgs[1]].i + irg[rgs[2]].i)/4];
      n_frg[rgs[0]].u = memory[(irg[rgs[1]].i + irg[rgs[2]].i)/4];      
      break;
    case 0b1101110: //fstorer
      cutoffOp(op,rgs,&option,2);
      memory[(irg[rgs[1]].i + irg[rgs[2]].i)/4] = frg[rgs[0]].u;
      if(optflgs[3]){
	memory[(irg[rgs[1]].i + irg[rgs[2]].i)/4] = n_frg[rgs[0]].u;
      }
      break;
    case 0b1110000: //read
      cutoffOp(op,rgs,&option,1);
      fread(&irg[rgs[0]].ch[0], sizeof(char), 1, input);
      for(i=1;i<4;i++){
	irg[rgs[0]].ch[i] = 0;
      }
      break;
    case 0b1110001: //write
      cutoffOp(op,rgs,&option,1);
      fwrite(&irg[rgs[0]].ch[0], sizeof(char), 1, output);
      break;
    default:
      fprintf(stderr, "invalid opration??\n");
      fprintf(stderr, "%05d :", irg[15].i);
      p_binary(op,32);
      break;
    } // --end switch

    //---- end
    irg[15].i = nextPC;
  }
 
  // print infos
  if(optflgs[0]){ // print rg
    fprintf(stderr, "irg[%d",irg[0].i);
    for(i=1;i<16;i++){ fprintf(stderr, ", %d",irg[i].i); }
    if(optflgs[3]){ // native FPU
      fprintf(stderr, "]\nn_frg[%f",n_frg[0].f);
      for(i=1;i<16;i++){ fprintf(stderr, ", %f",n_frg[i].f); }
    }
    fprintf(stderr, "]\nfrg[%f",frg[0].f);
    for(i=1;i<16;i++){ fprintf(stderr, ", %f",frg[i].f); }
    fprintf(stderr, "]\n");
  }
  if(optflgs[1]){ // total exec
    fprintf(stderr, "%d oprations\n",execCounter);
  }
  if(optflgs[2]){ // count op
    print_countOp(usedOpCounter);
  }
  if(optflgs[4]){ // exist debug input
    fclose(input);
  }
  //
  fclose(fp);
  return 0;
}
