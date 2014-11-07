CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -O2 -I../fpu/

TARGET = simulator
SRCS = moromoro
ARGS = ${TARGET:=.c} ${SRCS:=.c} 
OBJS = ${ARGS:.c=.o}
HEADS = ${SRCS:=.h} fpus.h

VPATH = ../fpu
FPUS = fadd fsub fmul fdiv i2f floor float
FPU_O = ${FPUS:=.o}
FPU_H = ${FPUS:=.h}

all: ${TARGET}

${TARGET}: ${OBJS} ${FPU_O}
	${CC} ${CFLAGS} -o $@ $^
${TARGET}: ${HEADS} ${FPUS_H}


clean:
	rm ${TARGET} *.o

.PHONY: all clean
