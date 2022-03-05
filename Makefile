CC=cc
CFLAGS=-g -std=c99 -Wall -pedantic -lX11 -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -O0 -I. -I/usr/include -I/usr/include/freetype2

all: test

%: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm  test
