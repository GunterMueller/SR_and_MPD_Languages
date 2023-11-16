#  This script reads an output file from srv, scans the vsuite,
#  and prints a summary.  It requires a modern version of awk,
#  and doesn't work on all systems (in particular SGI Iris).

# use nawk if it exists, else use awk and hope it's the modern version

if ( nawk '{print}' </dev/null ) >/dev/null 2>&1
    then AWK=nawk
    else AWK=awk
    fi

# pipe first through sed and then through nawk

sed '

/^[MTWFS].. ... [ 0123][0-9] ..:..:../s/^/#  /
/^SR version /s/^/#  /
/^MultiSR version /s/^/#  /
/^HOST=.* SR_PARALLEL=/s/^/#  /
/^-r........ /s/^/#  /
/.*srv: [0-9]* Memory fault/s//srv: Memory fault/
/^[^ ]*:$/i\
---end-of-test---
$a\
---end-of-test---

' $* | $AWK '

function oops()		{ if (!errs) printf ("\n%s:\n", TST); print; errs++; }
function note(n,s)	{ if (n) printf ("#  %3d %s\n", n, s); }

BEGIN	{
	VSUITE = "'$SRSRC/vsuite'"
	NOK = -1
	efile["sr"] = "Compiler.out"
	efile["srl"] = "Linker.out"
	efile["srm"] = "Maker.out"
	}

/^#/	{ print; next }

/^---end-of-test---$/ {
	if (errs && !reason) NOTHER++;
	if (errs) NBAD++; else NOK++;
	errs = reason = 0
	next
	}

/^[^ ]*:$/	{
	TST = substr ($0, 1, length($0)-1)
	NTESTS++
	next
	}

/^    output differs /  ||  /expected 0, got 1 *from cmp/  {
	oops()
	reason++
	NDIFF++
	next
	}

/^    expected [1-9][0-9]*, got [1-9][0-9]* *from sr/ {
	oops()
	if (!reason++) {
	    NBOGUS++
	    print "\t***** bogus test?"
	}
	next
	}

/^    expected [0-9]*, got [0-9]* *from / {
	oops()
	errcount[$NF]++
	if (NF > 6 && $6 == "a.out") {
	    if ($NF == "</dev/null")
		fname = "No_input.out"
	    else
		fname = substr ($NF, 2) ".out"
	    skipto = "warning:|abort:"
	} else {
	    fname = efile[$NF]
	    skipto = ""
	}
	if (fname == "") {
	    fname = "No_input.out"
	    skipto = "warning:|abort:|failure:"
	}
	fname = VSUITE "/" TST "/" fname
	while (getline <fname > 0) {
	    if (skipto != "") {
		while (! match ($0, skipto))
		    if (getline <fname <= 0)
			{ $0 = ""; break }
		skipto = ""
	    }
	    print "\t" $0
	    if (!reason++) {
		if (/parse error/)
		    NPARSE++
		else if (/slice parameter/)
		    NSLICE++
		else if (/swap operator/)
		    NSWAP++
		else if (/ld: Undefined symbol/)
		    NUNDEF++
		else if (/COMPILER MALFUNCTION/) {
		    if (/segmentation violation/)  NSEGM++;
		    else if (/records yet/)  NREC++;
		    else NMALF++;
		} else
		    reason--	# did not deduce it after all
	    }
	}
	close (fname)
	next
	}

/./	{
	oops()
	}

END	{
	print
	note(NTESTS,"total tests")
	note(NOK,"passed")
	note(NBAD,"failed")
	print
	note(NSEGM,  "got segmentation violation in compiler")
	note(NMALF,  "got compiler malfunction")
	note(NPARSE, "had parse errors")
	note(NREC,   "used records")
	note(NSWAP,  "used swap operator")
	note(NSLICE, "used slice parameters")
	note(NUNDEF, "had undefined global symbols")
	note(NDIFF,  "produced wrong output")
	note(NBOGUS, "looked bogus")
	note(NOTHER, "failed for other reasons")
	print
	note(errcount["sr"],"failed in compiler")
	note(errcount["srl"],"failed in linker")
	note(NBAD-errcount["sr"]-errcount["srl"],"failed elsewhere")
	}
'
