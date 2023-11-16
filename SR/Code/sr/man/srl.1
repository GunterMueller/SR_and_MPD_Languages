.TH SRL 1 "19 February 1993" "University of Arizona"
.SH NAME
srl \- SR linker
.SH SYNOPSIS
.HP
\fBsrl [ \-eglvwA ] [ \-I \fRdir\fB ] [ \-o \fRfile\fB ] [ \-r \fRfile\fB ]
[ \-CLNOPQRVS \fRsize\fB ] \fRcomponent ...
.SH DESCRIPTION
.LP
.I Srl
produces an executable file,
.B a.out
by default,
by linking together the components of an SR program.
The command line lists one or more resource and global names
and may also list additional 
`\fIfile\fP.o', `\fIfile\fP.a', or `\-l\fIxxx\fP'
arguments to link code from other languages.
These additional arguments are passed to
.IR ld (1).
.LP
The last resource named becomes the main resource;
it should have no parameters.
Abstract resources (those having no bodies) should not be named,
but all globals must be listed.
.LP
.I Srl
accepts the following general options:
.TP
.BI \-I " dir"
Use
.IR dir /Interfaces
as the Interfaces directory instead of ./Interfaces.
.TP
.BI \-o " file"
Use
.I file 
for executable output instead of
.BR a.out .
.TP
.B \-w
Suppress warnings about out-of-date files.
.TP
.B \-l
List runtime limits, then exit immediately.
.LP
The following options set values controlling the subsequent execution
of the SR program.
For most maxima, the default value is 1\^000\^000.
.TP
.B \-A
Enable asynchronous output.
When set, output calls do not block the entire program and are not atomic.
Instead, they can affect the flow of control by performing context switches.
.TP
.BI \-C " n"
Set the maximum number of active \fBco\fP statements.
.TP
.BI \-L " n"
Set the maximum number of loop iterations between context switches.
A value of zero is taken as infinite.
.TP
.BI \-N " n"
Set the maximum number of \fBin\fP operation classes.
.TP
.BI \-O " n"
Set the maximum number of active operations.
.TP
.BI \-P " n"
Set the maximum number of processes.
.TP
.BI \-Q " n"
Set the maximum number of pending remote requests.
.TP
.BI \-R " n"
Set the maximum number of active resources.
.TP
.BI \-V " n"
Set the maximum number of number of semaphores.
.TP
.BI \-S " size"
Set the size of a process stack.
.LP
Additional options are useful mainly when debugging the SR system:
.TP
.B \-e
Use the experimental runtime system.
.TP
.BI \-r " file"
Use the runtime system from
.I file.
.TP
.B \-g
Link to allow debugging with
.IR dbx (1).
This is useful mostly for debugging the runtime system and is not
particularly helpful towards understanding errant SR programs.
.TP
.B \-v
Announce other programs invoked by
.IR srl .
.SH ENVIRONMENT
.IP SR_PATH
.I Srl
searches for object files by looking first in the current directory,
then in the Interfaces directory,
and finally in the SR library.
If SR_PATH is set, any directories named there are checked
ahead of the standard search sequence.
Directories in SR_PATH are separated by colons (\fB:\fP).
.br
.ne 8
.SH FILES
.ta 27n
.br
Interfaces/component.o	object file
.br
srlib.a	runtime library
.br
a.out	executable program
.br
Interfaces/component.spec	specification file
.br
Interfaces/_ofile.c	configuration information
.br
Interfaces/_ofile.o	configuration object module
.SH "SEE ALSO"
.LP
sr(1), ld(1), dbx(1)
.SH DIAGNOSTICS
.LP
.I Srl
does not actually do the linking itself;
it is merely a front-end for the UNIX linker
.IR ld (1).
Messages not beginning with ``srl'' can be attributed to
.I ld.
.LP
Messages about undefined externals usually stem from the omission of a
resource or global name from the
.I srl
call.
.LP
.I Srl
verifies that object files of resource and globals
are newer than the corresponding source files.
