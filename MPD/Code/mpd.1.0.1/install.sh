#!/bin/sh
#
#  Install MPD.

. ./paths.sh
if [ -z "$MPDCMD" -o -z "$MPDLIB" ]; then
    echo 'cannot install -- MPDCMD or MPDLIB is null' 1>&2
    exit 1
fi

if [ ! -d "$MPDCMD" -o ! -d "$MPDLIB" ]; then
    echo "cannot install -- $MPDCMD or $MPDLIB is not a directory" 1>&2
    exit 1
fi

EXT3=`echo $MANEXT | tr 1 3`
EXT5=`echo $MANEXT | tr 1 5`

set -x

# commands

cp mpd/mpd         $MPDCMD;  strip $MPDCMD/mpd
cp mpdl/mpdl       $MPDCMD;  strip $MPDCMD/mpdl
cp mpdm/mpdm       $MPDCMD;  strip $MPDCMD/mpdm
cp mpdprof/mpdprof $MPDCMD;  strip $MPDCMD/mpdprof

# library components

cp mpd.h           $MPDLIB
cp mpdmulti.h      $MPDLIB
cp rts/mpdlib.a     $MPDLIB
if [ -f /bin/ranlib -o -f /usr/bin/ranlib ]; then ranlib $MPDLIB/mpdlib.a; fi
cp library/*.o     $MPDLIB
cp library/*.spec  $MPDLIB
cp library/*.impl  $MPDLIB
cp mpdmap          $MPDLIB
cp rts/mpdx        $MPDLIB;  strip $MPDLIB/mpdx

# man pages

if [ ! -z "$MAN1" -a -d "$MAN1" -a ! -z "$MANEXT" ]; then
    cp man/mpd.1       $MAN1/mpd.$MANEXT
    cp man/mpdl.1      $MAN1/mpdl.$MANEXT
    cp man/mpdm.1      $MAN1/mpdm.$MANEXT
    cp man/mpdprof.1   $MAN1/mpdprof.$MANEXT
else
    echo 'not installing man pages for commands' 1>&2
fi

if [ ! -z "$MAN3" -a -d "$MAN3" -a ! -z "$EXT3" ]; then
    cp man/mpdanimator.3 $MAN3/mpdanimator.$EXT3
    cp man/mpdgetopt.3  $MAN3/mpdgetopt.$EXT3
    cp man/mpdwin.3     $MAN3/mpdwin.$EXT3
else
    echo 'not installing mpdanimator/mpdgetopt/mpdwin man pages' 1>&2
fi

if [ ! -z "$MAN5" -a -d "$MAN5" -a ! -z "$EXT5" ]; then
    cp man/mpdmap.5    $MAN5/mpdmap.$EXT5
    cp man/mpdtrace.5   $MAN5/mpdtrace.$EXT5
else
    echo 'not installing mpdmap/mpdtrace man pages' 1>&2
fi

# we don't install mpdv anywhere because it's just a development tool.

exit 0
