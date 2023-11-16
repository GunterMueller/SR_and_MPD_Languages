.TH SRGETOPT 3 "5 October 1994" "University of Arizona" "SR Library"
.SH NAME
srgetopt \- parse command arguments
.SH SYNOPSIS
\fBimport SRgetopt\fP
.br
\fBdo c := getopt(\fIoptstring\fB) != EOF \-> \fIprocess option c \fBod\fR
.SH DESCRIPTION
.LP
.I SRgetopt
provides a means for an SR program to parse command
line arguments in accordance with the standard Unix conventions;
it is analogous to, and based on,
.IR getopt (3)
for C programs.
.I SRgetopt
is a global containing one procedure,
.I getopt,
and variables that control its behavior or return additional information.
.LP
.I Getopt
interprets command arguments in accordance with the standard
Unix conventions: option arguments are introduced by "\-"
followed by a key character; an argument value follows certain keys.
Multiple keys can be combined, as in "\-ab", if they do not require arguments.
A non-option argument terminates the processing of options,
as does the special argument "\-\^\-".
.LP
Option interpretation is controlled by the parameter
.I optstring,
which specifies the characters that designate
legal options and indicates which ones require associated values.
The call \fBgetopt("ab")\fP specifies that the command
line should contain only the options "\-a" and "\-b".
If a letter in
.I optstring
is followed by a colon, the option is expected to have an argument.
The argument may or may not be separated by whitespace from the option letter.
For example, \fBgetopt("w:")\fP accepts either "\-w\080" or "\-w80".
.LP
Each call to
.I getopt
returns the key of the next command line argument;
this key must match a letter in
.I optstring.
If the option accepts an argument, the string variable
.I optarg
is set to the argument value.
Predefined conversion functions such as
.I int,
.I char,
etc. can then be applied to
.I optarg.
The constant
.I optMAXLEN
defines the length of the longest string that
.I getopt
can handle; extra characters are truncated silently.
.LP
.I Getopt
places in the variable
.I optind
the index of the next command line argument to be processed;
.I optind
is automatically initialized to 1 before the first call to
.I getopt.
.LP
When all options have been processed, and only non-option arguments remain,
.I getopt
returns
.I optEOF.
The remaining arguments can be retrieved using the predefined function
.I getarg,
beginning with argument number
.I optind.
.SH DIAGNOSTICS
.LP
.I Getopt
prints an error message on
.B stderr
and returns a question mark
(\(fm?\(fm) when it encounters a command line argument not matched by
.I optstring.
Setting the variable
.I opterr
to
.B false
disables this error message.
.SH NOTES
.LP
The following notes describe
.IR SRgetopt 's
behavior in a few interesting or special cases;
this behavior is consistent with
.IR getopt (3).
.LP
A \(fm\-\(fm by itself is treated as a non-option argument.
By convention, most programs interpret this as specifying the use of
.B stdin
or
.BR stdout ,
depending on context, in place of a named file.
.LP
If
.I optstring
is "a:" and the command line arguments are "\-a \-x",
then "\-x" is treated as the argument associated with the "\-a".
.LP
Duplicate command line options are allowed; it is up to user to
deal with them appropriately.
.LP
An option ``letter'' can be a letter, number, or almost any special character.
Like
.IR getopt (3),
.I SRgetopt
disallows a colon as an option letter.
.br
.ne 40
.SH EXAMPLE
.LP
The following code fragment shows how to use
.I SRgetopt
to process a command that takes the options \(fma\(fm, \(fmf\(fm, and \(fmw\(fm
where \(fmf\(fm is followed by a file name and \(fmw\(fm is followed by an
integer.
.LP
.nf
.ta 4n 8n 12n 16n 20n
resource main()
.sp .4
	import SRgetopt
	var ch: char
.sp .4
	# command line arguments
	var aflg := 0
	var filename: string[optMAXLEN] := "out"
	var width := 80
.sp .4
	do (ch := getopt("abf:w:")) != optEOF \->
		if ch = \(fma\(fm \->
			aflg++
		[] ch = \(fmf\(fm \->
			filename := optarg
		[] ch = \(fmw\(fm \->
			width := int(optarg)
		[] else \->
			stop(1)
		fi
	od
.sp .4
	write("\-a", aflg)
	write("\-f", filename)
	write("\-w", width)
.sp .4
	fa k := optind to numargs() \->
		var xx: string[40]
		getarg(k,xx)
		write("normal argument", k, "is", xx)
	af
.sp .4
end
.fi
.SH SEE ALSO
getopt(3)
.SH CAVEATS
.LP
Changing the value of the variable
.I optind
may lead to unexpected behavior.
.LP
.I Getopt,
like the predefined functions
.I numargs
and
.I getarg,
is valid only on the main virtual machine.
