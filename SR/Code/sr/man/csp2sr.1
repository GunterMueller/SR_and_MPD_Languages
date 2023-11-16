.TH CSP2SR 1 "22 Dec 2000" "University of Arizona"
.SH NAME
csp2sr \- CSP notation to SR code preprocessor
.SH SYNOPSIS
\fBcsp2sr
[\|\-ti\|]
[\|\-te\|]
[\|\-e\|]
\fRfile.csp
.br
.SH DESCRIPTION
.LP
.I Csp2sr
converts a program written in CSP (Communicating Sequential Processes) notation
into an equivalent one written in the SR language.
The SR program is placed in file.sr,
which may then be compiled and linked using
.IR sr (1).
The
.B \-t
options specify which termination discipline is to be used.
The
.B \-e
(experimental) option
is for testing
.I csp2sr.
.LP
The exact syntax of the CSP notation can be discerned
by examining the CSP programs in the examples directory.
Each CSP-notation construct begins with an underscore;
CSP code is mixed with regular SR code.
Syntactic peculiarities
exist to ease implementation.
.LP
.I Csp2sr
translates a CSP program into an equivalent SR resource;
the SR generated code uses a slight modification of the
centralized clearing house technique
described in Andrews's book.
The generated code is not necessarily fair;
i.e., ports are serviced nondeterministically.
.LP
.I Csp2sr
supports implicit (i.e., automatic) or explicit
termination disciplines.
The choice is made
by the corresponding command line option.
The default is implicit termination.
If a CSP input/output command that appears as a statement
(e.g., _stmt_i but not _guard_i) fails, the
entire program terminates.
A process that attempts to send to or receive from
itself is not detected as an error.
.LP
Following are a few restrictions in
using
.I csp2sr .
CSP processes can have 0, 1, or 2 dimensions.
The subscripts for arrays
of processes must be integers.
A process's spec must appear before its body.
Code for all members of a process family must be the same.
Nested processes are not allowed.
SR operation names are used as constructors.
There is no empty constructor.
Operation names must be unique to the entire program, not
just to a process.
Parameters must be enclosed in `()' even if there
are none.
Operation declarations should not declare variable or result
parameters, and should not include returns clause.
Put no spaces around process or operation names in the CSP constructs.
.LP
The boolean part of a CSP guard can be only a single boolean expression;
so, use `&' to separate parts of a guard (not `;' as in the CSP
paper).
Quantifier variables should not be declared.
Quantifiers can
have 1 or 2 variables.
Variables cannot be declared as part of guards.
The CSP \fB_if\fP and \fB_do\fP constructs do not allow an else part.
The CSP \fB_do\fP construct does not allow exit or next statements.
.LP
A number of syntactic limitations exist to ease the implementation.
Do not use names of SR predefined functions or reserved words
(e.g., free, exit, or skip) as variable names within CSP code.
Do not use identifiers beginning with csp_ within CSP code.
Do not use any other SR synchronization or return/reply within a CSP program.
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
part of a CSP construct (e.g., _guard); instead use the [a][b]
form.
.LP
The predefined SR procedure nap,
which puts the currently executing process to sleep for a while
and performs a context switch to another process,
is useful in CSP programs
to alter the interleaving of process execution.
For example, it can be used
with the random number functions to obtain different
interleavings to test whether a critical section algorithm works.
See the programs in the examples directory for examples.
.SH QUICK REFERENCE
.nf
.ta 4n 40n
_program(\fIname\fP)
_program_end
.sp .6
_specs	follow with process specs and port
_specs_end
_dump_pidx	for implementation debugging
.sp .6
_process_spec(\fIname\fP)	\fIname\fP is a CSP process
_process_spec1(\fIname, l1, u1\fP)	one-dim CSP process array \fIname[l1:u1]\fP
_process_spec2(\fIname, l1, u1, l2, u2\fP)	two-dim CSP array \fIname[l1:u1,l2:u2]\fP
.sp .6
_port(\fIpname,oname,ospec\fP)	declares a CSP port
	\fIpname\fP, actually unused, is process that inputs from port
	\fIoname\fP is name of port
	\fIospec\fP is parameters
  	get one port for each element of an array
.sp .6
_process_body(\fIname\fP)	body of process \fIname\fP
_process_body1(\fIname, v1\fP)	one-dim body; \fIv1\fP is process id
_process_body2(\fIname, v1, v2\fP)	two-dim body; \fIv1,v2\fP is process id
_process_end
.sp .6
_stmt_i(\fIpname,ouse,args\fP)
	input statement, i.e., \fIpname?ouse(args)\fP
	\fIpname\fP is source; \fIouse\fP is port name; \fIargs\fP are formals
_stmt_iq1(\fIv1,l1,u1, pname,ouse,args\fP)
	one-dim quantified input statement,
	i.e., \fI(v1 := l1 to u1) pname?ouse(args)\fP
_stmt_iq2(\fIv1,l1,u1, v2,l2,u2, pname,ouse,args\fP)
	two-dim quantified input statement
	i.e., \fI(v1 := l1 to u1, v2 := l2 to u2) pname?ouse(args)\fP
.sp .6
_stmt_o(\fIpname,ouse,args\fP)
	output statement, i.e., \fIpname!ouse(args)\fP
	\fIpname\fP is destination; \fIouse\fP is port name; \fIargs\fP are actuals
_stmt_oq1(\fIv1,l1,u1, pname,ouse,args\fP)
	one-dim quantified output statement
	i.e., \fI(v1 := l1 to u1) pname!ouse(args)\fP
_stmt_oq2(\fIv1,l1,u1, v2,l2,u2, pname,ouse,args\fP)
	two-dim quantified output statement
	i.e., \fI(v1 := l1 to u1, v2 := l2 to u2) pname!ouse(args)\fP
.sp .6
_if		CSP \fBif\fP, for using I/O in guards
	don't separate guards with []
_fi
.sp .6
_do		CSP \fBdo\fP, for using I/O in guards
	don't separate guards with []
_od
.sp .6
_guard(\fIexpr\fP)	plain boolean guard of \fB_if\fP or \fB_do\fP
_guard_q1(\fIv1,l1,u1, expr\fP)	one-dim quantified guard
	i.e., \fI(v1 := l1 to u1) expr\fP
_guard_q2(\fIv1,l1,u1, v2,l2,u2, expr\fP)	two-dim quantified guard
	i.e., \fI(v1 := l1 to u1, v2 := l2 to u2) expr\fP
.sp .6
_guard_i(\fIexpr,pname,ouse,args\fP)
	input command as a guard of \fB_if\fP or \fB_do\fP; \fIexpr\fP is boolean
	i.e., \fIexpr; pname?ouse(args)\fP
_guard_iq1(\fIv1,l1,u1, expr,pname,ouse,args\fP)
	one-dim quantified input command as a guard of \fB_if\fP or \fB_do\fP
	i.e., \fI(v1 := l1 to u1) expr; pname?ouse(args)\fP
_guard_iq2(\fIv1,l1,u1, v2,l2,u2, expr,pname,ouse,args\fP)
	two-dim quantified input command as a guard of \fB_if\fP or \fB_do\fP
	i.e., \fI(v1 := l1 to u1, v2 := l2 to u2) expr; pname?ouse(args)\fP
.sp .6
_guard_o(\fIexpr,pname,ouse,args\fP)
	output command as a guard of \fB_if\fP or \fB_do\fP; \fIexpr\fP is boolean
	i.e., \fIexpr; pname!ouse(args)\fP
_guard_oq1(\fIv1,l1,u1, expr,pname,ouse,args\fP)
	one-dim quantified output command as a guard of \fB_if\fP or \fB_do\fP
	i.e., \fI(v1 := l1 to u1) expr; pname!ouse(args)\fP
_guard_oq2(\fIv1,l1,u1, v2,l2,u2, expr,pname,ouse,args\fP)
	two-dim quantified output command as a guard of \fB_if\fP or \fB_do\fP
	i.e., \fI(v1 := l1 to u1, v2 := l2 to u2) expr; pname!ouse(args)\fP
.fi
.SH FILES
.ta 20n
file.csp	CSP notation source file
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
sr(1), cpp(1), ccr2sr(1), m2sr(1)
.SH DIAGNOSTICS
.LP
Some erroneous CSP code will cause errors from cpp.
The line numbers
that cpp complains about generally correspond to the ones in the .csp
file.
Other than that,
.I csp2sr
does nearly no error checking.
To flag some errors,
.I csp2sr
intentionally
generates invalid SR code containing the word "ERROR" followed by an
explanation.
The SR compiler will detect that as an error later.
.LP
Other errors in CSP code are detected by the SR compiler.
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
