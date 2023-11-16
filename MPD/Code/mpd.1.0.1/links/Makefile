
LINKS = mpd.h mpdmulti.h mpdmap \
	mpd/mpd mpdl/mpdl rts/mpdlib.a rts/mpdx \
	library/MPDWin.impl library/MPDWin.o library/MPDWin.spec \
	library/MPDanimator.impl library/MPDanimator.o \
	library/MPDanimator.spec library/MPDgetopt.impl \
	library/MPDgetopt.o library/MPDgetopt.spec \
	library/mpdanimator.o library/mpdwin.o mpdm/mpdm mpdprof/mpdprof \
	mpdv/mpdv mpdv/mpdvi mpdv/mpdvsumm


links:
	for f in $(LINKS); do rm -f `basename $$f`; ln -s ../$$f .; done


clean:
	for f in $(LINKS); do rm -f `basename $$f`; done
