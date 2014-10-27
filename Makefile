CC = gcc
CFLAGS = -Wall -O2

TARGET = simulator
SRCS = moromoro
ARGS = ${TARGET:=.c} ${SRCS:=.c} 
OBJS = ${ARGS:.c=.o}
HEADS = ${SRCS:=.h} fpu_.h

FPUS = ../fpu/fadd ../fpu/fsub ../fpu/floor
FPU_S = ${FPUS:=.c}
FPU_O = ${FPUS:=.o}

all: ${TARGET}

${TARGET}: ${OBJS} ${FPU_O}
	${CC} ${CFLAGS} -o $@ $^

${TARGET}: ${HEADS}

clean:
	rm ${TARGET} *.o

.PHONY: all clean
