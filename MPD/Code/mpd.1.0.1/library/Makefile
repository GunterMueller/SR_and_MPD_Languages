##	Makefile for the MPD X-Windows interface, MPDWin.

CFLAGS = -g
MPDFLAGS = -g
SHELL = /bin/sh


all:	MPDgetopt.o MPDWin.o MPDanimator.o  mpdwin.o mpdanimator.o


MPDgetopt.spec MPDgetopt.impl MPDgetopt.o: MPDgetopt.mpd
	../mpd/mpd -e $(MPDFLAGS) -c MPDgetopt.mpd
	mv MPDinter/MPDgetopt.spec MPDinter/MPDgetopt.impl \
	   MPDinter/MPDgetopt.o .


MPDWin.spec MPDWin.impl MPDWin.o: gmpdwin.mpd
	../mpd/mpd -e $(MPDFLAGS) -c gmpdwin.mpd
	mv MPDinter/MPDWin.spec MPDinter/MPDWin.impl MPDinter/MPDWin.o .

mpdwin.o:	mpdwin.h mpdwin.c mpdwin.icon
	-. ../paths.sh; if [ "$$XINCL" != "" ]; then \
	    $(CC) -I$$XINCL $(CFLAGS) -c mpdwin.c; fi


MPDanimator.spec MPDanimator.impl MPDanimator.o: MPDanimator.mpd
	../mpd/mpd -e $(MPDFLAGS) -c MPDanimator.mpd
	mv MPDinter/MPDanimator.spec MPDinter/MPDanimator.impl \
	   MPDinter/MPDanimator.o .

mpdanimator.o:	mpdanimator.c
	-. ../paths.sh; if [ "$$XTANGO" != "" ]; then \
	    $(CC) -I$$XTANGO $(CFLAGS) -c mpdanimator.c; fi




# miscellaneous utility targets

clean:
	rm -rf core MPDinter *.o *.spec *.impl

depend:	

lint:
	. ../paths.sh; lint -I$$XTANGO -I$$XINCL *.c >lint.raw
	sed -f ../lint.sed <lint.raw >lint.out

tags:
	/usr/local/gnuemacs/etc/etags *.c
TAGS:
	ctags *.c
