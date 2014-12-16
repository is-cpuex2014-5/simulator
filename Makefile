CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -O2 -I../fpu/ -L../fpu/
CLIBS = -lm -lfpu

TARGET = simulator
SRCS = moromoro
ARGS = ${TARGET:=.c} ${SRCS:=.c} 
OBJS = ${ARGS:.c=.o}  libfpu.a
HEADS = ${SRCS:=.h} fpu.h

FPU_PATH = ../fpu
VPATH = ${FPU_PATH}
FPUS = fadd fsub fmul fdiv i2f floor float
FPU_O = ${FPUS:=.o}
FPU_H = ${FPUS:=.h}

all: fpu ${TARGET}

fpu:
	make -C ${FPU_PATH}

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${CLIBS}


clean:
	rm ${TARGET} *.o
#make -C ${FPU_PATH} full_clean

.PHONY: all clean
