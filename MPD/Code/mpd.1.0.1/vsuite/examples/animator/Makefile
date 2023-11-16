#   Makefile for building xtango demos.
#
#   Set TLIB to reflect the location of xtango.o from the xtango distribution.
#
#   Don't forget to merge xtango's "xtango.res" file into your X Resource
#   Database before running these programs.  If not, you'll see the message:
#	Error: Widget easel has zero width and/or height

MPD = mpd
MPDFLAGS = 
MPDL = mpdl
MPDLFLAGS =

TLIB = /home/sr/xtango/lib/xtango.o
XLIBS = -lXaw -lXmu -lXext -lXt -lX11
LIBS = mpdanimator.o $(TLIB) $(XLIBS)

ANIMS = bubblesort quicksort dining bbproc bbin rwstarve rwfair
TESTS = hello tfonts tlines tresize tsteps tzoom
ALL = $(ANIMS) $(TESTS)


default:	anims
anims:		$(ANIMS)
tests:		$(TESTS)
all:		$(ALL)


# animations

quicksort:	quicksort.mpd
		$(MPD) $(MPDFLAGS) -c quicksort.mpd
		$(MPDL) $(MPDLFLAGS) -L1 -o quicksort \
			MPDanimator quicksort $(LIBS)

bubblesort:	bubblesort.mpd
		$(MPD) $(MPDFLAGS) -o bubblesort bubblesort.mpd $(LIBS)

dining:		dining.mpd
		$(MPD) $(MPDFLAGS) -o dining dining.mpd $(LIBS)

bbproc:		bbproc.mpd
		$(MPD) $(MPDFLAGS) -o bbproc bbproc.mpd bbdriver.mpd $(LIBS)

bbin:		bbin.mpd
		$(MPD) $(MPDFLAGS) -o bbin bbin.mpd bbdriver.mpd $(LIBS)

rwstarve:	rwstarve.mpd
		$(MPD) $(MPDFLAGS) -o rwstarve rwstarve.mpd rwdriver.mpd $(LIBS)

rwfair:		rwfair.mpd
		$(MPD) $(MPDFLAGS) -o rwfair rwfair.mpd rwdriver.mpd $(LIBS)



# tests

hello:		hello.mpd
		$(MPD) $(MPDFLAGS) -o hello hello.mpd $(LIBS)

tfonts:		tfonts.mpd
		$(MPD) $(MPDFLAGS) -o tfonts tfonts.mpd $(LIBS)

tlines:		tlines.mpd
		$(MPD) $(MPDFLAGS) -o tlines tlines.mpd $(LIBS)

tresize:	tresize.mpd
		$(MPD) $(MPDFLAGS) -o tresize tresize.mpd $(LIBS)

tsteps:		tsteps.mpd
		$(MPD) $(MPDFLAGS) -o tsteps tsteps.mpd $(LIBS)

tzoom:		tzoom.mpd
		$(MPD) $(MPDFLAGS) -o tzoom tzoom.mpd $(LIBS)



clean:
		rm -rf $(ALL) MPDinter core *.o
