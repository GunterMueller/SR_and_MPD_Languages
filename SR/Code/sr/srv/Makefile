##	Makefile for the SR validation scripts: srv, srvi, srvsumm



SHELL = /bin/sh



all:	srv srvi srvsumm

srv:	../paths.sh setup.sh srv.sh
	echo "#!/bin/sh"					 >srv
	echo "#"						>>srv
	echo "#  srv - verify SR system"			>>srv
	cat ../paths.sh setup.sh srv.sh				>>srv
	chmod +x srv

srvi:	../paths.sh setup.sh srvi.sh
	echo "#!/bin/sh"					 >srvi
	echo "#"						>>srvi
	echo "#  srvi - install SR verification program"	>>srvi
	cat ../paths.sh setup.sh srvi.sh			>>srvi
	chmod +x srvi

srvsumm:	../paths.sh setup.sh srvsumm.sh
	echo "#!/bin/sh"					 >srvsumm
	echo "#"						>>srvsumm
	echo "#  srvsumm - summarize srv output"		>>srvsumm
	cat ../paths.sh srvsumm.sh				>>srvsumm
	chmod +x srvsumm

clean:

depend:

lint:

tags:

TAGS:
