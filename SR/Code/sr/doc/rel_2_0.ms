.\"  format with troff or nroff using the -ms macro package
.so common.ms
.Nm REL2.0
.TL
Release Notes for Version 2.0 of SR
.AU
Gregg M. Townsend
Gregory R. Andrews
Ronald A. Olsson
.AI
Department of Computer Science
The University of Arizona
.sp 1
July 23, 1992
.\" Ch title -- start chapter
.de Ch
.bp
.ps +2
.ce
\\fB\\$1\\fP
.ps -2
.sp 2
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
These release notes accompanied version 2.0 of SR and described
the extensive changes relative to version 1.
The version 2.0 implementation has since been superseded,
but the description of differences remains essentially correct.
.sp 3
.LP
Version 2 of SR extends the SR programming language with several new features.
These include shared global variables and operations,
circular imports, real numbers and math functions, formatted I/O,
a more general syntax with fewer special cases,
a more robust system with better error checking,
true concurrency on Sequent and Iris systems,
and many additional enhancements.
.PP
Several new platforms are supported by SR Version 2.
New context switch routines were written for the
SPARC (Sun 4) and HP PA-RISC architectures.
Other tested platforms include the
SGI Iris, Sequent Symmetry, Sun 3, HP 9000/300, NeXT, and Digital's MIPS line.
Code is also included for the
Data General AViiON, IBM RS/6000, DEC VAX,
Encore Multimax, Apollo DN, and others.
.PP
This is in many respects a new implementation.
The compiler has been completely rewritten from scratch.
There have been significant and pervasive changes to the runtime system.
However, the overall framework and organization remains much the same.
.PP
Version 2 of the SR Language is documented in the book
.I "The SR Programming Language: Concurrency in Practice"
by Gregory Andrews and Ronald Olsson
(Benjamin/Cummings, 1993, ISBN 0-8053-0088-0).
This document describes the differences between versions 1 and 2 of SR.
.PP
This document is organized in five major sections.
The first describes the new language features of version 2.
The second describes MultiSR, the multiprocessing implementation of SR.
The third describes a new set of preprocessors for SR.
The fourth notes language and implementation changes that can
affect existing programs developed under version 1.
The final section lists some of the limitations of the version 2
implementation.
.Ch "New Language Features"
.LP
For the most part, version 2 is an upward-compatible
extension of version 1.
This section summarizes the extensions and modifications.
Mechanisms are described in approximately the same
order as in the version 1.1 language report.
Syntactic forms are illustrated as in Appendix 1 of that report.
.SH
System Components
.SS "Globals"
Global components are extended so that they are essentially
single, unparameterized instances of resources.
They are used to declare types, constants,
variables, and operations and that are shared by resources.
They are now named along with resources as #srl arguments.
.PP
As with resources, globals have specs and optional bodies.
All names in the spec are exported; those in the body are private.
The spec and body can be separately compiled or combined.
However, if there is a body, it #must be compiled before the global
is imported by a resource or another global.
This permits the optimization of calls to procedures in the global.
.DS
spec        |1@global@ identifier
        /1    [@import@ clauses]
        /1    declarations
        /1@body@ identifier [@separate@]
.DE
.DS
body        |1@body@ identifier
        /1    [@import@ clauses]
        /1    declarations, statements, procs, and processes
        /1    [@final@ block @end@]
        /1@end@ [identifier]
.DE
A spec without a body replaces @body@ by @end@ and omits the keyword @separate@.
When the spec and body are combined, the last line of the spec is omitted.
The items in the spec and body can appear in any order; the above
order is only what a programmer would commonly use.
.PP
There is one instance of a global in each virtual
machine (address space) that needs it.
This is determined by the appearance of import clauses
in resources that are created on a virtual machine (see below).
A global is automatically created by execution of the first import clause
that references it.
This can trigger the creation of additional globals imported by the first one,
and so on.
.PP
When a global is created, statements are executed in the
order in which they appear.
Processes are created as their declarations are encountered.
If @reply@ is executed during initialization, further initialization
proceeds in parallel (conceptually) with initialization of
the resource that triggered initialization of the global.
.PP
If present, the final component in a global is executed when the address
space is destroyed.
On the main virtual machine this includes the implicit destruction at
program termination.
Execution of final code permits possible cleanup, e.g. of libraries
(and is analogous to final components in resources).
If A imports B,
then A is finalized before B in case the final code in A imports items in B.
.PP
Items exported from a global component are referenced directly
by name because they are in the scope of every resource that imports the global.
Access to shared variables is unprotected; the programmer must use
semaphores or other mechanisms to ensure mutual exclusion if that
is required.
.PP
An operation exported from a global can be invoked directly by any
resource that imports the global and can be invoked indirectly by
any resource that acquires a capability for the operation.
Such an operation can be implemented:
.RS
.Bu
by a @proc@ within the body of the global,
.Bu
by input or receive statements within the body of the global #or
within the bodies of resources that import the global.
.RE
.PP
However, global operations can only appear in input statements
by themselves or in combination with operations declared
in the same global component.
Operations declared in different globals cannot be serviced
by the same input statement.
Also, global operations cannot be serviced by input statements
that also service resource or local operations.
.PP
With the above extensions, global components support a variety
of useful functionality.
This includes the following:
.RS
.Bu
Collections of shared constants and types, as in version 1.
In this case, a global would have a spec but no body.
.Bu
Libraries, with one instance per virtual machine.
In this case, the spec would declare library operations
and the body would contain the implementation of the operations.
For libraries implemented entirely by C routines, the spec would contain an
external declaration for each operation and the body would be omitted.
Of course, hybrid combinations are also possible.
.Bu
Shared variables, e.g. for parallel programs.
These variables would be declared in the spec and initialized,
if necessary, by code in the body.
.Bu
Shared communication channels.
These would be operations declared in the spec and probably
serviced by input or receive statements in the resources that
import the global.
This supports a Linda-like functionality and can be used to
implement bags of tasks shared by worker processes in different resources
(on the same virtual machine).
.RE
.LP
.SS "Resources"
The body of a resource can contain @import@ clauses to gain
access to globals.
.PP
Statements can now appear at the outer level; i.e., they
can be body components.
As with globals, these are executed when the resource is created,
in the order in which they are listed in the body.
Processes are also created in the order in which they are listed.
Resource initialization code can contain return and reply
statements, with the expected semantics.
(As in version 1, initialization code is executed by an implicit process.)
.PP
With the above change, resources no longer have initialization
enclosed by @initial ... end@.
.SS "Blocks"
A block can be enclosed by @begin ... end@.
This kind of block can appear any place a statement can appear;
it would be used to limit the scope of local identifiers, for example
in resource initialization code.
The arms of structured statements (@if@, @do@, etc.) can still contain
local declarations; these do not have to be surrounded by @begin ... end@.
.SS "Resource Import Clauses"
Two resource specs can import each other.
This applies only to resources and not to globals.
.PP
The import of a resource is processed when first encountered.
Subsequent imports of the same resource, including
recursive imports, are ignored.
When two resources import each other,
the ``subsequent import'' rule prevents infinite recursion.
However, declarations preceding the mutual import have already been processed
when the rule takes effect so they are available to both resources.
For example, the following is legal:
.DS
@resource A@
@  const x := 10@
@  import B@
@  op foo(c : cap B)@
@end@

@resource B@
@  import A@
@  const v := 2*A.x@
@end@
.DE
However, the program would be illegal if the declaration of @x@ appeared
after @import B@ or if resource @A@ referenced constant @v@ after
importing @B@.
.SS "Program Execution"
Program execution begins, as in version 1, with the creation of one
instance of the main resource.
The statements of that resource are then executed in sequence;
there is no longer an @initial@ block.
Operation, variable, and other declarations perform any necessary
initialization when executed.
Each global is created on encountering its first @import@ statement.
.PP
Programs terminate when @stop@ is executed, all processes have terminated,
or deadlock has occurred.
Deadlock is now detected for distributed programs
as well as those confined to a single virtual machine.
.SH
Types and Declarations
.SS "General Type Declarations"
Type declarations can give new names to basic and user-defined types.
For example, the following are now allowed:
.DS
@type myint = int@
@type array = [10] int@
@type clist = [*] char@
.DE
.SS "Reals"
A @real@ data type has been added, together with associated
operators and decimal and exponential literals.
The underlying C type is @double@.
In expressions, integers are implicitly cast to reals when necessary,
but not the converse;  mixed mode expressions are allowed,
but real values cannot be assigned to integer variables.
.PP
Several new math functions have been added, as described later.
The operations @low(real)@ and @high(real)@ produce the smallest and largest
representable positive real numbers.
.SS "Unions"
A union is a collection of types that (in concept) share storage.
SR's unions are similar to those of other languages.
The syntax is similar to that of a record.
For example,
.DS
@type u = union (tag1 : type1; ...; tagN : typeN)@
.DE
declares @u@ to be the union of N types.
.PP
Unlike records, only one member of a union
\(em the one most recently stored \(em is valid at any time.
It is the programmer's responsibility to remember which one this is.
.PP
In the current implementation, union members do not actually share storage;
unions are implemented as records.
.SS "Strings"
The declaration of a string data type now uses @[size]@
instead of @(size)@ to specify the maximum length.
.SS "Multidimensional Arrays"
Arrays are no longer limited to a maximum of two dimensions.
Multidimensional arrays are treated as vectors of vectors (of vectors, ...).
For example, the following have identical signatures:
.DS
@var a[1:10][1:10] : int@
@var b[1:10] : [1:10] int@
@var c : [1:10] [1:10] int@
.DE
The standard mathematical notation for matrices is an abbreviation
for the vector of vector notation.
For example, @a[i][j]@ and @a[i,j]@ are equivalent.
This generalizes in the obvious way to higher dimensions.
.PP
The @lb1@, @lb2@, @ub1@, and @ub2@ functions have been replaced by more general
@lb(a,n)@ and @ub(a,n)@ functions.
The optional second argument @n@ specifies the dimension desired;
if present, it must be an integer literal,
and if omitted, the default is the first dimension.
.SS "Implicit Types for Initialized Variables"
If a variable is initialized, it need not be given an explicit type
unless the initializer is one of the ambiguous literals @null@ or @noop@.
.SS "Mutually Dependent Pointer Types"
A pointer declaration ``@ptr t@'' can now reference a type @t@
that is not yet defined.
Among other things, this allows the creation of record types
that contain pointers to each other.
.SS "Unspecified Array Ranges"
The specifier ``@*@'' is allowed as any upper or lower bound when
declaring an array type or formal parameter, or by itself to imply
an implicit lower bound of 1.
It may not be used for #both bounds of a dimension, but it may
appear in more than one dimension.
.SS "Semaphore Initialization"
Semaphore declarations can now include initialization,
as in
.DS
@sem mutex := 1@
.DE
However, assignment to semaphores remains illegal in all other cases.
.SH
Operations and Processes
.SS "Optional Identifiers in Operation Declarations"
Parameters and return values need not be named in an @op@ declaration, #e.#g.:
.DS
@op putrec (file, int, string[*])@
.DE
Names are still permitted for documentation purposes,
but only the types are actually required.
.SS "Procedure Declarations"
An operation declaration and @proc@ can be combined as follows:
.DS
@procedure@ operation_identifier (formals) [return_spec]
     block
@end@ [operation_identifier]
.DE
In this case, both the names and the types of the formals must be given.
This is an abbreviation for an @op@ declaration with a @{call}@ restriction
and for a @proc@ that implements the operation.
Hence, procedure declarations provide the dual of process declarations;
however, arrays of procedures are not allowed because an @op@ can be
implemented by at most one @proc@.
.SS "Reference Parameters"
Parameters declared as @ref@ parameters are passed by reference.
Reference parameters should not be passed between virtual machines.
Actual parameters passed by reference must be variables.
.SS "Process Priorities"
In version 1, all SR processes executed at the same priority.
In version 2, a new builtin procedure @setpriority(n)@ changes the priority
of the current process.
Process priorities are signed integers;
the main resource begins executing with a priority of zero, and
a new process inherits the priority of its progenitor.
Another new function @mypriority()@ returns the current priority.
.SH
Sequential Statements
.SS "Expressions as Statements"
Any expression can now be used in a statement context.
.SS "For-all Increment"
The increment of a for-all statement can now be specified by adding
a @by n@ clause to the quantifier.
.SS "Asynchronous Send"
Send invocations of operations on other virtual machines
now function asynchronously.
Before, they were semi-synchronous.
The difference is subtle, verging on undetectable.
.SS "Forward Statement"
The forward statement has the form:
.DS
@forward@ operation (expressions)
.DE
Execution of @forward@ takes the operation invocation currently being serviced
and sends it to the indicated operation.
An invocation can be forwarded to any operation having the same signature.
If the original invocation was the result of a call, the caller remains
blocked until the new invocation has been serviced (to completion or reply).
Thus, @forward@ defers replying to a called invocation and
instead passes on this responsibility.
The fact that an invocation was forwarded is invisible to the
original invoker.
.PP
After executing @forward@, the forwarding process continues with
the next statement just as if it had replied to the forwarded invocation.
A subsequent forward acts like a send, and a reply has no effect.
Similarly, return from within the block handling the invocation
has no effect on the caller if the invocation was called,
although it has the usual effect of causing the executing
process to exit that block, and therefore terminate.
.PP
While within the block handling a forwarded invocation,
the forwarding process may still reference formal parameters,
but no changes to @var@ or @res@ parameters will be seen
by any other process.
(Changes to reference parameters will be visible because such
parameters are in effect pointers.)
.SS "Create Statement"
In @create vm() on x@, the location @x@ may now be a string expression
specifying the name of a physical machine.
The particular value depends on the local network configuration.
.br
.ne 8
.SH
Expressions
.SS "Statements Generalized into Expressions"
Assignment, @create@, and pre- and post-decrement and -increment
are now expressions,
not statements, so they can be used in any expression context.
This allows increment or decrement inside a larger expression,
and allows the construction of multiple assignment statements
such as @a := b := 0@.
.SS "Indexing and Dereferencing"
An arbitrarily complex expression can now be
subscripted, sliced, or dereferenced.
.SS "Concatenation"
A simple @char@ expression can be used for either operand of the concatenation
operator.
.SS "New Operators"
A unary @+@ operator has been added for symmetry with unary @-@.
.LP
A new exponentiation operator, @**@, is valid for integers and reals.
.LP
A new modulo operation, @mod@, gives a result that is never negative
(unlike the @%@ operator).
.LP
Augmented assignment operators (@+:=@, @-:=@, etc.) have been added.
.SH
Conversion Functions
.LP
Explicit conversion functions now provide conversion between any two of
@int@, @real@, @char@, @bool@, @enum@, @ptr@, @string@, and array-of-@char@
types.
All combinations are allowed, although some make little sense.
.PP
Conversion to array of chars is provided by the function @chars(x)@.
Conversion to an enum type @T@ is @T(x)@.
The other cases are written as @int(x)@, @real(x)@, etc.
.SH
Math Functions
.LP
Several new math functions have been added.
All accept real arguments (or integers, by the conversion rules) and
return real results.
Angles are measured in radians.
Some arguments are optional.
The functions are:
.RS
.IP @sqrt(x)@ 16n
square root of @x@
.IP @log(x,b)@ 16n
log of @x@ to the base @b@ (default 2.7183...)
.IP @exp(x,b)@ 16n
@b@ (default 2.7183...) to the @x@ power
.IP @ceil(x)@ 16n
smallest integer not less than @x@
.IP @floor(x)@ 16n
largest integer not greater than @x@
.IP @round(x)@ 16n
nearest integer to @x@; even integer if tied
.IP @sin(r)@ 16n
sine of @r@
.IP @cos(r)@ 16n
cosine of @r@
.IP @tan(r)@ 16n
tangent of @r@
.IP @asin(x)@ 16n
arc sine of @x@
.IP @acos(x)@ 16n
arc cosine of @x@
.IP @atan(x,y)@ 16n
arc tangent of @x/y@; default @y@ is 1.0
.IP @random()@ 16n
pseudorandom number between 0.0 and 1.0
.IP @random(ub)@ 16n
pseudorandom number between 0.0 and @ub@
.IP @random(lb,ub)@ 16n
pseudorandom number between @lb@ and @ub@
.RE
.LP
The builtin procedure @seed(x)@ seeds the random number generator
with the real value @x@.
If @x@ is zero, or if @seed@ is not called,
@random@ generates an irreproducible sequence.
.SH
Input and Output
.LP
Alternate radices (octal and hexadecimal, using SR notation)
are allowed on integer input including @getarg()@.
.PP
Reals can be read and written.
.PP
New builtin procedures @printf@, @sprintf@, @scanf@, and @sscanf@
have been added.
They are similar to the equivalent C functions:
.DS I 3n
@printf (@[@file,@]@ format, arg...)@
@sprintf (buffer, format, arg...)@
@scanf (@[@file,@]@ format, arg...)@
@sscanf (buffer, format, arg...)@
.DE
The conversion specifier @%@@b@ can be used to convert boolean values.
For @scanf@ and @sscanf@,
the arguments are variables (not pointers to variables).
.PP
Files are flushed after every output call, so calls to @flush@ are now
superfluous.
.Ch "MultiSR: True Multiprocessing for SR"
.SH
Introduction
.LP
MultiSR is a configuration of the SR implementation
that provides true concurrency on a multiprocessor
without requiring the use of virtual machines or other explicit 
programming changes.
The MultiSR configuration is a new feature of Version 2, and
is available on the Silicon Graphics Iris and the Sequent Symmetry.
The effect on performance depends on the particular program;
``embarrassingly parallel'' algorithms like matrix multiplication
can achieve near-perfect speedup.
.SH
Using MultiSR
.LP
MultiSR behaves just like regular SR unless the environment variable
SR_PARALLEL is set.
If set to a number greater than 1, multiple copies of the program are forked;
they share memory and service a shared list of SR processes.
On the Sequent, one additional process services all I/O except for writes to
.B stdout
and
.B stderr .
The practical maximum for SR_PARALLEL is the number of processors.
.PP
For example, under #csh,
.DS
@env SR_PARALLEL=4 a.out@
.DE
executes #a.out with up to four SR processes running concurrently.
.SH
Limitations
.LP
Some differences can be observed when SR_PARALLEL is enabled:
.IP \(bu
Using multiprocessing adds a new dimension of nondeterminism to SR programs.
.IP \(bu
Forking the multiple Unix processes adds a delay when starting a program.
This is especially noticeable on a Sequent.
.IP \(bu
On the Sequent, processes do not (in general) share files.
Activity on any file except
.B stdout
or
.B stderr
necessitates a switch to the I/O process and results in a slowdown
compared to doing the same I/O without MultiSR.
Because files are not shared, external C functions should avoid doing I/O;
if this is not possible, these guidelines should be followed:
.RS
.IP
For
.B stdin ,
read all the way to EOF in a single external call.
.IP
For
.B stdout
or
.B stderr , 
flush output before returning.
.IP
For any other file, do all the I/O in a single external call:
open it, access it, and close it.
.RE
.Ch "CCR, Monitor, and CSP Preprocessors for SR"
.LP
Version 2 of SR includes three new preprocessors that convert
CCR notation, monitor notation, or CSP notation into equivalent SR programs.
The goal is to provide experience with those mechanisms
through hands-on experimentation.
These preprocessors together with SR provide a
complete teaching tool for a spectrum of synchronization mechanisms
(corresponding to chapters in Andrews's #Concurrent #Programming book):
shared variables, semaphores, CCRs, monitors, asynchronous message passing,
synchronous message passing (including output commands in guards,
as in extended CSP), and RPC and rendezvous.
Native SR directly contains the mechanisms other than CCRs, monitors, and CSP.
.PP
#ccr2sr converts CCR code to equivalent SR code.
Typically, the rest of the user's code is a single-resource program.
The CCR code and the code that use it are usually contained in a
single @.ccr@ file.
The program is preprocessed, compiled, and linked, for example, as follows:
.DS
@ccr2sr a.ccr@
@sr a.sr@
.DE
.PP
#m2sr converts monitor code to an equivalent SR global.
Typically, the rest of the user's code is a single-resource program.
Monitor operations are invoked from other SR code via call statements.
The SC, SX, SW, and SU signaling disciplines are supported;
the choice is made by a command line option.
The monitor code is usually in one file (say, @a.m@)
and the code to use it is in another (say, @test.sr@).
To build a running program, the first is preprocessed,
then both are compiled and linked.
For example:
.DS
    @m2sr a.m@
    @sr a.sr test.sr@
.DE
.PP
#csp2sr converts CSP code to equivalent SR code.
Specifically, it takes a CSP program and converts that
into a single-resource SR program.
A CSP program is preprocessed, compiled, and linked, for example, as follows:
.DS
@csp2sr a.csp@
@sr a.sr@
.DE
.PP
The exact syntaxes recognized by the preprocessors are not documented.
Sample programs are provided under the
.B examples
subdirectory of the SR distribution.
Syntactic peculiarities exist to ease implementation.
The preprocessor's #man pages provide further detail.
.PP
The implementations of #ccr2sr, #m2sr, and #csp2sr use just #cpp,
the C preprocessor, (with a little help from #sed) and so they are somewhat simplistic and far
from robust.
The deficiencies are noted in their #man pages.
The key point to keep in mind is that the preprocessors maintain
almost no state information; in particular, they are not parsers and
do almost no error detection, leaving that to the SR compiler.
For example, #ccr2sr does not check that a matching region statement has
been started when it processes an end statement for that region.
.Ch "Impacts on Programs Written for Version 1 of SR"
.LP
This section lists several of the changes that potentially affect
existing programs.
It is not possible to enumerate all of the possible impacts.
Of the ones of which we are aware, these are the most important.
.SH
The Programming Environment
.LP
There is no binary compatibility with version 1.
Version 1 programs must be recompiled to be linked with version 2 code.
.LP
Source files are now read twice by the compiler: first to export the specs,
then to compile the bodies.
.LP
The compiler's @-S@ option is gone, and @-d@ replaces @-D@.
.LP
All intermediate files including @.o@ files are now put in the Interfaces
directory.
.LP
The #srl command arguments must now list the globals to be linked as well
as the resources.
.LP
Some of the changes in internal data structures may be visible to
sophisticated externals.
Simple types are unaffected.
.LP
The Makefile generator, #srm, was significantly changed.
The visible effects are:
.RS
.Bu
There is no default set of @.sr@ files;
they must be supplied as #srm arguments.
.Bu
The @-r@ option (main resource name) is renamed @-m@.
.Bu
The @-g@ and @-G@ options (call #make automatically) are gone.
.Bu
The @-s@ and @-S@ options (semi-smart mode) are gone.
.Bu
The main resource must be specified explicitly if there is not
exactly one resource not imported by any other.
.Bu
The generated Makefiles are different from those of version 1.
.RE
.LP
Installation and configuration procedures for SR have changed.
.SH
Syntax
.LP
There are several new keywords:
@begin@, @forward@, @high@, @low@, @mod@, @procedure@, @real@, @ref@,
@union@, and @vm@.
.PP
Internal compiler changes affect the way newlines in source programs are
handled.
Generally, no source program changes are required.
If an ``unexpected newline'' diagnostic occurs,
it can be fixed by escaping the newline with a backslash
(#i.e., add @\e@ at the end of the offending line).
.PP
Newlines within string and character constants must now be escaped.
.PP
Multiple statements on a line must be separated by @;@.
.PP
In accordance with the rules for enums, predefined enums such as @READ@ can no
longer be redefined.
.PP
Type restrictions @{public}@ and @{private}@ are no longer part of the language,
and are ignored.  A warning is given.
.SH
Resources
.LP
The initial block no longer exists;
the keyword @initial@ is mapped to @begin@, and a warning is given.
The top-level code in a resource is executed sequentially, with both
declarations and statements processed as they are encountered.
.br
.ne 12
.PP
Because of this, it is now possible to access an uninitialized op or variable.
For example:
.DS
@resource r ()@
@    op p()@
@    p()@
@    const N := 3@
@@
@    proc p ()@
@       write (N)@
@    end p@
@end r@
.DE
In this example, an uninitialized (garbage) value of @N@ is written.
Although @N@ is in the static scope of proc @p@,
@p@ is called before the const declaration is executed.
This situation may occur in previously working SR programs
when a former @initial@ block is executed at its point of appearance
(instead of after processing all declarations).
.PP
Self-referential resource parameter lists such as @resource r (n, a[n]:int)@
are now illegal.
.PP
Extending a concrete resource no longer adds its arguments to the local
resource's argument list.
.PP
The semantics of globals have changed to be more like resources,
but the changes should be transparent except in unusual situations.
.SH
Operations
.LP
The bounds of an array of exported operations must be constant at compile time.
.PP
An operation cannot be serviced by @receive@ if it has var or res parameters
or if it returns a value.
.SH
Strings
.LP
The syntactic form @string(*)@ is replaced by @string[*]@.
The old form is accepted and a warning is given.
.PP
Strings are no longer compatible with character arrays.
Explicit conversion functions are provided.
.PP
A slice of a string now has the signature of a string, not a character array,
even if the slice has only one element.
However, if a string is subscripted with a #single index,
the result is a @char@.
.SH
Arrays
.LP
One- and two-dimensional arrays are no longer considered compatible.
Version 1
treated them as compatible although this was contrary to the specification.
.PP
A slice bound of @*@ means the corresponding upper or lower bound.
This was documented in the version 1.1 language report,
but the implementation allowed an upper bound @*@ to mean ``whatever comes
out right'' when a slice was the target of an assignment.
.PP
All elements of an array constructor must now have identical types.
Version 1 only required compatible types in an array constructor,
but it generated bad code depending on the particular elements involved.
.PP
Two arrays of strings are considered compatible even if the maximum
lengths differ.
The actual lengths must fit at execution time.
.PP
By default, array indices are now checked for validity at execution time.
This can be inhibited by compiling with the @-O@ option.
.PP
Predefined functions @lb1@, @lb2@, @ub1@, and @ub2@ no longer exist.
They are mapped to the new forms @lb(a,1)@, @lb(a,2)@, @ub(a,1)@, and
@ub(a,2)@.
.PP
Arrays cannot be compared.
.SH
Records
.LP
The size of an array or string contained in a record must be constant at
compile time.
.PP
Records cannot be compared.
Version 1 allowed this, but the generated code was not trustworthy.
.SH
Input and Output
.LP
The builtin @read(c)@ operation now works as documented when @c@ is a @char@:
it returns the first (nonblank) character
of the next whitespace-separated word.
.PP
By default, output is now synchronous; the entire SR program can block on
output, though in practice any delays are typically unnoticeable.  With this
change, adding @write()@ calls no longer perturbs the flow of program control.
The old behavior, where output does not block, is obtainable by specifying
@-A@ as an #srl argument.
.PP
@close(f)@ no longer sets @f@ to null
because @f@ can now be any file-valued expression
and need not be assignable.
.PP
The I/O return value @EOF@ is now a predefined constant, not a keyword.
.SH
Error Detection
.LP
Version 2 detects and diagnoses more programming errors than version 1,
both at compilation time and at execution time.
Some erroneous programs that ran to completion under version 1
now fail when run under version 2.
.PP
Compiler diagnostics are completely different.  Errors detected early in
compilation inhibit later compiler passes that might detect other errors.
.PP
Errors in the spec portion of a resource (other than syntax) are not diagnosed
until compilation of a body of a resource that imports that spec.  These
errors are reported in conjunction with every such body.
.PP
Some version 1 compiler warnings, such as for unreachable code or unreferenced
identifiers, are not given by the version 2 compiler.
.PP
Some error diagnostics that are common to both versions of SR are now
presented in different forms.
.Ch "Implementation Limitations"
This section lists some limitations of the current implementation.
Some of these could reasonably be considered bugs.
.SH
The Programming Environment
.LP
Source and object files are generally expected to reside in the current
directory;  explicit paths elsewhere don't always work.
.PP
Keywords in strings or comments occasionally make
.I srgrind
think it saw
a @pb@ (procedure begin), typesetting a new name in the margin.
.SH
Problems Seen at Compilation Time
.LP
Slices cannot be used with the swap (@:=:@) operator.
.PP
Builtin procedures having string result parameters
(@read@, @get@, @scanf@, @getarg@, @sprintf@)
cannot accept slice expressions for those parameters.
.PP
Unions are really implemented as records; no storage is shared.
.PP
Some older C compilers disallow the expression @f().m@ where @m@
is a member of the struct returned by function @f@.
If C lacks this support, the corresponding expression in SR
will fail to compile.
.PP
SR Version 2 is a multipass compiler.
If it finds an error on one pass, it skips the later passes,
with the consequence that not all errors are reported at once.
Errors in @spec@ sections may be reported once for each import.
.SH
Problems Seen at Execution Time
.LP
The bounds of an array passed by @ref@ are those of the underlying array,
not those declared for the parameter.
.PP
Null and noop capabilities cannot be created for resources that export arrays
of operations if any of those arrays has more than three dimensions.
.PP
Resource and global variables referenced in the synchronization part of an
input statement are never reexamined,
even though they can be changed by other processes.
.PP
Destroying a resource is not an atomic operation.
If processes in two
resources each simultaneously try to destroy the other resource, one or
both of the processes can be killed, leaving one or both resources
incompletely destroyed.
.SH
Problems Involving Distributed Programs
.LP
#srx generates an incorrect network file path if the executable is found not
in the current directory but in the shell search path.
The effect of this is that a distributed SR program can only be run
when it is in the current directory.
.PP
Because of network pipeline delays,
output from different virtual machines may be misordered,
even if it is explicitly synchronized.
.PP
Error messages can sometimes occur, due to race conditions, during normal
termination of distributed programs.
These can be caused, for example, if X exits while Y is sending to it.
.PP
Virtual machines don't always shut down cleanly if an SR program is run in
the background under the Bourne shell.
This is noticed when running #srv under #sh or #at.
.br
.ne 12
.SH
Undetected Errors
.LP
Although version 2 of SR detects many more execution time errors than version 1,
there are still several kinds of errors that are not detected and can cause
erroneous behavior.
Some of the more common ones are listed here:
.Bu
Arithmetic overflow is not detected by either the compiler or runtime system.
.Bu
The use of an uninitialized variable or operation is not detected,
nor is the use of a @ptr@ or @ref@ parameter on a different virtual machine.
.Bu
Values of enumeration types are not checked at execution time.
.Bu
Unreachable code is not diagnosed by the version 2 compiler.
.Bu
No warning is given if an invocation is still pending when an operation
terminates.
