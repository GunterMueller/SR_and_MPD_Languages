#  common parameter cracking code and other initialization for mpdv and mpdvi

# always run with standard umask
umask $VMASK

# make sure search path includes "."; put it at the end
PATH="$PATH:."
export PATH

# set paths to experimental versions by default
mpd=$MPDSRC/mpd/mpd
mpdl=$MPDSRC/mpdl/mpdl
rts=$MPDSRC/rts/mpdlib.a
mpdx=$MPDSRC/rts/mpdx
map=$MPDSRC/mpdmap
mpdm=$MPDSRC/mpdm/mpdm
mpdprof=$MPDSRC/mpdprof/mpdprof
# and set experimental flags
ec=-e
er=-e

# use necho script if "echo -n" doesn't work.  export for use by scripts.
if [ "X`echo -n YZ`" = "XYZ" ]
    then NECHO='echo -n'
    else NECHO=$MPDSRC/mpdv/necho.sh
    fi
export NECHO

# determine hostname
HOST=`hostname 2>/dev/null || uname -n`
if [ -z "$HOST" ]; then
    echo 1>&2 "can't figure out host name"
    exit 1
fi
if [ `uname -m 2>/dev/null` = 'paragon' ]; then
   HOST1="1"
   HOST2="2"
else
   HOST1=$HOST
   HOST2=$HOST
fi
export HOST HOST1 HOST2

# by default, don't trace in mpdv
trace=:

# process command options; some of these don't do anything in mpdvi
for i
do
    case $i in
	-p)			# -p: production versions of everything
	    mpd=$MPDCMD/mpd
	    mpdl=$MPDCMD/mpdl
	    rts=$MPDLIB/mpdlib.a
	    mpdx=$MPDLIB/mpdx
	    map=$MPDLIB/mpdmap
	    mpdm=$MPDCMD/mpdm
	    mpdprof=$MPDCMD/mpdprof
	    ec=
	    er=
	    shift;;
	-c)			# -c: production compiler
	    mpd=$MPDCMD/mpd
	    ec=
	    shift;;
	-l)			# -l: production linker
	    mpdl=$MPDCMD/mpdl
	    shift;;
	-r)			# -r: production runtime system
	    rts=$MPDLIB/mpdlib.a
	    mpdx=$MPDLIB/mpdx
	    map=$MPDLIB/mpdmap
	    er=
	    shift;;
	-t)			# -t: production versions of other tools
	    mpdm=$MPDCMD/mpdm
	    mpdprof=$MPDCMD/mpdprof
	    shift;;
	-v)			# -v: trace commands as read from script
	    trace=echo
	    shift;;
	-*)
	    echo 1>&2 \
		"usage: $0 [-p] [-c] [-l] [-r] [-t] [-v] dir"
	    exit 1;;
	*)
	    break;;
	esac
    done

# print date, hostname, MPD_PARALLEL value, MPD version number
date
echo "HOST=$HOST  MPD_PARALLEL=${MPD_PARALLEL-(unset)}"
if [ -r $mpd ]; then
    $mpd -v 2>&1
fi

# list the versions we will be using
ls -l $mpd $mpdl
ls -l $rts $mpdx

# make sure they're accessible
if [ ! '(' -r $mpd -a -r $mpdl -a -r $rts -a -r $mpdx ')' ]
    then
	echo $0: access tests failed 1>&2
	exit 1
    fi

# ensure a standard environment
SHELL=/bin/sh;	export SHELL
MPD_PATH=;	export MPD_PATH
MPD_TRACE=;	export MPD_TRACE
MPD_DEBUG=;	export MPD_DEBUG
MPDMOPTS=;	export MPDMOPTS
LC_ALL=C;	export LC_ALL

# set path for network mapping file
MPDMAP=$map
export MPDMAP
