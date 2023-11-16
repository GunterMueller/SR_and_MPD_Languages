#  Makefile for building SRWin demos
#
#  May need adjustment depending on where the X library lives.

SR = sr
SRFLAGS = 
LIBS = srwin.o -lX11

ALL = button demo gauss hello mandel qsort winquad


default: demo
all:	$(ALL)

button:	button.sr	; $(SR) $(SRFLAGS) -o button  button.sr  $(LIBS)
demo:	demo.sr		; $(SR) $(SRFLAGS) -o demo    demo.sr    $(LIBS)
gauss:	gauss.sr	; $(SR) $(SRFLAGS) -o gauss   gauss.sr   $(LIBS)
hello:	hello.sr	; $(SR) $(SRFLAGS) -o hello   hello.sr   $(LIBS)
mandel:	mandel.sr	; $(SR) $(SRFLAGS) -o mandel  mandel.sr  $(LIBS)
qsort:	qsort.sr	; $(SR) $(SRFLAGS) -o qsort   qsort.sr   $(LIBS)
winquad: winquad.sr	; $(SR) $(SRFLAGS) -o winquad winquad.sr $(LIBS)

clean:			; rm -rf $(ALL) Interfaces core *.o
