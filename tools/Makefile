
CC=gcc
CFLAGS=-g3 -O0
#`pkg-config --cflags glib-2.0 libxml-2.0`
LDFLAGS=

all: code_gen

%.o: %.c
	$(CC) -c $(CFLAGS) `pkg-config --cflags glib-2.0 libxml-2.0` $^

code_gen: code_gen.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) `pkg-config --libs glib-2.0 libxml-2.0` $^
