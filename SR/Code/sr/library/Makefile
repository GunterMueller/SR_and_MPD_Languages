##	Makefile for the SR X-Windows interface, SRWin.

CFLAGS = -g
SRFLAGS = -g
SHELL = /bin/sh


all:	SRgetopt.o SRWin.o SRanimator.o  srwin.o sranimator.o


SRgetopt.spec SRgetopt.impl SRgetopt.o: SRgetopt.sr
	../sr/sr -e $(SRFLAGS) -c SRgetopt.sr
	mv Interfaces/SRgetopt.spec Interfaces/SRgetopt.impl \
	   Interfaces/SRgetopt.o .


SRWin.spec SRWin.impl SRWin.o: gsrwin.sr
	../sr/sr -e $(SRFLAGS) -c gsrwin.sr
	mv Interfaces/SRWin.spec Interfaces/SRWin.impl Interfaces/SRWin.o .

srwin.o:	srwin.h srwin.c srwin.icon
	-. ../paths.sh; if [ "$$XINCL" != "" ]; then \
	    $(CC) -I$$XINCL $(CFLAGS) -c srwin.c; fi


SRanimator.spec SRanimator.impl SRanimator.o: SRanimator.sr
	../sr/sr -e $(SRFLAGS) -c SRanimator.sr
	mv Interfaces/SRanimator.spec Interfaces/SRanimator.impl \
	   Interfaces/SRanimator.o .

sranimator.o:	sranimator.c
	-. ../paths.sh; if [ "$$XTANGO" != "" ]; then \
	    $(CC) -I$$XTANGO $(CFLAGS) -c sranimator.c; fi




# miscellaneous utility targets

clean:
	rm -rf core Interfaces *.o *.spec *.impl

depend:	

lint:
	. ../paths.sh; lint -I$$XTANGO -I$$XINCL *.c >lint.raw
	sed -f ../lint.sed <lint.raw >lint.out

tags:
	/usr/local/gnuemacs/etc/etags *.c
TAGS:
	ctags *.c
