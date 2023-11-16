.TH SRM 1 "15 December 1993" "University of Arizona"
.SH NAME
srm \- SR makefile generator
.SH SYNOPSIS
.B srm
[ options ] file.sr ...  [ other files and libraries ]
.SH DESCRIPTION
.LP
\fISrm\fP generates a makefile (for use with \fImake\fP(1)) that builds an
SR program from the given \fIsr\fP source files (\fI.sr\fP files),
other files (\fI.c\fP, \fI.o\fP files), libraries (-l\fIlib\fP), and 
archives (\fI.a\fP files).
.LP
\fISrm\fP produces a makefile that uses \fIsr\fP(1) and \fIsrl\fP(1) to
compile, link, and execute the program described by the files and libraries
listed on the command line.
The generated makefile provides the following services:
.TP 15
.B make compile
compiles without linking
.TP
.B make link
links after a ``make\ compile'' (really the same thing as just ``make'')
.TP
.B make run
executes the program (runtime arguments may be specified with the
\fB\-R\fP option)
.TP
.B make ls
produces a list of the source files (e.g., ``pr\ `make ls`'')
.TP
.B make clean
removes all artifacts of compilation except for the executable
.TP
.B make cleanx
removes all artifacts of compilation including the executable
(additional files may be specified for removal with the \fB\-Z\fP option)
.TP
.B make make
re-makes the makefile as per the original instructions (useful if the
resource import graph changes and the makefile needs to reflect the new
dependencies)
.LP
The default makefile name is
.BR Makefile ,
and thus the command
``make'' executes it.
The makefile name can be changed using the \fB\-f\fP option.
As a safety
feature, \fIsrm\fP refuses to overwrite an existing file that it did
not create.
.LP
The default executable name is
.BR a.out.
This can be changed using the 
.B \-o
option.
.LP
\fISrm\fP attempts to determine the main resource (for linking purposes)
by analyzing the source files and selecting the first resource it
finds that is imported by the least number of resources.
If some other resource is to be the main resource, it should be specified 
with the \fB\-m\fP option.
.LP
\fISrm\fP takes a number of option flags, which are
specified in arguments beginning with a hyphen.
The following options are recognized:
.TP 10
.BI \-f " mfile"
Name the makefile 
.I mfile
instead of
.BR Makefile .
A file name of ``\-'' directs the makefile to standard output.
.TP
.BI \-I " dir"
Use
.IR dir /Interfaces
as the Interfaces directory instead of ./Interfaces.
.TP
.BI \-o " ofile"
Use
.I ofile
for executable output instead of
.BR a.out .
.TP
.BI \-c " compiler"
Use
.I compiler 
instead of the default compiler.
.TP
.BI \-m " res"
Use the specified resource as the main resource.
The default is the first resource found that is imported
by the fewest number of resources.
.TP
.B \-v
Display interesting information while building the makefile.
.TP
.B \-w
Output wide makefile lines instead of limiting to 78 columns.
.TP
.BI \-C " opts"
Include
.I opts
in the list of options passed to the
compiler \fIsr\fP(1).
.TP
.BI \-L " opts"
Include
.I opts
in the list of options passed to the
linker \fIsrl\fP(1).
.TP
.BI \-R " args"
Include
.I args
in the list of arguments passed to the
executable by ``make\ run''.
.TP
.BI \-Z " files"
Include
.I files
in the list of files removed by
``make\ cleanx''.
.SH EXAMPLES
.TP 30
srm *.sr
no options, all .sr files in  current directory
.TP
srm \-m main \-o x a.sr b.sr
the main resource and executable name are specified
.TP
srm \-C "\-q \-O" a.sr b.sr 
flags are passed to \fIsr\fP(1) 
.TP
srm a.sr b.sr c.o \-ll r.c \-lm
objects, libraries and \fIC\fP sources listed
are passed on to \fIsrl\fP(1) to be linked in with the sr program.
.SH ENVIRONMENT
.IP SRMOPTS
The environment variable SRMOPTS may contain
.I srm
options to be processed ahead of explicit options.
.IP SR_PATH
.I Srm
searches for resources and object files
by looking first in the current directory,
then in the Interfaces directory,
and finally in the SR library.
(Items found somewhere other than the current directory are included in
the makefile as
.I srl
arguments but not as dependencies.)
If SR_PATH is set, any directories named there are checked
ahead of the standard search sequence.
Directories in SR_PATH are separated by colons (\fB:\fP).
.SH FILES
.ta 30n
Makefile	generated makefile
.br
file.sr		SR source file
.br
a.out	executable program
.br
Interfaces/component.o	object file
.br
Interfaces/resource.spec	export information
.SH SEE ALSO
sr(1), srl(1), make(1)
.SH DIAGNOSTICS
\fISrm\fP uses the \fIsr\fP compiler to obtain dependency information.
In addition to the syntax errors detected by the compiler,
.I srm
detects missing resource/global
specifications and duplicate spec/body definitions.  Warnings are 
issued if a resource or global has no body or a resource that is not the main
resource is not imported by any other resource. 
.SH CAVEATS
.PP
Omitting \fIsrm's\fP initial
.B s
can have disastrous results.
.PP
\fISrm\fP doesn't escape shell meta-characters in the makefile.
.PP
If the main resource is imported by others, the
.B \-m
option will probably be required to produce a correct makefile.
