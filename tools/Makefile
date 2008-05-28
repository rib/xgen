
CC=gcc
CFLAGS=-g3 -O0 -I. -Wall
#`pkg-config --cflags glib-2.0 libxml-2.0`
LDFLAGS=

all: code_gen

%.o: %.c
	$(CC) -c $(CFLAGS) `pkg-config --cflags glib-2.0 libxml-2.0` $^

code_gen: code_gen.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) `pkg-config --libs glib-2.0 libxml-2.0` $^

.PHONY: test
test:
	./code_gen
	gcc -o test test.c `pkg-config --cflags --libs glib-2.0`

.PHONY:
gx: code_gen
	./code_gen
	./post_process.sh
	$(CC) $(CFLAGS) -c gx-connection.c gx-screen.c gx-window.c gx-drawable.c gx-gcontext.c `pkg-config --cflags glib-2.0 gobject-2.0 xcb`
	$(CC) $(CFLAGS) -o gx-connection-test gx-connection-test.c gx-connection.o gx-screen.o gx-window.o gx-drawable.o gx-gcontext.o `pkg-config --cflags --libs glib-2.0 gobject-2.0 xcb`

clean:
	rm -f *.o
	rm -f code_gen
	rm -f gx-connection-test
