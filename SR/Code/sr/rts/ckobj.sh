#!/bin/sh
#
#  ckobj file... - validate .o files
#  Needed only for Dynix MultiSR; no effect in other environments.
#
#  Checks that each .o file was compiled with -Y for shared memory.
#  (Failure to compile with -Y leads to ghastly runtime failures.)

if cmp -s ../srmulti.h ../multi/dynix.h; then
    size $* | awk 1>&2 '
	BEGIN		{ code = 0; }
	NR==1		{ continue; }	# header line
	NF<7		{ print "not compiled with -Y:", $6; code = 1; }
	NF>6 && $4==0	{ print "not compiled with -Y:", $8; code = 1; }
	END		{ exit(code); }
    '
    exit
else
    exit 0
fi
