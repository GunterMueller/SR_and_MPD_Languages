.TH SRTRACE 5 "25 January 1993" "University of Arizona"
.SH NAME
srtrace \- runtime event trace file for SR program
.SH DESCRIPTION
The runtime events of any SR program can be recorded during execution in an
.I srtrace 
file by setting the environment variable SR_TRACE to the name of the 
.I srtrace 
file.
Two specific names, \fIstdout\fP and \fIstderr\fP, can be used
to direct the trace output to standard output and standard error 
output respectively.
.LP
Each line of the 
.I srtrace 
file has the following fields:
.LP
.B Filename
.RS
Name of the file where the corresponding SR statement is located.
.RE
.LP
.B Line number
.RS
Line number of the corresponding SR statement.
.RE
.LP
.B Proc name
.RS
Proc name of the corresponding SR statement.
This has the form
.RS
[\fIvm\fP(\fIn\fP).]\fIresource\fP.\fIproc\fP
.RE
which reads as 
.I proc 
in 
.I resource 
of virtual machine 
.I n.
.I vm(n)
is displayed only for distributed programs.
.RE
.LP
.B Event
.RS
Name of the event used in 
.I srtrace 
file.
The available events are as follows:
.nf
.ta 14n 51n
.de Ev
\\$1	\\$2	\\$3
..
.LP
.Ev "\fBEvent\fP" "\fBMeaning\fP" "\fBSR statement\fP"
.LP
.Ev CREATER "creation of resource" create
.Ev CREATEG "creation of global" import
.Ev CREATEV "creation of virtual machine" create
.Ev DESTROYR "destruction of resource" destroy
.Ev DESTROYV "destruction of virtual machine" destroy
.LP
.Ev CALL "synchronous invocation" call
.Ev SEND "asynchronous invocation" send
.Ev FORWARD "transfer of responsibility for reply" forward
.Ev REPLY "reply and continue execution" reply
.Ev RETURN "terminate and return" return
.LP
.Ev BODY "beginning of resource init code" resource
.Ev ENDBODY "end of resource init code" end 
.Ev FINAL "beginning of resource final code" final
.Ev ENDFINAL "end of resource final code" end
.Ev PROC "beginning of service by proc" proc
.Ev ENDPROC "end of service by proc" end
.LP
.Ev IN "entry to input statement" in
.Ev ARM "service of arm of an input statement" "\fR\->\fP"
.Ev NI "exit from an input statement" ni
.LP
.Ev CREATES "creation of semaphore" sem
.Ev INITS "initialization of semaphore" 
.Ev P "beginning of P operation" P
.Ev CONTP "completion of P operation" 
.Ev V "V operation" V
.LP
.Ev CO "beginning of co statement" co
.Ev OC "end of co statement" oc
.fi
.RE
.LP
.B Process ID
.RS
A hexadecimal number identifying the particular process that
generated the event.
.RE
.LP
.B Additional Field
.RS
For the following events, there is a second hexadecimal
number following the process ID. The meanings are:
.LP
.nf
.ta 1i
RETURN	process ID of the invoker
REPLY	process ID of the invoker
FORWARD	process ID of the invoker
ARM	process ID of the invoker
NI	process ID of the invoker
PROC	process ID of the invoker
ENDPROC	process ID of the invoker
.LP
INITS	initial value of the semaphore
.LP
CREATES	semaphore ID
P	semaphore ID
CONTP	semaphore ID
V	semaphore ID
.fi
.LP
Other events have 0 in the place of this field.
.RE
.SH EXAMPLE
Some sample output:
.LP
.nf
.ta 14n 30n 41n 49n
CS.sr, 15	main.body	BODY	1730b8	0
CS.sr, 16	main.body	CREATEG	1730b8	0
CS.sr, 1	CS.body	BODY	173168	0
CS.sr, 5	CS.body	SEND	173168	0
CS.sr, 1	CS.body	ENDBODY	173168	0
CS.sr, 5	CS.arbitrator	PROC	1731c0	173168
CS.sr, 20	main.body	SEND	1730b8	0
CS.sr, 15	main.body	ENDBODY	1730b8	0
CS.sr, 7	CS.arbitrator	IN	1731c0	0
CS.sr, 20	main.user	PROC	173168	1730b8
CS.sr, 22	main.user	CALL	173168	0
CS.sr, 8	CS.arbitrator	ARM	1731c0	173168
CS.sr, 7	CS.arbitrator	NI	1731c0	173168
CS.sr, 10	CS.arbitrator	IN	1731c0	0
CS.sr, 24	main.user	SEND	173168	0
CS.sr, 10	CS.arbitrator	ARM	1731c0	173168
CS.sr, 10	CS.arbitrator	NI	1731c0	173168
CS.sr, 7	CS.arbitrator	IN	1731c0	0
CS.sr, 20	main.user	ENDPROC	173168	1730b8
.fi
.LP
.SH SEE ALSO
.LP
sr(1), srprof(1)
.SH CAVEATS
.I srtrace
output reflects the actual SR implementation, which differs in
some details from the SR source language. For example, a \fIprocess\fP
statement is traced as a SEND followed by a PROC. Some \fIP\fP
and \fIV\fP statements are implemented as \fIin\fP and \fIsend\fP
respectively, and vice versa.
.LP
The following 
.I srtrace 
events report a line number different from the line number of the 
corresponding SR statement:
.LP
.nf
.ta 15n
NI	line number of corresponding IN
ENDPROC	line number of corresponding PROC
ENDBODY	line number of corresponding BODY
ENDFINAL	line number of corresponding FINAL
