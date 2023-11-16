#!/bin/sh
#
#  usage:  sh run.sh [file...]

if [ $# = 0 ]
then
  set - *.in
fi

echo "testing scanf:"
for i
do
  TST=`basename $i .in`
  echo "  testing" $TST
  ./scan $TST.in > $TST.out 2>&1
  diff $TST.std $TST.out
done

echo "testing sscanf:"
for i
do
  TST=`basename $i .in`
  echo "  testing" $TST
  ./sscan $TST.in > $TST.sout 2>&1
  diff $TST.sstd $TST.sout
done

exit 0
