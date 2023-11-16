##	Makefile for the SR man pages.
#
#	Makes a .ps file of all the man pages.



SHELL = /bin/sh
ROFF = psroff -t

D = ../ps

MANS = sr.1 srl.1 srm.1 srprof.1 srgrind.1 srtex.1 srlatex.1 \
	ccr2sr.1 csp2sr.1 m2sr.1 srv.1 srvi.1 \
	sranimator.3 srgetopt.3 srwin.3 srmap.5 srtrace.5



$D/manpages.ps:	$(MANS)
	-if [ ! -d $D ]; then mkdir $D; fi
	$(ROFF) -man $(MANS) >$D/manpages.ps
