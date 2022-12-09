PREFIX := ${HOME}

CC := clang
STRIP := llvm-strip

CFLAGS := -std=c11 -Weverything -Wno-declaration-after-statement -glldb
LDFLAGS :=

#STRIPFLAGS := --strip-all -R .comment -R .eh_frame -R .data.rel.ro -R .got.plt -R .dtors -x
STRIPFLAGS :=

all:
	ib ib.c.ib
	$(CC) ib.c -o ib_prot $(CFLAGS) $(LDFLAGS)
	#${STRIP} $(STRIPFLAGS) ib_prot -o ib

install:
	cp ib ${PREFIX}/bin/

test: all
	rm ib.c
	./ib ib.c.ib
	$(CC) ib.c -o ib_prot $(CFLAGS) $(LDFLAGS)
	${STRIP} $(STRIPFLAGS) ib_prot -o ib
	rm ib.c
	./ib ib.c.ib
	$(CC) ib.c -o ib_prot $(CFLAGS) $(LDFLAGS)
	${STRIP} $(STRIPFLAGS) ib_prot -o ib

clean:
	rm ib.c ib_prot ib
