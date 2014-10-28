CC = gcc
CFLAGS = -Wall -O2

TARGET = simulator
SRCS = moromoro
ARGS = ${TARGET:=.c} ${SRCS:=.c} 
OBJS = ${ARGS:.c=.o}
HEADS = ${SRCS:=.h}

VPATH = ../fpu
FPUS = fadd fsub floor float fmul i2f
FPU_O = ${FPUS:=.o}

all: ${TARGET}

${TARGET}: ${OBJS} ${FPU_O}
	${CC} ${CFLAGS} -o $@ $^
${TARGET}: ${HEADS}

${FPUS}: %: %.o %.h
	 ${CC} ${CFLAGS} -o $@ $<

clean:
	rm ${TARGET} *.o

.PHONY: all clean fpus
