.TH SRMAP 5 "16 March 1992" "University of Arizona"
.SH NAME
srmap \- network mapping file for SR
.SH DESCRIPTION
.I Srmap
is used to find the executable program
when a virtual machine is created with an
.RB ` on
.IR n '
clause and an explicit pathname has not been given for machine
.I n.
.LP
.I Srmap
is read from a standard location;
this may be overridden by supplying the name of a file
in the environment variable
.I SRMAP.
.LP
Lines in the file have two whitespace-separated fields, a
.I pattern
and a 
.I template.
Lines with less than two fields are ignored.
Comments are introduced by `#' and terminate at end of line.
.LP
When an SR program runs, a program
.I location
is defined by concatenating the hostname, a colon (:), and
the absolute pathname of the executable.
The hostname is chopped at the first `.', if any.
The location is matched against each pattern in turn until one succeeds;
then the corresponding template is used to form a network pathname.
This pathname is passed to
.IR rsh (1)
on the remote machine.
.LP
Patterns have the general form
.I host:path,
with these characters having special meaning:
.RS
.ta 5n
?	matches any single character except `/'
.br
*	matches any string of characters except `/'
.br
**	matches any string of characters \fIincluding\fP `/'
.RE
.LP
Templates construct network file paths corresponding to matched patterns.
Each occurrence of
.BI $ n
in a template is replaced by the characters matched by the
.IR n th
?, *, or ** in the corresponding pattern.
.I n
may be 1 to 9, or A to Z for the 10th through 35th strings.
.SH EXAMPLE
Consider the mapping file:
.RS
.nf
.ta 12n 26n
# sample mapping file
*:/r/**	/r/$2	# already a network path
client:/**	/r/server/$1	# client disks are on server
???*:/**	/r/$1$2$3/$5	# general rule for others
.fi
.RE
.LP
A program run on host `caslon.arizona.edu' generates these
different network paths, depending on the program's path:
.RS
.nf
.I
.ta 24n
local path	network path generated
/r/bas/usr/abc/foo	/r/bas/usr/abc/foo
/usr/xyz/bar	/r/cas/usr/xyz/bar
.fi
.RE
.LP
Note that `caslon:' was prepended to the program path before
scanning the patterns.
.SH CAVEATS
.I Srmap
is designed for environments providing transparent access to
remote disks via a systematic naming scheme.
Other environments may require explicit path specification
by the SR program.
