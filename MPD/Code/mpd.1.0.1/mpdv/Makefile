##	Makefile for the MPD validation scripts: mpdv, mpdvi, mpdvsumm



SHELL = /bin/sh



all:	mpdv mpdvi mpdvsumm

mpdv:	../paths.sh setup.sh mpdv.sh
	echo "#!/bin/sh"					 >mpdv
	echo "#"						>>mpdv
	echo "#  mpdv - verify MPD system"			>>mpdv
	cat ../paths.sh setup.sh mpdv.sh			>>mpdv
	chmod +x mpdv

mpdvi:	../paths.sh setup.sh mpdvi.sh
	echo "#!/bin/sh"					 >mpdvi
	echo "#"						>>mpdvi
	echo "#  mpdvi - install MPD verification program"	>>mpdvi
	cat ../paths.sh setup.sh mpdvi.sh			>>mpdvi
	chmod +x mpdvi

mpdvsumm:	../paths.sh setup.sh mpdvsumm.sh
	echo "#!/bin/sh"					 >mpdvsumm
	echo "#"						>>mpdvsumm
	echo "#  mpdvsumm - summarize mpdv output"		>>mpdvsumm
	cat ../paths.sh mpdvsumm.sh				>>mpdvsumm
	chmod +x mpdvsumm

clean:

depend:

lint:

tags:

TAGS:
