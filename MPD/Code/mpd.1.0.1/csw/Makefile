##	Makefile for the MPD context switch routines.

CFLAGS = -g
SHELL = /bin/sh

SRC =	alpha.s ap3000.s bobcat.s encore.s i386.s m68k.s m88k.s mips.s \
	paragon.s precision.s rs6000.s sgi.s sparc.s tahoe.s v68.s vax.s


# run the context switch test and check the output

OKAY:	cstest
	rm -f OKAY
	@echo ...checking context switch code
	./cstest >cstest.out
	cmp cstest.out cstest.stdout
	echo "`date`: cstest passed" >OKAY


# cstest is the test program

cstest:	cstest.o asm.o
	. ../paths.sh; $(CC) $(CFLAGS) -o cstest \
		cstest.o asm.o $$LIBM $$LIBC


# csloop is a timing program, built only on request

csloop:	csloop.o asm.o
	. ../paths.sh; $(CC) -O -o csloop csloop.o asm.o $$LIBR $$LIBC

csloop.o: csloop.c
	$(CC) -O -c csloop.c


# asm.o holds the actual context switch code

asm.o:  asm.c ../arch.h $(SRC)
	$(CC) $(CFLAGS) -E asm.c | \
	    sed -e '/^ *$$/d' -e '/^#/'d -e 's/ :/:/' >asm.s
	as -o asm.o asm.s



# miscellaneous utility targets

clean:
	rm -f *.o OKAY asm.[os] cstest cstest.out csloop core

depend:	

lint:
	lint cstest.c						 >lint.raw
	echo =====						>>lint.raw
	lint csloop.c						>>lint.raw
	sed -f ../lint.sed <lint.raw >lint.out
