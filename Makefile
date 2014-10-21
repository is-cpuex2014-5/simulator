
CC = gcc
CFLAGS = -Wall -O2
SRCS := ${wildcard *.c}
TARGET = ${SRCS:.c=}

all: ${TARGET}

${TARGET}: %: %.o
	${CC} ${CFLAGS} -o $@ $<

clean:
	rm -f ${TARGET} *.o

.PHONY: all clean