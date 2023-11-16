.\"  format with troff or nroff using the -ms macro package
.so common.ms
.Nm INSTALL
.TL
Installing the SR Programming Language
.br
Version 2.3
.AU
Gregg Townsend
.AI
Department of Computer Science
The University of Arizona
.sp 1
October 7, 1994
.sp 1
.ps -1
Minor update August 12, 1999 for SR 2.3.2
.ps
.de CD
.nf
.sp .4
.in 10n
\\fB\\$1\\fR
.in
.sp .4
.fi
..
.SH
General Notes
.LP
The SR Programming Language runs under several different Unix systems
including those of Sun, Digital, Hewlett-Packard, Silicon Graphics, and IBM.
A list is included in the
.B Systems
file in the main directory.
.PP
The standard distribution package requires about five megabytes of disk space.
This expands to around ten megabytes during the build process.
The final, installed files occupy about three megabytes.
.PP
The SR system is in the public domain and you may use and distribute it as you
wish.  We ask that you retain credits referencing the University of Arizona and
that you identify any changes you make.
.PP
We can't provide a warranty with SR; it's up to you to determine its suitability
and reliability for your needs.  We do intend to continue to develop and
maintain SR as resources permit and we would like to hear of any problems.
.SH
Unpacking the \fItar\fP file
.LP
If you haven't done so already, create a new directory for building the
SR system.
This directory can be located anywhere and can be removed completely
after the system has been built and installed.
The new directory is called the ``main directory''
in these instructions
and is assumed to be your current directory for all commands illustrated.
.PP
Make the main directory your current directory
and unpack the tar file.
This creates several files and subdirectories.
The procedure for unpacking the tar file depends on its form.
.PP
If you have a tar file on tape,
or already online in a disk file,
enter
.CD "tar xf \fIfile\fP"
where
.I file
is the tape device or disk file name of the SR distribution.
.PP
If you have a pair of diskettes, unpack only Disk 1.
Ensure that it is write protected
(the tiny windows in the corners should both be open)
and insert it in the diskette drive.
Enter
.CD "dd if=\fIdev\fP bs=36b | uncompress | tar xf \-"
where
.I dev
is the raw diskette device.
On a Sun, for example, this is
.B /dev/rfd0c;
or, if the volume daemon is running, use
.B /vol/dev/aliases/floppy0
after first running
.B volcheck .
Some systems may report ``Broken pipe'' or ``dd: write error''
at the end of the unpacking step;
don't worry about this.
It occurs because
.B dd
doesn't stop at the end of the tar file.
.SH
System Components
.LP
The SR system consists of several components.
.B sr
and
.B srl
are the key programs used directly by an SR programmer.
The SR runtime system is loaded implicitly by \fBsrl\fP.
Supplemental tools, example programs, and documentation are also provided.
.PP
.B sr
is the compiler proper.
It translates an SR program into a C program
and then calls the C compiler to produce object code.
.PP
.B srl
is the SR linker.
It generates tiny C files of configuration information
and invokes the C compiler to combine these with object
files and the runtime library.
The end product is an executable program.
.PP
.B srx
is an executive program that supervises ``distributed'' SR programs:
those that use \fBcreate vm()\fP.
It is not called explicitly but is forked automatically by the runtime system.
.PP
.B srm
assists in the construction of complex programs by
inspecting SR source code and automatically
creating a correct
.I make
description file.
.PP
.B srprof
reads a trace file produced during the execution of an SR program
and produces a report of event counts by line number or an annotated listing
showing the counts.
.PP
.B srtex
formats an SR program for typesetting by TeX;
.B srlatex
formats a program for LaTeX.
.B srgrind
does a similar job for use with
.I troff .
.B srgrind
requires the
.I vgrind
program in order to function.
.PP
.B ccr2sr ,
.B m2sr ,
and
.B csp2sr
are preprocessors that convert CCR notation, monitor notation,
and CSP notation (respectively) into equivalent SR programs.
.PP
The
.B library
contains resources, globals, and externals that can be linked with SR programs.
It includes an option processor and interfaces to X Windows and to the 
XTANGO animation package.
.PP
.B srv
runs verification tests to check that the SR system is functioning correctly.
.B srvi
installs new verification tests.
These programs are part of the installation process and are not used
by SR programmers.
.SH
Directory Structure
.LP
Important files in the main directory include:
.IP
.RS
.IP \fBREADME\fP 17n
General release information and last minute notes.  READ THIS FILE!
.IP \fBSystems\fP 17n
Detailed system-specific information.  READ THIS, TOO.
.IP \fBConfiguration\fP 17n
File paths and other miscellaneous configuration data.
.IP \fBMakefile\fP 17n
The master file of
.I make
directives for building SR.
.IP \fBsr-mode.el\fP 17n
Lisp code implementing an SR editing mode for GNU Emacs.
.RE
.LP
There are also several source code files and other files used in the
build process.
.LP
Subdirectories of the main directory are:
.IP
.RS
.IP \fBsr\fP 17n
Source code for \fBsr\fP, the SR compiler.
.sp -.3
.IP \fBsrl\fP 17n
Source code for \fBsrl\fP, the SR linker.
.sp -.3
.IP \fBrts\fP 17n
Source code for the SR runtime system, including \fBsrx\fP.
.sp -.3
.IP \fBlibrary\fP 17n
Source code for the SR library routines.
.sp -.3
.IP \fBcsw\fP 17n
Platform-specific runtime code for context switching.
.sp -.3
.IP \fBmulti\fP 17n
Platform-specific runtime code for multiprocessing support.
.sp -.3
.IP \fBsrm\fP 17n
Source code for \fBsrm\fP, the Makefile builder.
.sp -.3
.IP \fBsrgrind\fP 17n
Source code for \fBsrgrind\fP, the troff formatter.
.sp -.3
.IP \fBsrtex\fP 17n
Source code for \fBsrtex\fP, the TeX formatter.
.sp -.3
.IP \fBsrlatex\fP 17n
Source code for \fBsrlatex\fP, the LaTeX formatter.
.sp -.3
.IP \fBpreproc\fP 17n
Source code for the SR preprocessors
.B ccr2sr ,
.B m2sr ,
and
.B csp2sr .
.sp -.3
.IP \fBsrv\fP 17n
Source code for \fBsrv\fP and \fBsrvi\fP, the verification tools.
.IP \fBlinks\fP 17n
A collection of symbolic links pointing to locations where the executables
are built.
This directory can be put in a search path to assist in testing.
.IP \fBvsuite\fP 17n
A suite of verification programs used by \fBsrv\fP.
.IP \fBexamples\fP 17n
Examples of SR programs,
including programs from the SR book
and programs that utilize library routines.
Also included are examples of CCR, monitor, and CSP programs
for use with the preprocessors.
.B examples
is actually a symbolic link to \fBvsuite/examples\fP.
.IP \fBps\fP 17n
Preformatted documentation, in PostScript format.
See the
.B README
file in that directory for details.
.IP \fBman\fP 17n
Individual
.I man
pages for the various programs in the SR system.
.IP \fBdoc\fP 17n
Source for the rest of the documentation, in
.I troff
form.
.IP \fBnotes\fP 17n
Some miscellaneous text files, not necessarily accurate or current,
containing additional, informal documentation that may or may not be useful.
.RE
.SH
Configuring the SR System
.LP
Before you build SR, you must decide where to install it.
Pathnames are embedded in the binaries so that (for example)
.I sr
can call
.I srl
and
.I srl
can find the runtime library.
Five directories must be specified: one for the commands,
one for hidden files such as the runtime library,
and three for the
.I man
pages.
All of the installed files have names beginning with
.B sr 
or containing the string
.B 2sr .
.PP
If you plan to install SR as a local utility you might choose
distinct directories such as
.B /usr/local/bin,
.B /usr/local/lib/sr,
and then
.B /usr/man/manl
for all the
.I man
pages.
Alternatively, everything can be collected in a single directory
such as
.B /usr/sr/bin
or
.B /home/yourname/bin.
Do not use any
.I existing
directories within the SR distribution,
but you can safely create a new
.B bin
subdirectory under the main directory.
.PP
If the directories you have chosen do not now exist,
you must create them manually.
The installation process does not create new directories.
.PP
To configure the system, edit the file
.B Configuration
in the main directory.
Change the SRSRC definition to reflect the path of the main directory.
Define installation directories as described above for
SRCMD, SRLIB, MAN1, MAN3, and MAN5.
All directories must be absolute paths (beginning with `/'),
and no comments may appear on the definition lines.
.PP
If the X window system is installed, set XINCL to the parent directory
of the X11 include directory.  Usually, that is
.B /usr/include .
On a Sun running OpenWindows, set XINCL to
.B /usr/openwin/include .
If X is not installed, set XINCL to be empty.
.PP
If you wish to use the animation interface provided in the library,
you must first obtain and install the XTANGO package from Georgia Tech.
At this writing it is available by anonymous FTP from
the /pub/people/stasko directory on
.B ftp.cc.gatech.edu .
Then define XTANGO as the name of the XTANGO
.B include
directory.
.PP
If the
.I vgrind
program is available on your system, set VFPATH and VGMACS to the absolute
paths of its back end program and macro package.
Typical paths for Berkeley-derived systems are
/usr/lib/vfontedpr and /usr/lib/tmac/tmac.vgrind
respectively.
If
.I vgrind
is not available, set these definitions empty.
Note: Without
.I vgrind,
.I srgrind
does not function.
.PP
A few other values can also be configured in the main
.B Makefile .
MANEXT defines the file extension for installed
.I man
pages.
The \fIxx\fPPATH definitions define the location of the C compiler
and other utilities called from within SR commands,
but do
.I not
affect the building of SR.
The CFLAGS definition sets compilation options for use while building SR.
.PP
Some operating systems require additional changes to
.B Configuration
and/or
.B Makefile .
See the section in the
.B Systems
file describing your particular operating system.
.SH
Configuring for MultiSR
.PP
A multithreaded version of SR, called MultiSR, is available for
Intel Linux, Sun Solaris, Silicon Graphics Irix, and Sequent Symmetry systems.
MultiSR provides true concurrency on a shared-memory multiprocessor
without requiring any programming changes.
.I Note:
MultiSR is 
.I not
used on the Intel Paragon; multiprocessing there
utilizes SR's virtual machine facilities.
.PP
To configure MultiSR, edit the main Makefile and define
.ta 10n 30n
.RS
.RS
.IP MULTI=linux\-x86 20n
for Linux on Intel x86
.IP MULTI=solaris
for Solaris
.IP MULTI=irix
for Irix
.IP MULTI=dynix
for Sequent
.RE
.RE
.ne 12
.PP
A few system-dependent configuration changes are also needed as noted below.
.RS
.RS
.nf
.IP "Intel Linux:" 20n
(no other changes needed)
.IP "Sun Solaris:" 20n
Define LIBR=\-lthread in the \fBConfiguration\fP file.
.IP "SGI Irix:" 20n
Define LIBR=\-lmpc in the \fBConfiguration\fP file.
.IP "Sequent Symmetry:" 20n
Define LIBR=\-lpps in the \fBConfiguration\fP file;
define CFLAGS=\-Y in the main \fBMakefile\fP.
Ignore warning messages about ``Parallel library not detected''
that occur during the build.
.fi
.RE
.RE
.SH
Building and Testing
.LP
To build the SR system, simply type
.CD "make"
in the main directory.
This builds all the components of the SR system
within the SR directory structure,
altering nothing outside the structure.
The build process takes two to ten minutes on a typical modern workstation.
.PP
If you later need to change the configuration information,
do so and again type \fBmake\fP.
This sort of rebuild goes relatively quickly because only a few
files need to be recompiled.
.PP
After building the system,
check that it is functional by entering
.CD "srv/srv \-v quick"
to run the ``quick'' set of SR tests.
This prints some environmental information and then runs a small
set of tests.
.B \-v
causes each verification directive to be echoed.
The ``quick'' tests may actually take a few minutes, even on a fast machine;
quick/jumble, in particular, has grown to over 500 lines of SR and is
compiled and executed twice.
.PP
No error messages are expected.
If ``expected 0, got 1 from $RSHPATH'' appears for the
.B quick/vm
test, it indicates a problem running the
.I rsh
(or \fIremsh\fP) program.
This is discussed below under
.B "Configuring Virtual Machines".
.PP
It is not necessary to complete the installation
to manually test the newly built system.
If you put the 
.B links
subdirectory in your search path,
you can compile SR programs by running 
.B sr
and
.B srl
with the
.B "\-e"
option.
This causes them to load libraries and other files from within the source
directory instead of from the ultimate, installed locations.
.SH
Installing the System
.LP
After the system has been built and verified,
it must be installed in its ultimate destination as configured above.
Type
.CD "make install"
to copy the commands,
.I man
pages, and support files.
.PP
To verify a correct installation, type
.CD "srv/srv \-v \-p quick"
to run the same tests as before, but using the installed files.
.PP
The install script places
.B sr_mode.el ,
an SR editing mode for GNU Emacs, and
.B srlatex.sty ,
a LaTeX style file for use with
.I srlatex ,
in the directory configured as SRLIB.
Emacs and LaTeX will not look for them there, however.
If you use those programs, we recommend that you put symbolic links
in the directories that they search.
The symbolic links should point to the files in SRLIB.
.SH
Configuring Virtual Machines
.LP
At this point,
a complete SR system has been built and installed.
Without further reconfiguration, however,
remote virtual machines may not work properly.
This reconfiguration was deferred
until now in order to have a working SR system as a testbed.
If you don't have a network of machines or don't need to use
.B "create vm() on"
.I n ,
you can skip this section.
If you're building SR on an Intel Paragon, you should also skip this section.
.PP
When a program creates a virtual machine
and specifies a host machine on which to place it,
SR uses a remote shell to run the program on the remote host.
The remote shell program is
.I rsh (1)
on most systems or
.I remsh (1)
on some System V derived Unixes.
Set the RSHPATH definition in the Configuration file
to name the correct program.
Under Solaris 2, use
.B /bin/rsh .
.PP
Verify that
.I rsh
or
.I remsh
is working by entering
.CD "rsh `hostname` date"
If necessary, substitute
.B remsh
for
.B rsh
and/or
.B "uname \-n"
for
.B hostname.
If this doesn't print the date, there is a configuration problem
outside of SR; seek local assistance.
.PP
The name of the executable file passed to
.I rsh
is the same for all remote hosts and is controlled
by an
.B srmap
file.
This contains patterns for matching the program's filename
and corresponding templates for generating the remote filename for
.I rsh.
The format of
.B srmap
is described in its \fIman\fP page,
\fBman/srmap.5\fP.
.PP
If your network provides transparent access to remote disks,
it should be possible to make remote execution work automatically
by specifying templates that generate host-independent filenames.
The 
.B srmap.az
file in the main directory
is an example of how this is done at Arizona.
.PP
Without remote disk access,
users of multiple virtual machines will need to manually
copy their programs to the remote hosts
(\fIe.g.\fP, using
.I rcp
or
.I rdist )
before beginning execution.
The configuration in
.B srmap
controls where the programs must be placed,
so a simple and straightforward method is desirable.
One way to do this is to generate a path relative to the
user's home directory.
For example, if \fBsrmap\fP contains the line
.CD "sequoia:/usr?/*/**     ~$2/$3"
then when a program
.B /usr3/username/path
is run on host \fBsequoia\fP,
SR expects to find copies of the executable in
.B ~username/path
on remote machines.
.PP
Edit
.B srmap
in the main directory
and set up a configuration appropriate to your local situation.
Refer to the
.B srmap
.I man
page for a detailed description of the format,
and use the Arizona configuration as a starting point and an example.
.PP
Make sure that your search path includes the directory where you
just installed \fBsr\fP,
and type 
.B rehash
if your shell requires it.
Go into the
.B examples/remote
subdirectory and type
.CD "sr remote.sr"
to compile a test program.
Set the environment variable SRMAP
to the
.I absolute
pathname of the new
.B srmap
file.
.PP
Begin with a simple test by typing
.CD a.out
to run the program with no arguments.
It should simply tell you the local hostname;
this verifies that
.B srx
is accessible,
and that the location of
.B a.out
matches one of the patterns in \fBsrmap\fP.
Then try giving the local hostname as an explicit argument;
this verifies that the generated filename works on the present host.
.PP
Now add other hostnames as command arguments;
if remote execution fails (perhaps as expected),
then the diagnostics from
.I rsh
give the file name attempted.
Copy
.B a.out
into other directories,
and onto other hosts if necessary.
Run it from various locations with various hostname arguments.
Run it using absolute and relative paths.
Try to test any special cases used in the
.B srmap
file.
.PP
When you are satisfied with the configuration in \fBsrmap\fP,
return to the main directory and type
.CD "make install"
to reinstall the system including the revised
.B srmap
file.
.SH
Cleaning Up
.LP
After the system has been installed, nothing within the SR directory structure
is needed to build or run SR programs.
The structure can be backed up on tape and removed from the disk.
.PP
If you wish to keep the source code online, type
.CD "make clean"
to remove executables and intermediate files from the build process.
.B Warning:
the cleanup process is a bit aggressive, and it removes all files
within the directory structure that satisfy certain tests.
If you have created files of your own within the structure, and you
wish to preserve them, it would be prudent to first copy them elsewhere.
.PP
The
.B doc
and
.B ps
directories can be deleted manually if you do not wish to retain them.
.SH
The Full Verification Suite
.LP
The standard distribution of SR includes a few confidence tests
and sample programs in the main
.I tar
file.
These are the tests run by \fBsrv/srv quick\fP,
and should be sufficient to verify correct installation of
an unmodified SR system.
.PP
A more extensive set of tests is available for those who wish
to modify the system or transport it to a different system
architecture.
There are about 450 tests in 2400 files,
requiring about three megabytes of disk space.
The set includes a
.B timings
subdirectory of performance tests.
.PP
The full test set is provided as a second diskette,
a second
.I tar
file on tape,
or as a separate file for
.I ftp
distributions.
This
.I tar
file, when unpacked in the main directory,
creates several new subdirectories of \fBvsuite\fP.
If
.B srv/srv
is run with no parameters it executes all of the tests.
A full run typically takes between one and four hours.
.SH
Porting to Other Systems
.LP
It is possible to port SR to other system architectures
besides those presently supported;
some assembly language programming is required.
32-bit Unix systems with conventional memory models
are most easily accommodated.
Instructions for porting SR are contained in
\fBdoc/port.ms\fP.
.SH
Feedback
.LP
Please let us know of any problems you encounter
so that we can continue to improve SR.
We can be reached by electronic mail at:
.DS
sr-project@cs.arizona.edu
.DE
Our FAX machine is at:
.DS
+1 520 621 4246
.DE
Our mailing address is:
.DS
SR Project
Department of Computer Science
Gould-Simpson Building
University of Arizona
Tucson, Arizona  85721
.DE
We'll need to know what computer and operating system you are using,
what version of SR,
and your name, address, and telephone number.
.PP
Because of limited resources we can't promise to fix every problem,
but we appreciate all comments and acknowledge all mail.
.SH
References
.LP
Gregory R. Andrews and Ronald A. Olsson,
.I "The SR Programming Language:  Concurrency in Practice"
Benjamin/Cummings, 1993, ISBN 0-8053-0088-0.
.LP
Gregory R. Andrews, Ronald A. Olsson, et al.,
.I "An Overview of the SR Language and Implementation."
ACM Trans. on Prog. Lang. and Systems 10, 1 (January, 1988), 51-86.
