#include <stdio.h>
#include <stdint.h>

typedef union{
  uint32_t u;
  char ch[4];
} endian;
uint32_t change_endian(uint32_t u){
  // big endian <-> little endian
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

int main(int argc, char*argv[]){
  FILE*fp;
  if((fp=fopen("test","wb"))==NULL){
    printf("err@file open");
    return 1;
  } 
  int i;
  //  ------7---4---4---4--3--------10
  ///* fibloop
  uint32_t ops[100] = {
    0b00000010001000000000000000001010, //r1 = 10
    0b00000010010000000000000000000001, //r2 = 1
    0b00000010011000000000000000000001, //r3 = 1
    0b00000010101000000000000000000010, //r5 = 2
    0b10000100001010111110000000011000, //L1:r1<r5 -> end
    0b00000110001000100000000000000001, //r1 -= 1
    0b00000000100001000110000000000000, //r4 = r2+r3
    0b00000000011000000100000000000000, //r3 = r0+r2
    0b00000000010000001000000000000000, //r2 = r0+r4
    0b10000000000000011111111111101100, //         -> L
    0b00000000001001000000000000000000, //end: r1 = r2
    0b11110100000000000000000000000000,
    0b11111010000000000000000000000000};
  //*/
  int num=100;
  for(i=0;i<num;i++){ ops[i] = change_endian(ops[i]); }
  fwrite(ops,sizeof(uint32_t),num,fp);
  
  fclose(fp);
  return 0;
}
