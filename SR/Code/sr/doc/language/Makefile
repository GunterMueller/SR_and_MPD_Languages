##  Makefile for the SR Language Description
#
#   This directory contains the source for
#	SR: A Language for Parallel and Distributed Programming
#   which introduces version 2 of SR.
#
#   You'll probably need to fiddle with this a bit to get it to run
#   anywhere other than the University of Arizona.

SHELL = /bin/sh

D = ../../ps
ROFF = psroff -t

ALL = sec0 sec1 sec2 sec3 sec4 sec5 sec9

$D/language.ps:	Makefile font.sed $(ALL) model.ps cover
	sed -f font.sed $(ALL) | tbl | eqn | psfig | \
	    $(ROFF) -me cover - >$D/language.ps
