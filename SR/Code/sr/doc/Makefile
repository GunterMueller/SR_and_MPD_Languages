##	Makefile for the SR documentation.
#
#	By default, makes .ps versions of all important documents in ../ps.



MAKE = make
SHELL = /bin/sh
ROFF = psroff -t

D = ../ps



all:$&	$D/release.ps $D/install.ps $D/port.ps $D/language.ps $D/runtime.ps \
	$D/rel_2_0.ps $D/rel_2_1.ps $D/rel_2_2.ps

$D:
	-if [ ! -d $D ]; then mkdir $D; fi


$D/release.ps:	$D release.ms common.ms
	sed -f fmt.sed release.ms | $(ROFF) -ms - >$D/release.ps

$D/install.ps:	$D install.ms common.ms
	$(ROFF) -ms install.ms >$D/install.ps

$D/port.ps:	$D port.ms common.ms
	$(ROFF) -ms port.ms >$D/port.ps

$D/language.ps:	$D FORCE
	cd language; $(MAKE) $(MFLAGS) "ROFF=$(ROFF)"

$D/runtime.ps:	$D runtime.ms common.ms
	$(ROFF) -ms runtime.ms >$D/runtime.ps

$D/rel_2_0.ps:	$D rel_2_0.ms common.ms
	sed -f fmt.sed rel_2_0.ms | $(ROFF) -ms - >$D/rel_2_0.ps

$D/rel_2_1.ps:	$D rel_2_1.ms common.ms
	sed -f fmt.sed rel_2_1.ms | $(ROFF) -ms - >$D/rel_2_1.ps

$D/rel_2_2.ps:	$D rel_2_2.ms common.ms
	sed -f fmt.sed rel_2_2.ms | $(ROFF) -ms - >$D/rel_2_2.ps

FORCE:


clean:
	rm -f *.dvi *.out *.log *.aux
