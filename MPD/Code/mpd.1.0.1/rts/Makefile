##	Makefile for the MPD runtime system, mpdlib.a, and VM supervisor, mpdx.


CFLAGS = -g
SHELL = /bin/sh
MAKE = make


CSW =	../csw/asm.o
COBJ =	main.o \
	alloc.o array.o co.o conv.o debug.o event.o invoke.o io.o iop.o \
	math.o misc.o net.o oper.o pool.o process.o \
	remote.o res.o scan.o semaphore.o socket.o trace.o vm.o
OBJ = 	$(COBJ) $(CSW)

XOBJ =	mpdx.o netpath.o socket.mpdx.o debug.mpdx.o

SABER =	$(COBJ:o=c) $(CSW)



all:	mpdlib mpdx
mpdlib:	mpdlib.a



mpdlib.a:$&  $(OBJ) Makefile
	rm -f mpdlib.a
	sh ckobj.sh $(COBJ)
	ar cr mpdlib.a $(OBJ)
	-if [ -f /bin/ranlib -o -f /usr/bin/ranlib ]; then ranlib mpdlib.a; fi

$(CSW):
	cd ../csw; $(MAKE)



mpdx:	$(XOBJ)
	. ../paths.sh; $(CC) $(CFLAGS) -o mpdx $(XOBJ) $$LIBC

# defining MPDX disables MultiMPD to build single threaded versions mpdx needs
# (dependencies on .o files force rebuild when .h files change)

mpdx.o:
	$(CC) $(CFLAGS) -DMPDX -c mpdx.c
netpath.o:
	$(CC) $(CFLAGS) -DMPDX -c netpath.c

socket.mpdx.o: socket.c socket.o
	cp socket.c socket.mpdx.c
	$(CC) $(CFLAGS) -DMPDX -c socket.mpdx.c
	rm -f socket.mpdx.c

debug.mpdx.o: debug.c debug.o
	cp debug.c debug.mpdx.c
	$(CC) $(CFLAGS) -DMPDX -c debug.mpdx.c
	rm -f debug.mpdx.c



# miscellaneous utility targets

clean:
	rm -f *.o *.mpdx.c core

depend:	
	sh ../depend.sh


lint:
	lint `ls *.c | egrep -v 'mpdx|netpath'`			>lint.raw
	echo =====						>>lint.raw
	lint -DMPDX mpdx.c netpath.c socket.c debug.c 	>>lint.raw
	sed -f ../lint.sed <lint.raw >lint.out


tar:
	@tar cBf - [A-Z]* *.[ch] *.sed


tags:
	/usr/local/gnuemacs/etc/etags *.c
TAGS:
	ctags *.c




################################################################################
# The rest of this file is generated by `make depend'.  Don't change it by hand.
################################################################################

# >>DO NOT DELETE THIS LINE<< #
#   "make depend" was last run:  Tue Jan 2 14:50:49 MST 2001
alloc.o: alloc.c
alloc.o: rts.h
alloc.o: ../mpdmulti.h
alloc.o: ../gen.h
alloc.o: multimac.h
alloc.o: ../mpd.h
alloc.o: ../config.h
alloc.o: debug.h
alloc.o: procsem.h
alloc.o: netw.h
alloc.o: oper.h
alloc.o: res.h
alloc.o: funcs.h
alloc.o: macros.h
alloc.o: globals.h
alloc.o: ../runerr.h
array.o: array.c
array.o: rts.h
array.o: ../mpdmulti.h
array.o: ../gen.h
array.o: multimac.h
array.o: ../mpd.h
array.o: ../config.h
array.o: debug.h
array.o: procsem.h
array.o: netw.h
array.o: oper.h
array.o: res.h
array.o: funcs.h
array.o: macros.h
array.o: globals.h
array.o: ../runerr.h
co.o: co.c
co.o: rts.h
co.o: ../mpdmulti.h
co.o: ../gen.h
co.o: multimac.h
co.o: ../mpd.h
co.o: ../config.h
co.o: debug.h
co.o: procsem.h
co.o: netw.h
co.o: oper.h
co.o: res.h
co.o: funcs.h
co.o: macros.h
co.o: globals.h
co.o: ../runerr.h
conv.o: conv.c
conv.o: rts.h
conv.o: ../mpdmulti.h
conv.o: ../gen.h
conv.o: multimac.h
conv.o: ../mpd.h
conv.o: ../config.h
conv.o: debug.h
conv.o: procsem.h
conv.o: netw.h
conv.o: oper.h
conv.o: res.h
conv.o: funcs.h
conv.o: macros.h
conv.o: globals.h
conv.o: ../runerr.h
debug.o: debug.c
debug.o: rts.h
debug.o: ../mpdmulti.h
debug.o: ../gen.h
debug.o: multimac.h
debug.o: ../mpd.h
debug.o: ../config.h
debug.o: debug.h
debug.o: procsem.h
debug.o: netw.h
debug.o: oper.h
debug.o: res.h
debug.o: funcs.h
debug.o: macros.h
debug.o: globals.h
debug.o: ../runerr.h
event.o: event.c
event.o: rts.h
event.o: ../mpdmulti.h
event.o: ../gen.h
event.o: multimac.h
event.o: ../mpd.h
event.o: ../config.h
event.o: debug.h
event.o: procsem.h
event.o: netw.h
event.o: oper.h
event.o: res.h
event.o: funcs.h
event.o: macros.h
event.o: globals.h
event.o: ../runerr.h
invoke.o: invoke.c
invoke.o: rts.h
invoke.o: ../mpdmulti.h
invoke.o: ../gen.h
invoke.o: multimac.h
invoke.o: ../mpd.h
invoke.o: ../config.h
invoke.o: debug.h
invoke.o: procsem.h
invoke.o: netw.h
invoke.o: oper.h
invoke.o: res.h
invoke.o: funcs.h
invoke.o: macros.h
invoke.o: globals.h
invoke.o: ../runerr.h
io.o: io.c
io.o: rts.h
io.o: ../mpdmulti.h
io.o: ../gen.h
io.o: multimac.h
io.o: ../mpd.h
io.o: ../config.h
io.o: debug.h
io.o: procsem.h
io.o: netw.h
io.o: oper.h
io.o: res.h
io.o: funcs.h
io.o: macros.h
io.o: globals.h
io.o: ../runerr.h
iop.o: iop.c
iop.o: rts.h
iop.o: ../mpdmulti.h
iop.o: ../gen.h
iop.o: multimac.h
iop.o: ../mpd.h
iop.o: ../config.h
iop.o: debug.h
iop.o: procsem.h
iop.o: netw.h
iop.o: oper.h
iop.o: res.h
iop.o: funcs.h
iop.o: macros.h
iop.o: globals.h
iop.o: ../runerr.h
main.o: main.c
main.o: rts.h
main.o: ../mpdmulti.h
main.o: ../gen.h
main.o: multimac.h
main.o: ../mpd.h
main.o: ../config.h
main.o: debug.h
main.o: procsem.h
main.o: netw.h
main.o: oper.h
main.o: res.h
main.o: funcs.h
main.o: macros.h
main.o: globals.h
main.o: ../runerr.h
main.o: ../runerr.h
math.o: math.c
math.o: rts.h
math.o: ../mpdmulti.h
math.o: ../gen.h
math.o: multimac.h
math.o: ../mpd.h
math.o: ../config.h
math.o: debug.h
math.o: procsem.h
math.o: netw.h
math.o: oper.h
math.o: res.h
math.o: funcs.h
math.o: macros.h
math.o: globals.h
math.o: ../runerr.h
misc.o: misc.c
misc.o: rts.h
misc.o: ../mpdmulti.h
misc.o: ../gen.h
misc.o: multimac.h
misc.o: ../mpd.h
misc.o: ../config.h
misc.o: debug.h
misc.o: procsem.h
misc.o: netw.h
misc.o: oper.h
misc.o: res.h
misc.o: funcs.h
misc.o: macros.h
misc.o: globals.h
misc.o: ../runerr.h
mpdx.o: mpdx.c
mpdx.o: ../paths.h
mpdx.o: rts.h
mpdx.o: ../mpdmulti.h
mpdx.o: ../gen.h
mpdx.o: multimac.h
mpdx.o: ../mpd.h
mpdx.o: ../config.h
mpdx.o: debug.h
mpdx.o: procsem.h
mpdx.o: netw.h
mpdx.o: oper.h
mpdx.o: res.h
mpdx.o: funcs.h
mpdx.o: macros.h
mpdx.o: globals.h
mpdx.o: ../runerr.h
net.o: net.c
net.o: rts.h
net.o: ../mpdmulti.h
net.o: ../gen.h
net.o: multimac.h
net.o: ../mpd.h
net.o: ../config.h
net.o: debug.h
net.o: procsem.h
net.o: netw.h
net.o: oper.h
net.o: res.h
net.o: funcs.h
net.o: macros.h
net.o: globals.h
net.o: ../runerr.h
netpath.o: netpath.c
netpath.o: ../config.h
netpath.o: ../gen.h
netpath.o: ../mpdmulti.h
netpath.o: multimac.h
netpath.o: debug.h
oper.o: oper.c
oper.o: rts.h
oper.o: ../mpdmulti.h
oper.o: ../gen.h
oper.o: multimac.h
oper.o: ../mpd.h
oper.o: ../config.h
oper.o: debug.h
oper.o: procsem.h
oper.o: netw.h
oper.o: oper.h
oper.o: res.h
oper.o: funcs.h
oper.o: macros.h
oper.o: globals.h
oper.o: ../runerr.h
pool.o: pool.c
pool.o: rts.h
pool.o: ../mpdmulti.h
pool.o: ../gen.h
pool.o: multimac.h
pool.o: ../mpd.h
pool.o: ../config.h
pool.o: debug.h
pool.o: procsem.h
pool.o: netw.h
pool.o: oper.h
pool.o: res.h
pool.o: funcs.h
pool.o: macros.h
pool.o: globals.h
pool.o: ../runerr.h
process.o: process.c
process.o: rts.h
process.o: ../mpdmulti.h
process.o: ../gen.h
process.o: multimac.h
process.o: ../mpd.h
process.o: ../config.h
process.o: debug.h
process.o: procsem.h
process.o: netw.h
process.o: oper.h
process.o: res.h
process.o: funcs.h
process.o: macros.h
process.o: globals.h
process.o: ../runerr.h
process.o: ../mpdmulti.c
remote.o: remote.c
remote.o: rts.h
remote.o: ../mpdmulti.h
remote.o: ../gen.h
remote.o: multimac.h
remote.o: ../mpd.h
remote.o: ../config.h
remote.o: debug.h
remote.o: procsem.h
remote.o: netw.h
remote.o: oper.h
remote.o: res.h
remote.o: funcs.h
remote.o: macros.h
remote.o: globals.h
remote.o: ../runerr.h
res.o: res.c
res.o: rts.h
res.o: ../mpdmulti.h
res.o: ../gen.h
res.o: multimac.h
res.o: ../mpd.h
res.o: ../config.h
res.o: debug.h
res.o: procsem.h
res.o: netw.h
res.o: oper.h
res.o: res.h
res.o: funcs.h
res.o: macros.h
res.o: globals.h
res.o: ../runerr.h
scan.o: scan.c
scan.o: rts.h
scan.o: ../mpdmulti.h
scan.o: ../gen.h
scan.o: multimac.h
scan.o: ../mpd.h
scan.o: ../config.h
scan.o: debug.h
scan.o: procsem.h
scan.o: netw.h
scan.o: oper.h
scan.o: res.h
scan.o: funcs.h
scan.o: macros.h
scan.o: globals.h
scan.o: ../runerr.h
semaphore.o: semaphore.c
semaphore.o: rts.h
semaphore.o: ../mpdmulti.h
semaphore.o: ../gen.h
semaphore.o: multimac.h
semaphore.o: ../mpd.h
semaphore.o: ../config.h
semaphore.o: debug.h
semaphore.o: procsem.h
semaphore.o: netw.h
semaphore.o: oper.h
semaphore.o: res.h
semaphore.o: funcs.h
semaphore.o: macros.h
semaphore.o: globals.h
semaphore.o: ../runerr.h
socket.o: socket.c
socket.o: rts.h
socket.o: ../mpdmulti.h
socket.o: ../gen.h
socket.o: multimac.h
socket.o: ../mpd.h
socket.o: ../config.h
socket.o: debug.h
socket.o: procsem.h
socket.o: netw.h
socket.o: oper.h
socket.o: res.h
socket.o: funcs.h
socket.o: macros.h
socket.o: globals.h
socket.o: ../runerr.h
trace.o: trace.c
trace.o: rts.h
trace.o: ../mpdmulti.h
trace.o: ../gen.h
trace.o: multimac.h
trace.o: ../mpd.h
trace.o: ../config.h
trace.o: debug.h
trace.o: procsem.h
trace.o: netw.h
trace.o: oper.h
trace.o: res.h
trace.o: funcs.h
trace.o: macros.h
trace.o: globals.h
trace.o: ../runerr.h
vm.o: vm.c
vm.o: rts.h
vm.o: ../mpdmulti.h
vm.o: ../gen.h
vm.o: multimac.h
vm.o: ../mpd.h
vm.o: ../config.h
vm.o: debug.h
vm.o: procsem.h
vm.o: netw.h
vm.o: oper.h
vm.o: res.h
vm.o: funcs.h
vm.o: macros.h
vm.o: globals.h
vm.o: ../runerr.h
