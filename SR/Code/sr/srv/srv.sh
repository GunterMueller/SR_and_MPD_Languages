#  srv code -- find and follow Script files in verification suite

TMP1=/tmp/srv$$a
TMP2=/tmp/srv$$b

# start at the top of the vsuite
cd $SRSRC/vsuite

# find all the Script files in the named subdirectories (or all, if none given).
# pipe through "sed" to remove leading and trailing noise.
# read the results to drive a loop.

for i in ${*-*}
do
    find $i -name Script -print | sort
done |
sed -e 's!\./!!' -e 's!/Script!!' |
while read subd
do
    # echo subdirectory and go there.  enable the "rm" command.
    echo "$subd:"
    cd $SRSRC/vsuite/$subd
    rm=rm

    # loop reading the Script file and process each line.
    while read code verb args		# reading <Script -- redirected below
    do
	# skip comments or empty line
	case x$code in
	    x | x#*)	continue;;
	    *)		;;
	    esac

	# echo if tracing was set
	$trace " " "$code" "$verb" "$args"

	# process certain command verbs specially:
	#    use appropriate versions of SR system components
	#    treat "run" as a special case macro
	#    honor "rm" only if not disabled by earlier errors
	case $verb in

	    sr)      $sr  $ec       $args >Compiler.out 2>&1;
		     result=$?			# save $?
		     grep '\.c".*warning:' Compiler.out | \
			grep -v 'statement not reached';
		     (exit $result);;		# set $?

	    srl)     $srl $er       $args >Linker.out   2>&1;;
	    srm)     $srm -w -c $sr $args >Maker.out    2>&1;;
	    srprof)  $srprof	    $args >Profiler.out 2>&1;;
	    srgrind) $srgrind       $args >Grinder.out  2>&1;;
	    srtex)   $srtex         $args >Texer.out    2>&1;;
	    srlatex) $srlatex       $args >Latexer.out  2>&1;;
	    ccr2sr)  $ccr2sr $ec    $args >CCR.out	2>&1;;
	    m2sr)    $m2sr $ec	    $args >M.out	2>&1;;
	    csp2sr)  $csp2sr $ec    $args >CSP.out	2>&1;;

	    run)
		# check args to set stdin and stdout
		case x$args in
		    x)			# no arguments
			ifile=/dev/null
			ofile=No_input;;
		    *)			# arguments present
			set $args
			ifile=$1	 	# use first as file name
			ofile=$1
			shift
			args="$*";;		# pass the rest to the program
		    esac
		# run a.out and save result
		a.out <$ifile >$ofile.out 2>&1 $args
		result=$?
		# compare output if exit code as expected
		case $result in
		    $code)
			verb=CMP
			code=0			# expect 0 from cmp
			cmp -s $ofile.std $ofile.out
			result=$?;;
		esac
		(exit $result);;		# set $?

	    scmp)
		# sorted file comparison
		set $args
		sort $1 >$TMP1
		sort $2 >$TMP2
		cmp -s $TMP1 $TMP2
		result=$?
		rm $TMP1 $TMP2
		(exit $result);;		# set $?

	    rm)     $rm $args;;			# special case rm

	    *)      eval $verb "$args";;	# unrecognized, just execute it

	    esac

	# check if result was what we expected
	result=$?
	case $verb$result in
	    $verb$code)			# yes, back to top of loop
		continue;;
	    CMP1)			# no, output differed
		$NECHO "    output differs";;
	    CMP2)
		$NECHO "    missing $ofile.std to compare output";;
	    *) 				# no, other problem
		$NECHO "    expected $code, got $result"
		case $result in		# diagnose if known
		    129)  $NECHO ' (hangup)';;
		    132)  $NECHO ' (illegal instruction)';;
		    138)  $NECHO ' (bus error)';;
		    139)  $NECHO ' (segmentation violation)';;
		    esac;;
	    esac

	rm=:			# disable rm due to unexpected results
	case $verb in
	    run | CMP)
		echo " from a.out <$ifile";
		continue;;	# continue loop if difference in output
	    *)
		echo " from $verb"
		break;;		# else quit this subdirectory on other error
	    esac

	done <Script

    done  # <|find

date
exit 0

