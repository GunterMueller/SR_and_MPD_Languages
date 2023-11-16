.\"  format with troff or nroff using the -ms macro package
.so common.ms
.Nm REL2.1
.TL
Release Notes for Version 2.1 of SR
.AU
Gregg M. Townsend
.AI
Department of Computer Science
The University of Arizona
.sp 1
March 17, 1993
.\" Ch title -- start chapter
.de Ch
.br
.ne 15
.sp 3
.ps +2
.ce 1
\\fB\\$1\\fP
.ps -2
.sp .5
.LP
..
.\" SS title - start subsection
.de SS
.sp
.ne 3.1
\\fI\\$1\\fP
.LP
..
.\" Bu -- start indented, bulleted paragraph
.de Bu
.IP \(bu 3n
..
.\"
.\" for code use Courier-Bold if "psroff", else use standard bold
.ds C \\fB
.if 1i=576u .ds C \\f(CB
.\"
.QP
.B
These release notes accompanied version 2.1 of SR.
.LP
Version 2.1 extends the SR language in minor ways
and adds supplemental tools such as
tracing facilities, new utilities, and an X-Windows interface.
Several bugs and portability problems have been fixed.
.PP
SR is documented in
.I "The SR Programming Language: Concurrency in Practice,"
by Gregory R. Andrews and Ronald A. Olsson
(Benjamin/Cummings, 1993, ISBN 0-8053-0088-0).
Information about concurrent programming in general can be found in
.I "Concurrent Programming: Principles and Practice,"
by Gregory R. Andrews
(Benjamin/Cummings, 1991, ISBN 0-8053-0086-4).
Manual pages for the compiler and other components
are provided with the SR system.
.Ch "New Features"
.LP
This section surveys the differences between Versions 2.0 and 2.1 of SR.
For additional details on how to use the new features,
check the appropriate
.I man
pages.
.SH
Language Changes
.Bu
The call @nap(0)@ now yields control to another process of equal or
higher priority, if there is one.
It used to have no effect.
.Bu
In a @fa@...@downto@...@by@... statement, the step size following the
@by@ is expected to be negative (as documented in the SR book),
not positive (as in Version 2.0).
.Bu
The
.I csp2sr
preprocessor now allows a user @final@ block.
.Bu
Some compiler diagnostics have been improved.
.SH
Library Support
.LP
Two new features, when used in combination, make it possible to create
and use a library of SR procedures.
.Bu
.I sr
now passes to
.I srl
the names of all imported resources or globals,
even if they were compiled separately,
and also accepts and passes along
.B .o ,
.B .a ,
and \fB\-l\fP\fIxxx\fP command arguments.
.Bu
.I sr
and
.I srl
now check the directories specified in an SR_PATH environment variable 
when searching for resources to import or files to load.
.SH
Graphics Interface
.Bu
The new
.I SRWin
library provides an interface to the X Window System and
makes it easy to create simple graphical applications in SR.
A wide range of drawing operations is provided.
The concurrency inherent in SR makes it especially easy
for the programmer to handle window events;
refresh events are handled automatically and require no attention at all.
.SH
New Utilities
.Bu
.I srlatex
is a formatter designed specifically for use with LaTeX.
.Bu
.I srprof
reads an SR_TRACE event log to produce a profile of an SR program.
.SH
Execution Time Changes
.Bu
Messages reporting blocked processes now include the source line number
where each process is blocked.
.Bu
Some sanity checks have been added to better detect the use of
uninitialized capability variables.
.Bu
To prevent lost output, the runtime system now sets O_APPEND mode for
standard output and standard error before creating virtual machines.
.SH
Event Tracing
.Bu
Setting the environment variable SR_TRACE to a file name
outputs to that file a record of invocations and other events.
The file can be processed by the new utility
.I srprof
to produce an execution profile.
.Bu
The previous use of SR_TRACE for debugging the runtime system
is now assumed by a new SR_DEBUG environment variable.
.Bu
The @-T@ option has been removed from
.I srl.
.SH
Performance
.Bu
Array indexing has been speeded up in the case where the array bounds
are known at compile time.
.Bu
The runtime system now blocks when there is nothing to do but wait for input.
MultiSR still polls if more than one processor is enabled.
.Bu
The SR compiler no longer takes an inordinate amount of time to process
programs that use recursively defined capabilities.
.Bu
Deleting a semaphore, a fairly common operation inside the runtime system,
no longer requires a large stack allocation.
.Bu
Some primitive operations are slightly slower as a consequence of the
improvements in diagnostic and traceback information.
.SH
New Platforms
.Bu
In addition to the previously supported systems,
SR Version 2.1 has been tested under
Solaris 2.1 (Sun Sparc) and AIX 3.1 (IBM RS/6000).
.Ch "Bug Fixes"
.LP
Most of the problems reported about SR Version 2.0 have been fixed.
The corrections are as follows:
.Bu
Separate bodies now work for globals as well as resources.
.Bu
The double-subscript form @opname[a,b]@ is now accepted by
@receive@, @P@, and @in@.
.Bu
Calls to procs having @array[N]@ parameters (vs. @array[*]@ parameters)
no longer generate bad C code.
.Bu
The object of a @destroy@ can now be an expression.
.Bu
A compiler malfunction involving the export of records, capabilities,
and optypes has been fixed.
.Bu
A @receive@ of a @ref@ parameter is now handled properly.
.Bu
The arguments of @min@ or @max@ are all converted to a common type
even when there are more than three of them.
.Bu
Problems related to incorrectly freeing memory allocated inside
@in@...@by@ or @in@...@st@ have been corrected.
.Bu
Processes that become unblocked are now properly positioned by priority
in the ready queue.
.Bu
Forwarding an invocation from a co-call no longer causes a segmentation fault.
.Bu
For MultiSR on an SGI Iris, an intermittent ``Bad File Number''
problem has been fixed.
.Bu
The @-A@ option of
.I srl
is no longer ignored.
.Bu
.I srl
no longer deletes the @a.out@ file before rewriting it.
.Ch "Compatibility"
Version 2.1 is almost completely compatible with the previous version 2.0
at the source level.
.Bu
SR source programs require no changes to run under version 2.1
unless they use @downto@ in a quantifier with an explicit @by@ clause.
.Bu
There is no binary compatibility; all programs must be recompiled.
Resources compiled by version 2.0 cannot be linked with version 2.1
resources or libraries.
.Bu
Makefiles and other scripts are affected only by the removal of the
obscure @srl -T@ option and the renaming of the old SR_TRACE
environment variable to SR_DEBUG.
.Ch "Installation Changes"
The installation process is essentially the same as for SR Version 2.0.
.Bu
Minor portability problems have been fixed to allow SR to run
under Sun's Solaris 2.1 operating system.
Problems with the AIX port for the IBM RS/6000 have also been corrected.
.Bu
The
.B sr-mode.el
file is no longer copied into an Emacs directory;
instead, it is installed with the SR library files,
and we suggest putting a symbolic link in the Emacs directory.
A new
.B srlatex.sty
style file for
.I srlatex
is handled similarly.
.Bu
The inclusion of SRWin,
which includes X11 files during its compilation,
may possibly require some Makefile changes.
If X11 is not available, the building of SRWin is disabled by
renaming its subdirectory.
A new MAN3 configuration option is added to handle the SRWin
.I man
page.
.Ch "Implementation Limitations"
.LP
This section lists some limitations of the current implementation.
Some of these could reasonably be considered bugs.
.SH
The Programming Environment
.Bu
Source files are generally expected to reside in the current directory;
explicit paths elsewhere don't always work.
.Bu
Keywords in strings or comments occasionally make
.I srgrind
think it saw
a @pb@ (procedure begin), typesetting a new name in the margin.
.Bu
The
.I SRWin
library does not function well on a Sequent Symmetry.
.SH
Problems Seen at Compilation Time
.Bu
Slices cannot be used with the swap (@:=:@) operator.
.Bu
Builtin procedures having string result parameters
(@read@, @get@, @scanf@, @getarg@, @sprintf@)
cannot accept slice expressions for those parameters.
.Bu
Unions are really implemented as records; no storage is shared.
.Bu
Some older C compilers disallow the expression @f().m@ where @m@
is a member of the struct returned by function @f@.
If C lacks this support, the corresponding expression in SR
will fail to compile.
.Bu
SR uses a multipass compiler.
If it finds an error on one pass, it skips the later passes,
with the consequence that not all errors are reported at once.
Errors in @spec@ sections may be reported once for each import.
.SH
Problems Seen at Execution Time
.Bu
The bounds of an array passed by @ref@ are those of the underlying array,
not those declared for the parameter.
.Bu
Null and noop capabilities cannot be created for resources that export arrays
of operations if any of those arrays has more than three dimensions.
.Bu
Synchronization expressions are not recomputed when their values
are changed by the actions of other processes (e.g. by altering
global variables or sending to operations referenced by a @?@ operator).
.Bu
Destroying a resource is not an atomic operation.
If processes in two
resources each simultaneously try to destroy the other resource, one or
both of the processes can be killed, leaving one or both resources
incompletely destroyed.
.Bu
In most situations, the process with the highest priority is executed.
However, processes that are blocked waiting for messages become
unblocked without regard to priority.
.SH
Problems Involving Distributed Programs
.Bu
If a distributed SR program is found using the shell's search path,
but is not in the current directory, then #srx generates an incorrect
network file path and virtual machines cannot be created.
.Bu
Because of network pipeline delays,
output from different virtual machines may be misordered,
even if it is explicitly synchronized.
.Bu
Error messages can sometimes occur, due to race conditions, during normal
termination of distributed programs.
For example, these can be caused if X exits while Y is sending to it.
.Bu
Virtual machines don't always shut down cleanly if an SR program is run in
the background under the Bourne shell.
This is noticed when running #srv under #sh or #at.
.br
.ne 12
.SH
Undetected Errors
.LP
Not all errors are detected by the SR compiler or runtime system.
These errors can manifest themselves in various ways including
segmentation faults, other runtime errors, or even misleading diagnostics.
Some of the more common such errors are these:
.Bu
Arithmetic overflow is not detected by either the compiler or runtime system.
.Bu
The use of an uninitialized variable is not detected,
nor is the use of a @ptr@ or @ref@ parameter on a different virtual machine.
.Bu
Values of enumeration types are not checked at execution time.
.Bu
Unreachable code is not diagnosed.
.Bu
No warning is given if an invocation is still pending when an operation
ceases to exist.
