#!/bin/sh
#
#  build.sh [compiler-path] -- build benchmark programs for timing

MPD=${1-mpd}

rm -f build.out

$MPD -v 2>&1 || exit 1

cat List | while read DIR SCALE DESCR
do
    case $DIR in
	"#"*|"")	continue;;
    esac
    echo "  cd $DIR; $MPD -O $DIR.mpd"
    (cd $DIR; $MPD -O $DIR.mpd)
done

echo $MPD >build.out
$MPD -v >>build.out 2>&1

exit 0
