#  Makefile for building MPDWin demos
#
#  May need adjustment depending on where the X library lives.

MPD = mpd
MPDFLAGS = 
LIBS = mpdwin.o -lX11

ALL = button demo gauss hello mandel qsort winquad


default: demo
all:	$(ALL)

button:	button.mpd	; $(MPD) $(MPDFLAGS) -o button  button.mpd  $(LIBS)
demo:	demo.mpd	; $(MPD) $(MPDFLAGS) -o demo    demo.mpd    $(LIBS)
gauss:	gauss.mpd	; $(MPD) $(MPDFLAGS) -o gauss   gauss.mpd   $(LIBS)
hello:	hello.mpd	; $(MPD) $(MPDFLAGS) -o hello   hello.mpd   $(LIBS)
mandel:	mandel.mpd	; $(MPD) $(MPDFLAGS) -o mandel  mandel.mpd  $(LIBS)
qsort:	qsort.mpd	; $(MPD) $(MPDFLAGS) -o qsort   qsort.mpd   $(LIBS)
winquad: winquad.mpd	; $(MPD) $(MPDFLAGS) -o winquad winquad.mpd $(LIBS)

clean:			; rm -rf $(ALL) MPDinter core *.o
