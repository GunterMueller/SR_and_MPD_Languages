#!/bin/sh
#
#  build.sh [compiler-path] -- build benchmark programs for timing

SR=${1-sr}

rm -f build.out

$SR -v 2>&1 || exit 1

cat List | while read DIR SCALE DESCR
do
    case $DIR in
	"#"*|"")	continue;;
    esac
    echo "  cd $DIR; $SR -O $DIR.sr"
    (cd $DIR; $SR -O $DIR.sr)
done

echo $SR >build.out
$SR -v >>build.out 2>&1

exit 0
