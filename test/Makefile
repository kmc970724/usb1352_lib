CC = gcc
CFLAG = -o

SRCDIR = ../src
BINDIR = $(SRCDIR)bin
BINS = $(foreach dir, $(SRCDIR)/bin, $(wildcard $(SRCDIR)/bin/*.o))
LIBS = -lpthread -lft4222 -ldl
OBJECT = usb1352

all:
	cd $(SRCDIR) && make
	$(CC) $(CFLAG) $(OBJECT) usb1352.c $(BINS) $(LIBS)
