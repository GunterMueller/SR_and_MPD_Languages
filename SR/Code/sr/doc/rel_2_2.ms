.\"  format with troff or nroff using the -ms macro package
.so common.ms
.Nm REL2.2
.TL
Release Notes for Version 2.2 of SR
.AU
Gregg M. Townsend
.AI
Department of Computer Science
The University of Arizona
.sp 1
August 30, 1993
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
.LP
Version 2.2 of SR introduces dynamic operations and
allows the use of operation capabilities in
@in@, @receive@, and @P@ statements.
Version 2.2 also includes a minor enhancement of @srm@
to make it usable with the X-Windows interface
and provides MultiSR for Sun systems running Solaris version 2.
Version 2.1 of SR modified @nap()@ and added
tracing facilities, new utilities, and an X-Windows interface.
.PP
The base version of SR, version 2.0, is documented in
.I "The SR Programming Language: Concurrency in Practice,"
by Gregory R. Andrews and Ronald A. Olsson
(Benjamin/Cummings, 1993, ISBN 0-8053-0088-0).
Information about concurrent programming in general can be found in
.I "Concurrent Programming: Principles and Practice,"
by Gregory R. Andrews
(Benjamin/Cummings, 1991, ISBN 0-8053-0086-4).
These books can be ordered through your local bookstore
or by telephoning 1-800-950-2665 or 1-415-594-4400.
Manual pages for the compiler and other components
are provided with the SR system.
.Ch "New Language Features"
This section describes changes made to the SR language
after the first printing of the SR book.
Most of these are new with Version 2.2.
.SH
Input from Capabilities
.LP
In earlier versions of SR, an operation capability could be used as a
destination for an invocation but not as a source.
This asymmetry is removed, to a reasonable extent, in version 2.2.
The remaining limitations ensure that an input statement requires at most
a single exchange of messages with a different virtual machine.
.PP
An operation capability can now replace an operation name in an input statement,
without regard to how the capability was created, subject to meeting
.I either
of the following two restrictions:
.sp .4
	\(bu  the input statement has just one arm and no @by@ or @st@ 
(but it may have an @else@), or
.sp .2
	\(bu  the capability references an operation in the current resource.
.sp .2
.LP
Capabilities are also allowed in @P@ and @receive@ statements;
these are shorthand forms of input statements
meeting the first restriction.
.PP
As an example, a bag of tasks can be encapsulated in a resource by exporting
an operation name that represents the bag.
Processes in other resources can both contribute and withdraw tasks 
by using a capability for the exported operation.
.PP
The question mark operator (@?@) can be used to query the number of pending
invocations available through a capability.
.PP
A @noop@ capability is treated as an operation that is never invoked.
Hence, input from a @noop@ capability never succeeds.
Querying a @noop@ capability with the @?@ operator always returns 0.
Attempting to query or input from a @null@ capability\(emor a capability
for an operation that no longer exists\(emproduces a runtime error.
.PP
A capability used with @P@, @receive@, or @?@ can be any arbitrary expression.
For syntactic reasons, a capability used in an @in@ statement
is limited to the form of an identifier, possibly qualified and/or
subscripted.
.SH
Dynamic Operations
.LP
It is now possible to create new operations on demand.
Any of the following expressions creates one instance of a new,
anonymous operation and returns a capability for that operation:
.nf
.sp .6
	@new@ @(#optype_id)@
	@new@ @(op #operation_specification)@
	@new@ @(op #operation_specification #operation_restriction)@
	@new@ @(sem)@
.sp .6
.fi
.R
The operation can be invoked or serviced until the
resource or operation is destroyed.
.PP
The operation referenced by capability #ocap can be destroyed by executing
.sp .6
	@destroy@ #ocap
.sp .6
Only dynamically created operations can be destroyed.
Note that in this case @destroy@ and not @free@ is the opposite of @new@;
this is consistent with the destruction of resources using capabilities.
A subsequent invocation, query, input, or destroy of a destroyed operation
produces a runtime error.
.PP
Arrays of operations can be created using any of the forms
.nf
.sp .6
	@new@ @([\fIsubscripts\fP] \fIoptype_id\fP)@
	@new@ @([\fIsubscripts\fP] op \fIoperation_specification\fP)@
	@new@ @([\fIsubscripts\fP] op \fIoperation_specification operation_restriction\fP)@
	@new@ @([\fIsubscripts\fP] sem)@
.sp .6
.fi
.R
The array of operations referenced by #ocap_array can be destroyed
by executing
.sp .6
	@destroy@ #ocap_array
.sp .6
Elements of an array can also be destroyed individually;
destruction of any individual element renders wholesale destruction
of the array illegal,
because that would try to destroy the individual element a second time.
.SH
Process Handoff
.LP
The call @nap(0)@ now yields control to another process of equal or
higher priority, if there is one.
This change was made in version 2.1;
prior to that, @nap(0)@ had no effect.
.Ch "MultiSR for Solaris"
.LP
MultiSR (true multiprocessing) is now available for multiprocessor
Sun systems running Solaris version 2.
MultiSR is also available for SGI Iris and Sequent Symmetry systems.
The installation instructions explain how to enable MultiSR.
.PP
On Suns running Solaris 2.2, distributed programs (those using
virtual machines) do not work if SR is configured for MultiSR.
Sun's software patch 101014-04, a socket library update, allows a distributed
program to run provided that SR_PARALLEL is not set when the program executes.
We anticipate that these problems will be fixed by a planned upgrade
of the socket library in Solaris 2.3.
.Ch "Other Changes"
.LP
@Srm@ now understands and handles SR programs that link files or
import resources from the SR library or from directories
specified in @SR_PATH@.
The immediate effect of this is to make @srm@ usable by programs
that use @SRWin@, the X-Windows interface.
.PP
Two new procedures, @WinSetForegroundByPixel@ and @WinSetBackgroundByPixel@,
have been added to SRWin.
These facilitate the use of nonstandard drawing operations
for sophisticated applications.
.PP
A technical report describing SRWin is now included in PostScript form
as @srwin.ps@ in the @ps@ subdirectory.
The runtime system interface document has been brought up to date.
.PP
Some minor bugs in the runtime system have been fixed.
.Ch "Compatibility"
Version 2.2 is upward compatible with the previous version 2.1
at the source level.
There is no binary compatibility, however; all programs must be recompiled.
Resources compiled by earlier versions cannot be linked with version 2.2
resources or libraries.
.PP
The installation process is essentially unchanged from version 2.1.
Some minor portability problems have been fixed;
in particular, SR now runs under Solaris version 2 using Sun's
unbundled C compiler as an alternative to GCC.
.Ch "Implementation Limitations"
.LP
This section lists some limitations of the current implementation.
Some of these could reasonably be considered bugs.
.LP
This section is unchanged from version 2.1.
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
