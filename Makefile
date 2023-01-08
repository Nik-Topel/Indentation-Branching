PREFIX := /usr

CC := gcc
STRIP := strip

CFLAGS := -Os -std=c11 -Wall -D_POSIX_C_SOURCE -fanalyzer
LDFLAGS := -static

STRIPFLAGS := --strip-all -R .comment -x

all:
	ib ib.c.ib
	$(CC) ib.c -o ib $(CFLAGS) $(LDFLAGS)
	${STRIP} $(STRIPFLAGS) ib -o ib

install:
	cp ib ${PREFIX}/bin/

test: all
	rm ib.c
	./ib ib.c.ib
	$(CC) ib.c -o ib $(CFLAGS) $(LDFLAGS)
	${STRIP} $(STRIPFLAGS) ib -o ib

sa:
	cppcheck cppcheck --enable=all --inconclusive --library=posix --force --suppress=missingIncludeSystem ib.c
	frama-c ib.c -c11

clean:
	rm ib.c ib

bootstrap:
	$(CC) ib.c -o ib $(CFLAGS) $(LDFLAGS)
	${STRIP} $(STRIPFLAGS) ib -o ib
	cp ib ${PREFIX}/bin/
