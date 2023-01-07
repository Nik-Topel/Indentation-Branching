PREFIX := /usr

CC := gcc
STRIP := strip

CFLAGS := -Os -std=c11 -Wall -D_POSIX_C_SOURCE -fanalyzer
LDFLAGS :=

STRIPFLAGS := --strip-all -R .comment -x

all:
	ib ib.c.ib
	$(CC) ib.c -o ib_prot $(CFLAGS) $(LDFLAGS)
	${STRIP} $(STRIPFLAGS) ib_prot -o ib

install:
	cp ib ${PREFIX}/bin/

test: all
	rm ib.c
	./ib ib.c.ib
	$(CC) ib.c -o ib_prot $(CFLAGS) $(LDFLAGS)
	${STRIP} $(STRIPFLAGS) ib_prot -o ib

sa:
	cppcheck cppcheck --enable=all --inconclusive --library=posix --force --suppress=missingIncludeSystem ib.c
	frama-c ib.c -c11

clean:
	rm ib.c ib_prot ib

bootstrap:
	$(CC) ib.c -o ib_prot $(CFLAGS) $(LDFLAGS)
	${STRIP} $(STRIPFLAGS) ib_prot -o ib
	cp ib ${PREFIX}/bin/
