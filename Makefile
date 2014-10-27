CC = gcc
CFLAGS = -Wall -O2

TARGET = simulator
ARGS = moromoro

OBJS = ${ARGS:=.o} ${TARGET:=.o} 
HEADS = ${ARGS:=.h}

all: ${TARGET}

${TARGET}: ${OBJS} 
	${CC} ${CFLAGS} -o $@ $^
${TARGET}: ${HEADS}

clean:
	rm ${TARGET} *.o

.PHONY: all clean
