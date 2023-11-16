#!/bin/sh
#
#  run.sh [NTIMES [NLOOPS]]
#
#  run timing tests, assuming programs have already been compiled

NTIMES=${1-10000}
NLOOPS=${2-5}

rm -f run.out

cat List | while read DIR SCALE DESCR
do
    case $DIR in
	"#"*|"")	continue;;
    esac
    echo "  cd $DIR; ./a.out $NTIMES $SCALE $NLOOPS >$DIR.out"
    (cd $DIR; ./a.out $NTIMES $SCALE $NLOOPS >$DIR.out)
done

date >run.out
echo hostname: `hostname || uname` >>run.out

exit 0
