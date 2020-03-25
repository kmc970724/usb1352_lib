CC = gcc
CFLAG = -c

BINARY = bin

SRCS = $(foreach dir, ., $(wildcard *.c))
BINS = $(SRCS:.c=.o)

all: $(SRCS)
	$(CC) $(CFLAG) $(SRCS)
	mv $(BINS) $(BINARY)
