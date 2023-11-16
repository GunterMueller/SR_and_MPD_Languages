#!/bin/sh
#
#  Install SR.

. ./paths.sh
if [ -z "$SRCMD" -o -z "$SRLIB" ]; then
    echo 'cannot install -- SRCMD or SRLIB is null' 1>&2
    exit 1
fi

if [ ! -d "$SRCMD" -o ! -d "$SRLIB" ]; then
    echo "cannot install -- $SRCMD or $SRLIB is not a directory" 1>&2
    exit 1
fi

EXT3=`echo $MANEXT | tr 1 3`
EXT5=`echo $MANEXT | tr 1 5`

set -x

# commands

cp sr/sr           $SRCMD;  strip $SRCMD/sr
cp srl/srl         $SRCMD;  strip $SRCMD/srl
cp srm/srm         $SRCMD;  strip $SRCMD/srm
cp srprof/srprof   $SRCMD;  strip $SRCMD/srprof
cp srtex/srtex     $SRCMD;  strip $SRCMD/srtex
cp srlatex/srlatex $SRCMD;  strip $SRCMD/srlatex
cp srgrind/srgrind $SRCMD
cp preproc/*2sr    $SRCMD

# library components

cp sr.h            $SRLIB
cp srmulti.h       $SRLIB
cp rts/srlib.a     $SRLIB
if [ -f /bin/ranlib -o -f /usr/bin/ranlib ]; then ranlib $SRLIB/srlib.a; fi
cp library/*.o     $SRLIB
cp library/*.spec  $SRLIB
cp library/*.impl  $SRLIB
cp srmap           $SRLIB
cp rts/srx         $SRLIB;  strip $SRLIB/srx
cp srlatex/srlatex.sty $SRLIB
cp preproc/*2sr.h  $SRLIB
cp sr-mode.el      $SRLIB

# man pages

if [ ! -z "$MAN1" -a -d "$MAN1" -a ! -z "$MANEXT" ]; then
    cp man/sr.1        $MAN1/sr.$MANEXT
    cp man/srl.1       $MAN1/srl.$MANEXT
    cp man/srm.1       $MAN1/srm.$MANEXT
    cp man/srprof.1    $MAN1/srprof.$MANEXT
    cp man/srtex.1     $MAN1/srtex.$MANEXT
    cp man/srlatex.1   $MAN1/srlatex.$MANEXT
    cp man/srgrind.1   $MAN1/srgrind.$MANEXT
    cp man/ccr2sr.1    $MAN1/ccr2sr.$MANEXT
    cp man/m2sr.1      $MAN1/m2sr.$MANEXT
    cp man/csp2sr.1    $MAN1/csp2sr.$MANEXT
else
    echo 'not installing man pages for commands' 1>&2
fi

if [ ! -z "$MAN3" -a -d "$MAN3" -a ! -z "$EXT3" ]; then
    cp man/sranimator.3 $MAN3/sranimator.$EXT3
    cp man/srgetopt.3  $MAN3/srgetopt.$EXT3
    cp man/srwin.3     $MAN3/srwin.$EXT3
else
    echo 'not installing sranimator/srgetopt/srwin man pages' 1>&2
fi

if [ ! -z "$MAN5" -a -d "$MAN5" -a ! -z "$EXT5" ]; then
    cp man/srmap.5     $MAN5/srmap.$EXT5
    cp man/srtrace.5   $MAN5/srtrace.$EXT5
else
    echo 'not installing srmap/srtrace man pages' 1>&2
fi

# we don't install srv anywhere because it's just a development tool.

exit 0
