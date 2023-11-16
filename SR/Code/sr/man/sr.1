.TH SR 1 "7 October 1994" "University of Arizona"
.SH NAME
sr \- SR compiler
.SH SYNOPSIS
\fBsr [\|\-sbcqwegvOTMCFP\|]
[\|\-I \fRdir\fB\|]
[\|\-o \fRfile\fB\|]
[\|\-d \fRflags\fB\|]
\fRfile.sr ... otherfiles...
.br
.SH DESCRIPTION
.LP
.I Sr
compiles programs written in the SR language.
In the usual case, one or more
.B .sr
files are compiled to produce
.B .o
files, and then the SR linker,
.IR srl (1),
is called to produce an executable file
.BR a.out .
The last encountered resource is taken as the main resource.
.LP
Compilation and linking takes place in the context of an
.I Interfaces
directory used for passing specification and object files between
.I sr
and
.I srl
runs.
Files are created in the Interfaces directory
as each resource or global is compiled.
The Interfaces directory is needed for compilation and linking,
but not execution, and can be deleted after an executable has been produced.
.LP
Each source file is read twice.
The first pass copies component specifications into the Interfaces directory;
the second pass generates object code.  
If errors are detected in the first pass, the second pass is omitted.
.LP
Files not ending in \fB.sr\fP
(such as \fB.o\fP, \fB.a\fP, and \fP\-l\fP\fIxx\fP files)
are not processed by
.I sr
but are passed along to
.IR srl .
.SH OPTIONS
.LP
Any of the options
.BR s , 
.BR b , 
.BR c , 
.BR M , 
or 
.BR C
inhibits linking.
.TP
.BI \-I " dir"
Use
.IR dir /Interfaces
as the Interfaces directory instead of ./Interfaces.
.TP
.BI \-o " file"
Use
.I file
for the executable instead of
.BR a.out .
.TP
.B \-s
Create specification files only (first pass only).
.TP
.B \-b
Compile bodies only (second pass only), leaving specification files unaltered.
.TP
.B \-c
Compile specs and bodies, but do not link
.B .o
files to create an executable file.
.TP
.B \-q
Suppress the echoing of source file names as they are compiled.
.TP
.B \-w
Suppress warning messages.
.TP
.B \-O
Omit some runtime error checks and
invoke the C optimizer to improve the generated code.
.TP
.B \-T
Omit the ``timeslicing'' code that allows context switching at the top
of each loop.
.TP
.B \-M
Generate dependency information for use by
.IR srm (1),
then exit without compiling anything.
.LP
Additional options are useful mainly when debugging the compiler:
.TP
.B \-C
Stop after generating
.B .c
files, leaving them undeleted.
.TP
.B \-F
Inhibit constant folding.
This can break programs by rendering certain expressions non-constant.
.TP
.B \-P
Inhibit normal optimizations in the SR compiler.
.TP
.BI \-d " flags"
Write debugging output selected by
.I flags
on standard output.
.TP
.B \-e
Use experimental versions of
.I srl
and
.B .h
files,
and pass
.B \-e
to
.I srl.
.TP
.B \-g
Compile for debugging with
.IR dbx (1),
don't delete
.B .c
files, and pass
.B \-g
to
.I srl.
.TP
.B \-v
Announce version number,
trace other programs invoked by
.IR sr ,
and pass
.B \-v
to
.IR srl .
.SH ENVIRONMENT
.IP SR_PATH
When compiling an
.B import
statement,
.I sr
searches for a corresponding
.B .spec
file by looking first in the current directory,
then in the Interfaces directory,
and finally in the SR library.
If SR_PATH is set, any directories named there are checked
ahead of the standard search sequence.
Directories in SR_PATH are separated by colons (\fB:\fP).
.br
.ne 8
.SH "EXECUTION ENVIRONMENT"
.LP
These environment variables can be set at execution time to affect
the behavior of an SR program:
.IP SR_PARALLEL
Controls the number of processes that can run with true concurrency
(as opposed to simulated concurrency) under MultiSR.
MultiSR is a configuration option available on
the SGI Iris, Sun Sparc running Solaris 2.x, and Sequent Symmetry.  
The default value is 1 (no true concurrency).
It makes little sense to set SR_PARALLEL greater than the number
of available processors.
.IP SR_SPIN_COUNT
In MultiSR, the number of times an idle processor will check for a task
before relinquishing the CPU to the operating system.
Larger values increase the opportunity for processes to quickly resynchronize
at the expense of greater CPU usage.
The default is 35.
.IP SR_NAP_INTERVAL
In MultiSR, the number of milliseconds an idle processor will nap
after trying unsuccessfully to find something to run.
The default is 10.
.IP SR_TRACE
If not null, SR_TRACE specifies the name of a file to receive a trace
of messages, invocations, and other language-level events.
Two special names, 
.I stdout
and
.I stderr,
direct the trace output to standard output and standard
error output respectively.
The default is no tracing.
Traces from remote virtual machines appear only if the trace is
directed to
.I stdout
or 
.I stderr.
The
.IR srprof (1)
program can be used to summarize the trace output.
.IP SR_DEBUG
A hexadecimal number specifying a bit mask that enables debugging 
information of various internal runtime events on standard error 
output.  Each bit selects a different message category, and the 
higher order bits (FFFFFF00) produce the most voluminous output.  
Details are contained in the source code of the runtime system.  
The default is no debugging.
In contrast with SR_TRACE, the output from SR_DEBUG records
internal events of interest to maintainers of the runtime system.
.IP SRXPATH
Path to the executable of
.IR srx ,
which serves as the central controller of a distributed SR program.
The default is configured when the SR system is built.
.IP SRMAP
File to read for the network configuration information used to generate
a host-independent ``network path'' of the executable
for executing part of an SR program on a remote host.
The default is configured when the SR system is built.
.SH FILES
.ta 27n
file.sr	SR source file
.br
a.out	executable program
.br
Interfaces/component.c	C language intermediate file
.br
Interfaces/component.spec	specification file
.br
Interfaces/global.impl	implementation characteristics of global
.br
Interfaces/component.o	object file
.SH SEE ALSO
Gregory R. Andrews and Ronald A. Olsson,
.I "The SR Programming Language:  Concurrency in Practice."
Benjamin/Cummings, 1993, ISBN 0-8053-0088-0.
.LP
Gregory R. Andrews,
.I "Concurrent Programming: Principles and Practice."
Benjamin/Cummings, 1991, ISBN 0-8053-0086-4.
.LP
Ronald A. Olsson, Gregory R. Andrews, Michael H. Coffin, and Gregg M. Townsend,
.I "SR: A Language for Parallel and Distributed Programming."
TR 92-09, Dept. of Computer Science, The University of Arizona, 1992.
Included in the SR distribution.
.LP
Gregory R. Andrews, Ronald A. Olsson, et al.,
.I "An Overview of the SR Language and Implementation."
ACM Trans. on Prog. Lang. and Systems 10, 1 (January, 1988), 51-86.
.LP
srl(1), srm(1), srprof(1), cc(1), dbx(1), srmap(5), srtrace(5)
.SH DIAGNOSTICS
.LP
Messages diagnosing erroneous programs are intended to be self-explanatory.
.LP
Messages indicating ``compiler malfunction'',
or any error messages generated by the C compiler,
indicate bugs in the SR compiler
that should be reported to its implementors.
