
LINKS = sr.h srmulti.h srmap sr-mode.el \
	sr/sr srl/srl rts/srlib.a rts/srx \
	library/SRWin.impl library/SRWin.o library/SRWin.spec \
	library/SRanimator.impl library/SRanimator.o \
	library/SRanimator.spec library/SRgetopt.impl \
	library/SRgetopt.o library/SRgetopt.spec \
	library/sranimator.o library/srwin.o srm/srm srprof/srprof \
	srgrind/srgrind srtex/srtex srlatex/srlatex srlatex/srlatex.sty \
	preproc/ccr2sr preproc/ccr2sr.h preproc/csp2sr preproc/csp2sr.h \
	preproc/m2sr preproc/m2sr.h srv/srv srv/srvi srv/srvsumm


links:
	for f in $(LINKS); do rm -f `basename $$f`; ln -s ../$$f .; done


clean:
	for f in $(LINKS); do rm -f `basename $$f`; done
