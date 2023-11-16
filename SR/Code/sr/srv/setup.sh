#  common parameter cracking code and other initialization for srv and srv

# always run with standard umask
umask $VMASK

# make sure search path includes "."; put it at the end
PATH="$PATH:."
export PATH

# set paths to experimental versions by default
sr=$SRSRC/sr/sr
srl=$SRSRC/srl/srl
rts=$SRSRC/rts/srlib.a
srx=$SRSRC/rts/srx
map=$SRSRC/srmap
srm=$SRSRC/srm/srm
srprof=$SRSRC/srprof/srprof
srgrind=$SRSRC/srgrind/srgrind
srtex=$SRSRC/srtex/srtex
srlatex=$SRSRC/srlatex/srlatex
ccr2sr=$SRSRC/preproc/ccr2sr
m2sr=$SRSRC/preproc/m2sr
csp2sr=$SRSRC/preproc/csp2sr
# and set experimental flags
ec=-e
er=-e

# use necho script if "echo -n" doesn't work.  export for use by scripts.
if [ "X`echo -n YZ`" = "XYZ" ]
    then NECHO='echo -n'
    else NECHO=$SRSRC/srv/necho.sh
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

# by default, don't trace in srv
trace=:

# process command options; some of these don't do anything in srvi
for i
do
    case $i in
	-p)			# -p: production versions of everything
	    sr=$SRCMD/sr
	    srl=$SRCMD/srl
	    rts=$SRLIB/srlib.a
	    srx=$SRLIB/srx
	    map=$SRLIB/srmap
	    srm=$SRCMD/srm
	    srprof=$SRCMD/srprof
	    srgrind=$SRCMD/srgrind
	    srtex=$SRCMD/srtex
	    srlatex=$SRCMD/srlatex
	    ccr2sr=$SRCMD/ccr2sr
	    m2sr=$SRCMD/m2sr
	    csp2sr=$SRCMD/csp2sr
	    ec=
	    er=
	    shift;;
	-c)			# -c: production compiler
	    sr=$SRCMD/sr
	    ec=
	    shift;;
	-l)			# -l: production linker
	    srl=$SRCMD/srl
	    shift;;
	-r)			# -r: production runtime system
	    rts=$SRLIB/srlib.a
	    srx=$SRLIB/srx
	    map=$SRLIB/srmap
	    er=
	    shift;;
	-t)			# -t: production versions of other tools
	    srm=$SRCMD/srm
	    srprof=$SRCMD/srprof
	    srgrind=$SRCMD/srgrind
	    srtex=$SRCMD/srtex
	    srlatex=$SRCMD/srlatex
	    ccr2sr=$SRCMD/ccr2sr
	    m2sr=$SRCMD/m2sr
	    csp2sr=$SRCMD/csp2sr
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

# print date, hostname, SR_PARALLEL value, SR version number
date
echo "HOST=$HOST  SR_PARALLEL=${SR_PARALLEL-(unset)}"
if [ -r $sr ]; then
    $sr -v 2>&1
fi

# list the versions we will be using
ls -l $sr $srl
ls -l $rts $srx

# make sure they're accessible
if [ ! '(' -r $sr -a -r $srl -a -r $rts -a -r $srx ')' ]
    then
	echo $0: access tests failed 1>&2
	exit 1
    fi

# ensure a standard environment
SHELL=/bin/sh;	export SHELL
SR_PATH=;	export SR_PATH
SR_TRACE=;	export SR_TRACE
SR_DEBUG=;	export SR_DEBUG
SRMOPTS=;	export SRMOPTS
LC_ALL=C;	export LC_ALL

# set path for network mapping file
SRMAP=$map
export SRMAP
