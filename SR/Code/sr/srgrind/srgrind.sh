#   This script calls the undocumented vgrind back end "vfontedpr"
#   directly because /usr/ucb/vgrind is badly (& differently) broken
#   on both Vax (4.3 Mt Xinu) and Sun (3.5); for example, try a  simple
#   "TROFF=cat vgrind foo.c".

#   Note that we allow "-s 12" and "-dfile" as well as the "-s12" and "-d file"
#   forms vgrind insists on; but we still don't allow "-fn" for "-f -n".

#   vgrindefs below are based on efforts of Roger Hayes, Doug Cook, Ron Olsson.

USAGE="[-f] [-n] [-w] [-x] [-s size] [-d defs] [-h header] [file...]"
TEMP=/tmp/srg.$$


# first, check that vgrind really exists
if [ -z "$VFPATH" ]
then
    echo "$0: srgrind is not available on this system" 1>&2
    exit 1
fi


# copy default vgrindefs(5) to a temp file
#
# strings don't really end at EOL, but we'll say they do to help
# resynchronize things if vgrind gets confused

trap 'x=$?; rm -f $TEMP; exit $x' 0 1 2 15
cat >$TEMP <<"==END=="
sr|SR:\
	:pb=\d(((proc(ess|edure)?))|resource|global)\d\p(\d|\())|(body\d\p\d?(/|#|$)):\
	:bb=\d(in|if|fa|do|co|in|begin|initial|final)\d:\
	:be=\d(fi|af|od|oc|ni|end)\d:\
	:cb=#:ce=$:\
	:ab=/*:ae=*/:\
	:sb=":se=(\e"|$):\
	:lb=':le=(\e'|$):\
	:kw=P V af and any begin body bool by call cap char co const create \
	destroy do downto else end enum exit extend external fa false fi file \
	final forward global high if import in initial int low mod new next \
	ni noop not null oc od on op optype or proc procedure process ptr \
	real rec receive ref reply res resource return returns sem send \
	separate skip st stop string to true type union val var vm xor:
==END==


# process command options

DEFS=$TEMP
OPTIONS=
while test $# != 0
do
    case "$1" in
	-[fnx])
	    OPTIONS="$OPTIONS $1"
	    shift;;
	-w)
	    OPTIONS="$OPTIONS -t"
	    shift;;
	-s)
	    shift
	    test $# '=' 0 && echo "$0: $1 requires a value" 1>&2 && exit 1
	    OPTIONS="$OPTIONS -s$1"
	    shift;;
	-s?*)
	    OPTIONS="$OPTIONS $1"
	    shift;;
	-d)
	    shift
	    test $# '=' 0 && echo "$0: $1 requires a value" 1>&2 && exit 1
	    DEFS="$1"
	    shift;;
	-d?*)
	    val=`echo $1 | sed 's/..//'`
	    OPTIONS="$OPTIONS -d $val"
	    shift;;
	-h)
	    shift
	    test $# '=' 0 && echo "$0: $1 requires a value" 1>&2 && exit 1
	    OPTIONS="$OPTIONS -h '$1'"
	    shift;;
	-h?*)
	    val=`echo "$1" | sed 's/..//'`
	    OPTIONS="$OPTIONS -h '$val'"
	    shift;;
	--)
	    shift
	    break;;

	-*)
	    echo "usage: $0 $USAGE" 1>&2
	    exit 1;;
	*)
	    break;;
	esac
    done


# turn a null file list into "-" for the back end

case $# in
    0)	FILES="-";  break;;
    *)	FILES="$@"; break;;
    esac


# insert a call for the macros, and then call the back end
# "eval" is used to get quoting right (e.g. "srgrind -h 'tricky header')

echo ".so $VGMACS"
eval $VFPATH -lsr -d $DEFS $OPTIONS $FILES
exit
