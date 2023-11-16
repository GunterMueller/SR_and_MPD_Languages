.\"  format with troff or nroff using the -ms macro package
.ds LF \\s7SR\-CODING\\s0      \" document label
.if t .nr PO 1.25i
.LP
.DS C
\fBCoding Conventions for the SR System\fP
.sp .5
.ps -1
\fIGregg Townsend\fP
Department of Computer Science  /  The University of Arizona
September 1, 1989
.ps
.DE
.PP
This document describes the coding conventions used in the C code
that implements the SR programming language.
These conventions are not necessarily better
than other conventions in common use;
the main thing they provide is consistency.
Consistency serves two main purposes:
improved readability and ease in processing of the source code by programs.
.SH
.RS
.de IT
.IP \(bu 3n 
\\$1:\0
..
.IT "Indentation"
Four spaces per level, using tabs for 8 spaces when possible.
.IT "Line length"
Not to exceed 80 characters with tabs expanded.
.IT "Inter-line spacing"
No formfeeds (^L) anywhere.
Three blank lines between functions, and if appropriate
between major sections of global declarations.
Single blank line when useful for clarity in other situations.
.IT "Function declarations"
Preceded by block comment giving name, purpose, and any other needed details.
Storage/type class on separate line preceding function name.
No indentation on argument declarations.
Function body, excluding braces, indented four spaces.
Braces enclosing functions on lines by themselves.
.IT "Braces"
Beginning brace at end of line for construction involved (except
for function declarations as noted above).
Ending brace on separate line aligned with beginning of line
of opening construction.
.IT "\fBIf\fP statements"
Statement below conditional expression, indented four spaces.
If \fBelse\fP is present, on separate line
(following brace if present)
aligned with corresponding \fBif\fP line.
.IT "\fBSwitch\fP statements"
Case labels indented under \fBswitch\fP;
case bodies further indented under each case.
If possible for entire switch body (usually it isn't), use tabular format
with each \fBcase\fP...\fBbreak\fP sequence on a single line
with \fBbreak\fPs aligned.
.IT "Unary operators"
No space following operator.
.IT "Binary operators"
One space on either side.
.IT "Argument lists"
One space before opening parenthesis.
One space after each comma.
.IT "Return statements"
No parentheses around argument of \fBreturn\fP.
.IT "Casts"
One space between right parenthesis and operand.
.IT "Line comments"
Positioned by tabs to column 41 (5th tab) or beyond.
No ``cute'' or illiterate comments.
No personal identifications
(the lose utility with age, and just clutter things up).
.IT "Block Comments"
Blank line before but not after.
Text begins on opening line;
no text on closing line if multi-line comment.
Text aligned on same four-column boundaries as code.
One `\fB*\fP' on each body line, aligned with 
`\fB*\fP' on opening and closing lines.
No decorative frame on top, bottom, or right side.
.IT "Conditional compilation"
One blank line before and after \fB#ifdef/endif\fP groups.
Identifying C-style comment on \fB#else\fR and \fB#endif\fP.
Minimize use of conditional compilation; try to restrict to \fB.h\fP files.
.IT "Case conventions"
Full upper case for \fBcpp\fP macros including defined constants.
Mixed case for defined types.
Lower case for variables and functions.
.IT "Deviations"
Spaces can be removed to make a line fit within 80 columns.
Other deviations are allowed if the situation is a rare one
and the result is clearer code.
Use common sense.
