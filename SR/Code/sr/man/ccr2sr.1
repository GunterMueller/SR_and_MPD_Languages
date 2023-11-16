.TH CCR2SR 1 "21 March 1995" "University of Arizona"
.SH NAME
ccr2sr \- CCR notation to SR code preprocessor
.SH SYNOPSIS
\fBccr2sr
[\|\-e\|]
\fRfile.ccr
.br
.SH DESCRIPTION
.LP
.I Ccr2sr
converts a program written in CCR (Conditional Critical Region) notation
into an equivalent one written in the SR language.
The SR program is placed in file.sr,
which may then be compiled and linked using
.IR sr (1).
The
.B \-e
(experimental) option
is for testing
.I ccr2sr.
.LP
The exact syntax of the CCR notation can be discerned
by examining the CCR programs in the examples directory.
Each CCR-notation construct begins with an underscore;
CCR code is mixed with regular SR code.
Syntactic peculiarities
exist to ease implementation.
.LP
.I Ccr2sr
translates a CCR resource into an SR global
and translates a CCR region statement
into code that uses Rem's algorithm.
Typically, the rest
of the user's code will be a single SR resource program.
The resultant SR code should be
run on only one virtual machine so that only one global is created.
Variables declared within CCR resources can be used only within region
statements.
.LP
One-dimensional and two-dimensional arrays of CCR resources are supported.
The bounds and subscripts for arrays
of CCR resources must be integers.
In conventional CCR notation (e.g., see Andrews's book),
each variable declared within a CCR resource
is replicated within each element of the CCR resource array.
In the 
.I ccr2sr
CCR notation,
such replication does
.I not
occur.
Instead, to effect such replication,
the programmer needs to augment the declaration
of each variable declared within a CCR resource array
with array bounds matching those for the CCR resource array;
an element of these arrays should be accessed only within
a region statement for the corresponding element of the CCR resource.
Hence,
former simple variables become arrays
and former arrays become higher-dimensional arrays.
A variable whose declaration is not augmented as described above
will be shared between all elements of the CCR resource array,
which is not conventional CCR semantics.
.LP
A number of syntactic limitations exist to ease the implementation.
Do not use names of SR predefined functions or reserved words
(e.g., free, exit, or skip) as variable names within CCRs.
Do not use identifiers beginning with r_ within CCR code.
Do not use any other SR synchronization or return/reply within a CCR program.
Do not use the # form of comment;
use only the /* */ form, but do not nest comments.
Do not use /% or %/.
Spaces in source text, even within string literals,
between the following pairs of characters will be deleted:
/ and %,
% and /,
/ and /,
[ and ],
* and *,
~ and =,
and : and =;
in addition spaces preceding : and = will also be deleted.
Within string literals,
use an escape character (\e) to retain the desired spacing.
Do not use the [a,b] form of subscripting a two-dimensional array as
part of a CCR construct (e.g., _region2); instead use the [a][b]
form.
.LP
The predefined SR procedure nap,
which puts the currently executing process to sleep for a while
and performs a context switch to another process,
is useful in CCR programs
to alter the interleaving of process execution.
For example, it can be used
with the random number functions to obtain different
interleavings to test whether a critical section algorithm works.
See the programs in the examples directory for examples.
.SH QUICK REFERENCE
.nf
.ta 28n
_resource(\fIname\fP)
_resource_end(\fIname\fP)
.sp .6
_region(\fIname,when\fP)	use \fBtrue\fP for \fIwhen\fP if empty
_region_end(\fIname\fP)
.sp .6
_resource1(\fIname,l1,u1\fP)	one-dim CCR resource \fIname[l1:u1]\fP
_resource_end1(\fIname\fP)
.sp .6
_region1(\fIname,v1,when\fP)	use \fBtrue\fP for \fIwhen\fP if empty; \fIv1\fP is subscript
_region_end1(\fIname,v1\fP)
.sp .6
_resource2(\fIname,l1,u1,l2,u2\fP)	two-dim CCR resource \fIname[l1:u1,l2:u2]\fP
_resource_end2(\fIname\fP)
.sp .6
_region2(\fIname,v1,v2,when\fP)	use \fBtrue\fP for \fIwhen\fP if empty; \fIv1\fP and \fIv2\fP are subscripts
_region_end2(\fIname,v1,v2\fP)
.fi
.SH FILES
.ta 20n
file.ccr	CCR source file
.br
file.sr	generated SR source file
.SH SEE ALSO
Gregory R. Andrews and Ronald A. Olsson,
.I "The SR Programming Language:  Concurrency in Practice."
Benjamin/Cummings, 1993, ISBN 0-8053-0088-0.
.LP
Gregory R. Andrews,
.I "Concurrent Programming: Principles and Practice."
Benjamin/Cummings, 1991, ISBN 0-8053-0086-4.
.LP
sr(1), cpp(1), csp2sr(1), m2sr(1)
.SH DIAGNOSTICS
.LP
Some erroneous CCR code will cause errors from cpp.
The line numbers
that cpp complains about generally correspond to the ones in the .ccr
file.
Other than that,
.I ccr2sr
does nearly no error checking.
To flag some errors,
.I ccr2sr
intentionally
generates invalid SR code containing the word "ERROR" followed by an
explanation.
The SR compiler will detect that as an error later.
.LP
Other errors in CCR code are detected by the SR compiler.
The line numbers for these errors
will not correspond to those in the original source file,
but they can be mapped back by looking in
the generated code file.
That file will be rather ugly, but the
cause of the error can be found there.
To give some help in tracing
back errors to the original source file, the generated code file
contains comments of the form /*--X--*/,
where X is a line number in the original source file.
These comments are generated for any construct that the preprocessor
replaces by other text.  If, for example, the SR compiler
reports an error on line 38,
then look at line 38 of the generated SR file.
If on that line you find the comment /*--12--*/,
examine line 12 of the original source file to see the error's cause.
If you see no /*--X--*/ comment on the
line itself, you should look for the first /*--X--*/ comment that
appears above the line and use that number to point you back to the
correct line in the original source file.
