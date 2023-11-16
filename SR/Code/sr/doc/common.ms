.	\" common setup for -ms documents
.if t .nr PO 1.25i
.ds lq \&"\"
.ds rq \&"\"
.if t .ds lq ``
.if t .ds rq ''
.nr PI 3n
.ds CH 
.ds RF \\n%
.	\" .Nm WORD -- set up document label for ordering system
.de Nm
.ds LF \\s7SR2.3\-\\$1\\s0
..
.	\" boilerplate for footnote giving pointer to UA
.nr FL \n(LL
.FS
The SR Programming Language comes from the Department of Computer Science,
The University of Arizona, Tucson, Arizona 85721 USA.
The implementation is available by anonymous FTP from ftp.cs.arizona.edu,
and the SR Project may be reached by sending electronic mail to
sr-project@cs.arizona.edu.
.FE
