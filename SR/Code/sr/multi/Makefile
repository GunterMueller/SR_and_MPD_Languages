##  Makefile for testing MultiSR primitives.
#
#   This works only on the architectures that support MultiSR.


SHELL = /bin/sh
MAKE = make

CFLAGS = -g


default:
	case `uname -a` in \
	    *DYNIX*)  $(MAKE) dynix;; \
	    *IRIX*)   $(MAKE) irix;; \
	    *SunOS*)  $(MAKE) solaris;; \
	    *)        echo "make what?"; exit 1 ;; \
	esac; exit


irix:
	$(MAKE) NCPU=`hinv -c processor | awk '{print $$1; exit}'` vfy

dynix:
	$(MAKE) CFLAGS=-Y NCPU=`/etc/online -N` vfy

solaris:
	$(MAKE) NCPU=`/usr/sbin/psrinfo | awk 'END {print NR}'` vfy


# run the test and check the output

vfy:	mtest mexpect
	rm -f vfy.out
	./mexpect $(NCPU) | sort >mtest.std
	./mtest   $(NCPU) >mtest.out
	sort mtest.out >mtest.srt
	cmp mtest.std mtest.srt
	echo "`date`: mtest $(NCPU) passed " >vfy.out


# mtest is the test program

mtest:	mtest.c mtest.h ../srmulti.h ../srmulti.c
	. ../paths.sh; $(CC) $(CFLAGS) -o mtest mtest.c $$LIBR


# mexpect tells us what to check for,
# because that varies with the processor count

mexpect: mexpect.c ../srmulti.h mtest.h
	$(CC) $(CFLAGS) -o mexpect mexpect.c


# miscellaneous utility targets

clean:
	rm -f *.o *.out mtest mexpect mtest.std mtest.srt core
