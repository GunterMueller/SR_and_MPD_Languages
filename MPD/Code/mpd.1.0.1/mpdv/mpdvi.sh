#  mpdvi code -- read info and install program

# check for exactly one parameter
case $# in
    1)	;;
    *)	echo "usage: $0 [options] dir" 1>&2; exit 1;;
    esac

# set up some strings
dir=$MPDSRC/vsuite/$1
script=$dir/Script
date=`date`
clean="rm -rf MPDinter core *.out"

# make the subdir; error if it already has a Script file
# (no error if already exists; earlier mpdvi may have failed partway)
if [ -d $dir ]
    then if [ -r $script ]
	then echo "$dir already exists" 1>&2; exit 1;
	fi
    else if mkdir $dir
	then :
	else echo "$dir: can't mkdir" 1>&2; exit 1;
	fi
    fi

# read list of .mpd files and check accessibility
$NECHO ".mpd files?   "
read mpdfiles
if [ -z "$mpdfiles" ]
    then echo "no source files!"; exit 1;
    fi
for i in $mpdfiles
    do
    if [ ! -r $i ]
	then echo "$i: can't access"; exit 1;
	fi
    done

# read list of data files and check accessibility
$NECHO "input files? "
read ifiles
for i in $ifiles
    do
    if [ ! -r $i ]
	then echo "$i: can't access"; exit 1;
	fi
    done

# at this point we're committed to filling the subdirectory.

set -x					# echo our commands from here on
echo "#  $USER  $date" >$script		# init Script with user name and date
cp $mpdfiles $ifiles $dir		# copy source and data files
cd $dir					# move to new directory
if $mpd $ec $mpdfiles >Compiler.std 2>&1	# compile
    then
	echo "0 mpd $mpdfiles" >>$script	# record command
	if [ -z "$ifiles" ]
	    then  
		a.out </dev/null >No_input.std 2>&1	# run </dev/null
		echo "$? run" >>$script			# record command
	    else for i in $ifiles
		do
		    a.out <$i >$i.std 2>&1		# run <datafile
		    echo "$? run $i" >>$script		# record command
		done
	    fi
    else
	echo "$? mpd -c $mpdfiles" >>$script	# record failing compile
	echo "warning: compilation failed"
    fi

$clean					# clean up the subdirectory
echo "0 $clean" >>$script		# record a cleanup command
find C*.std -size 0 -exec rm {} \;	# remove compiler output if null
exit 0
