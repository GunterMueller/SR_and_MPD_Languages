#   Makefile for building xtango demos.
#
#   Set TLIB to reflect the location of xtango.o from the xtango distribution.
#
#   Don't forget to merge xtango's "xtango.res" file into your X Resource
#   Database before running these programs.  If not, you'll see the message:
#	Error: Widget easel has zero width and/or height

SR = sr
SRFLAGS = 
SRL = srl
SRLFLAGS =

TLIB = /home/sr/xtango/lib/xtango.o
XLIBS = -lXaw -lXmu -lXext -lXt -lX11
LIBS = sranimator.o $(TLIB) $(XLIBS)

ANIMS = bubblesort quicksort dining bbproc bbin rwstarve rwfair
TESTS = hello tfonts tlines tresize tsteps tzoom
ALL = $(ANIMS) $(TESTS)


default:	anims
anims:		$(ANIMS)
tests:		$(TESTS)
all:		$(ALL)


# animations

quicksort:	quicksort.sr
		$(SR) $(SRFLAGS) -c quicksort.sr
		$(SRL) $(SRLFLAGS) -L1 -o quicksort SRanimator quicksort $(LIBS)

bubblesort:	bubblesort.sr
		$(SR) $(SRFLAGS) -o bubblesort bubblesort.sr $(LIBS)

dining:		dining.sr
		$(SR) $(SRFLAGS) -o dining dining.sr $(LIBS)

bbproc:		bbproc.sr
		$(SR) $(SRFLAGS) -o bbproc bbproc.sr bbdriver.sr $(LIBS)

bbin:		bbin.sr
		$(SR) $(SRFLAGS) -o bbin bbin.sr bbdriver.sr $(LIBS)

rwstarve:	rwstarve.sr
		$(SR) $(SRFLAGS) -o rwstarve rwstarve.sr rwdriver.sr $(LIBS)

rwfair:		rwfair.sr
		$(SR) $(SRFLAGS) -o rwfair rwfair.sr rwdriver.sr $(LIBS)



# tests

hello:		hello.sr
		$(SR) $(SRFLAGS) -o hello hello.sr $(LIBS)

tfonts:		tfonts.sr
		$(SR) $(SRFLAGS) -o tfonts tfonts.sr $(LIBS)

tlines:		tlines.sr
		$(SR) $(SRFLAGS) -o tlines tlines.sr $(LIBS)

tresize:	tresize.sr
		$(SR) $(SRFLAGS) -o tresize tresize.sr $(LIBS)

tsteps:		tsteps.sr
		$(SR) $(SRFLAGS) -o tsteps tsteps.sr $(LIBS)

tzoom:		tzoom.sr
		$(SR) $(SRFLAGS) -o tzoom tzoom.sr $(LIBS)



clean:
		rm -rf $(ALL) Interfaces core *.o
