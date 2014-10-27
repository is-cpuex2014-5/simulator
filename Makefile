CC = gcc
CFLAGS = -Wall -O2

PROGRAM = simulator
ARGS = simulator.o

.SUFFIXES = .c .o

all: ${PROGRAM}

${PROGRAM}: ${ARGS}
	${CC} ${CFLAGS} -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm ${PROGRAM} *.o

.PHONY: all clean
