PREFIX := /usr

CC := gcc
STRIP := strip

CFLAGS := -Os -std=c11 -Wno-declaration-after-statement -g
LDFLAGS :=

#STRIPFLAGS := --strip-all -R .comment -R .eh_frame -R .data.rel.ro -R .got.plt -R .dtors -x
STRIPFLAGS := 


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

clean:
	rm ib.c ib_prot ib

bootstrap:
	$(CC) ib.c -o ib_prot $(CFLAGS) $(LDFLAGS)
	${STRIP} $(STRIPFLAGS) ib_prot -o ib
	cp ib ${PREFIX}/bin/
