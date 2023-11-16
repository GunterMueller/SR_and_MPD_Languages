.TH M2SR 1 "21 March 1995" "University of Arizona"
.SH NAME
m2sr \- monitor notation to SR code preprocessor
.SH SYNOPSIS
\fBm2sr
[\|\-sc\|]
[\|\-sw\|]
[\|\-su\|]
[\|\-sx\|]
[\|\-e\|]
\fRfile.m
.br
.SH DESCRIPTION
.LP
.I M2sr
converts a program written in monitor notation
into an equivalent one written in the SR language.
The SR program is placed in file.sr,
which may then be compiled and linked using
.IR sr (1).
The
.B \-s
options specify which monitor signaling discipline is to be used.
The
.B \-e
(experimental) option
is for testing
.I m2sr.
.LP
The exact syntax of the monitor notation can be discerned
by examining the monitor programs in the examples directory.
Each monitor-notation construct begins with an underscore;
monitor code is mixed with regular SR code.
Syntactic peculiarities
exist to ease implementation.
.LP
.I M2sr
translates a monitor into an equivalent SR global;
the SR generated code uses a slight modification of the technique
described in Joe Herman's thesis.
Typically, the
rest of the user's code will be a single SR resource program.
Monitor
operations must be declared in the monitor's specification part.
User code 
needs to import the monitor global;
it can invoke monitor operations via call statements
(in which the operation name is qualified by the name of the monitor,
as per usual SR rules).
.LP
.I M2sr
supports
SC (signal and continue),
SX (signal and exit),
SW (signal and wait),
and
SU (signal and urgent wait) signaling disciplines.
The choice is made
by the corresponding command line option.
The default is SC.
.LP
The operations on condition variables are the standard ones: wait(cv),
signal(cv), pri_wait(cv,rank), empty(cv), minrank(cv), and
signal_all(cv). (signal_all only makes sense in the SC signaling
discipline; it is not allowed in the others).
In addition, print(cv),
intended for primitive debugging, outputs the number of processes
waiting on the condition variable and their ranks.
Arrays of one or
two dimensions of condition variables can be declared, although the
syntax is baroque; e.g., condvar1(scan,0:1) declares scan to be a one
dimensional array with indices 0 and 1, and condvar2(foo,3,5:9)
declares foo to be a two dimensional array with indices 1 through 3 in
the first dimension and 5 through 9 in the second dimension.
.LP
A number of syntactic limitations exist to ease the implementation.
Do not use names of SR predefined functions or reserved words
(e.g., free, exit, or skip) as variable names within monitors.
Do not use identifiers beginning with m_ within monitor code.
Do not use any other SR synchronization or return/reply within a monitor program.
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
Do not use the [a,b] form of subscripting a two-dimensional array
within a monitor construct (e.g., _wait); instead use the [a][b]
form.
.LP
The predefined SR procedure nap,
which puts the currently executing process to sleep for a while
and performs a context switch to another process,
is useful in monitor programs
to alter the interleaving of process execution.
For example, it can be used
with the random number functions to obtain different
interleavings to test whether a critical section algorithm works.
See the programs in the examples directory for examples.
.SH QUICK REFERENCE
.nf
.ta 20n
_monitor(\fIname\fP)	(after which, declare each monitor proc as an op)
_body(\fIname\fP)
_monitor_end
.sp .6
_condvar(\fIx\fP)	declare condition variable \fIx\fP
_condvar1(\fIx,s\fP)	declare one-dim array condition variable \fIx[s]\fP
_condvar2(\fIx,s,t\fP)	declare two-dim array condition variable \fIx[s,t]\fP
.sp .6
_proc(\fIx\fP)	monitor procedure with name and parameters \fIx\fP
_proc_end
.sp .6
_wait(\fIcv\fP)
_empty(\fIcv\fP)
_pri_wait(\fIcv,r\fP)	prioritized wait by rank \fIr\fP on condition variable \fIcv\fP
_minrank(\fIcv\fP)
_print(\fIcv\fP)	(for debugging; not a regular monitor primitive)
.sp .6
_signal(\fIcv\fP)
_signal_all(\fIcv\fP)	(only for SC signaling discipline)
.fi
.SH FILES
.ta 20n
file.m	monitor notation source file
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
sr(1), cpp(1), ccr2sr(1), csp2sr(1)
.SH DIAGNOSTICS
.LP
Some erroneous monitor code will cause errors from cpp.
The line numbers
that cpp complains about generally correspond to the ones in the .m
file.
Other than that,
.I m2sr
does nearly no error checking.
To flag some errors,
.I m2sr
intentionally
generates invalid SR code containing the word "ERROR" followed by an
explanation.
The SR compiler will detect that as an error later.
.LP
Other errors in monitor code are detected by the SR compiler.
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
.LP
In some cases,
the generated code for monitor code using the SX signaling discipline
causes complaints from the SR compiler, or later from the C
compiler, about unreachable code.
These warnings can be ignored.
