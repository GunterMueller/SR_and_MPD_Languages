#!/bin/sh
#
#   mkscript.sh [file] -- make shell script for checking SR programs
#
#   Input is a file of SR programs, each beginning with a "%..." line.
#   Output is a series of executions of "$CHECKER filename errmsg" where
#	$CHECKER comes from the definition below
#	filename is a unique .sr file name
#	errmsg is the remainder of the "%..." line beyond the "%"
#   with standard input containing the SR program.

CHECKER="sh ../errck.sh"

echo "#!/bin/sh"
echo "#"
echo "#  script created by $0 $*"
echo ""

awk '

BEGIN { print "cat <<@EOF >/dev/null" }

/^%/ {
	print "@EOF"
	errmsg = substr($0, 3)
	printf ("'"$CHECKER"' <<'"'@EOF'"' test%d.sr \"%s\"\n", ++i, errmsg)
	print "#" $0
	next
      }

     { print }

END  { print "@EOF" 
       print "exit 0"
     }
' $*

exit 0
