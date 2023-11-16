#!/bin/sh
#
#  errck  <program  scratchfile.sr  expected_error_message
#
#  Checks that SR gives the right error message for a bogus program.

#  Warning to would-be hackers: do not try to add grep arguments -e or -s;
#  they are nonportable.

sr=${sr-sr}
FILE=${1?"missing filename"}
MSG="${2?'missing errmsg'}"
RM="rm -f"

cat >$FILE

rm -f Interfaces/*
$sr -C $FILE >Compiler.out 2>&1
X=$?
case $X in
    0)	if echo "$MSG" | grep warning: >/dev/null; then :; else
	    echo "$FILE: status = 0 ($MSG)"
	    RM=:
	fi;;
    [1-9])
	;;
    *)	echo "$FILE: status = $X"
	RM=:
	;;
esac

grep ".$MSG" Compiler.out >/dev/null
case $? in
    0)	if grep MALFUNCTION Compiler.out >/dev/null; then
	    echo "$FILE: MALFUNCTION ($MSG)"
	else
	    echo "ok:  $MSG"
	    $RM $FILE
	fi;;
    1)  echo "$FILE: FAILED ($MSG)";;
    *)  echo "$FILE: BOOM ($MSG)";;
esac
exit 0
