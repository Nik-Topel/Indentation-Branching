PREFIX := ${HOME}/

CC := gcc
STRIP := strip

CFLAGS := -std=c11 -Wall 
LDFLAGS := 

STRIPFLAGS := --strip-all -R .comment -x

all:
	ib ib.c.ib -i --flags "$(CFLAGS) $(LDFLAGS)" --compiler gcc
	#${STRIP} $(STRIPFLAGS) ib -o ib

install:
	cp ib ${PREFIX}/bin/

test: all
	./ib ib.c.ib -i --flags "$(CFLAGS) $(LDFLAGS)"
	#${STRIP} $(STRIPFLAGS) ib -o ib

sa:
	cppcheck cppcheck --enable=all --inconclusive --library=posix --force --suppress=missingIncludeSystem ib.c
	frama-c ib.c -c11

clean:
	rm ib

bootstrap:
	$(CC) ib.c -o ib $(CFLAGS) $(LDFLAGS)
	#${STRIP} $(STRIPFLAGS) ib -o ib
	cp ib ${PREFIX}/bin/
