.TH SRANIMATOR 3 "7 October 1994" "Drexel University" "SR Library"
.SH NAME
sranimator \- XTANGO based animation package for SR
.SH SYNOPSIS
.ds XT \fIXTANGO\fP/xtango.o \-lXaw \-lXmu \-lXext \-lXt \-lX11
.nf
.if n .ta 2n
.if t .ta 5n
\fI(in SR program:)
	\fBimport SRanimator\fP
\fI(to build:)
	\fBsr \fRprogram.sr\fP sranimator.o \*(XT
\fI(with srm:)
	\fBsrm \fRprogram.sr\fP sranimator.o \*(XT
\fI(to run:)
	\fBxrdb \-merge \fIXTANGO\fB/xtango.res
	\fBa.out
.fi
.SH DESCRIPTION
.ds b \h'-.25i'\z\(bu\h'+.25i'
.ds z \h'-.25i'
.de HQ
.IP "" 3n
..
SRanimator provides a front-end to XTANGO,
a library of algorithm animation routines.
The SR application imports a global \fBSRanimator\fP and links with
SR, XTANGO, and X library files.
The library lists shown above assume that XTANGO was built
using the Athena widget set.
.SH "RUNNING AN ANIMATION"
.LP
Before running an XTANGO application you must run
\fBxrdb \-merge \fIXTANGO\fP/xtango.res\fR,
where \fBxtango.res\fP is the application default file from the
XTANGO distribution.
Alternative methods of initialization are discussed in the XTANGO
documentation.
.LP
To run an animation, compile and run the SR application
containing the procedure calls described below.
When the animation window appears, press the
\fBrun animation\fP button to begin the animation.
.LP
Animation parameters can be altered using other pushbuttons
either before or after the animation has been started.
The arrows and zoom pushbuttons on the left control the display, while
the buttons at the bottom and the scrollbar at the right
affect the animation behavior.
An animation may be aborted at any time by selecting the \fBquit\fP
pushbutton in the lower right-hand corner of the animation window.
.LP
.B
\*zPanning and Zooming
.LP
The pushbuttons along the left-hand edge of the animation screen affect
what portions of an animation are displayed while an animation is running.
To pan the animation window in a specific direction, select the
appropriate arrow pushbutton.
To zoom in on a section of the animation
select the \fBin\fP pushbutton;
select the \fBout\fP pushbutton to zoom back out.
.LP
.B
\*zPausing a Running Animation
.LP
After initially selecting the \fBrun animation\fP pushbutton to start the
animation running, the \fBrun animation\fP pushbutton changes its name to
\fBpause\fP.
An animation may be paused at any time by selecting the
\fBpause\fP pushbutton which then pauses the animation and changes its name
to \fBunpause\fP.
To continue the animation, select the \fBunpause\fP
pushbutton which then changes its name back to \fBpause\fP.
.LP
.ne 5v
.B
\*zChanging the Animation Refresh Mode
.LP
When using Motif and HP widgets,
XTANGO supports three different times to redraw all images
displayed in an animation:
between each frame, between each scene, and none.
The default refresh mode is between each \fIframe\fP.
(With Athena widgets, the mode setting has no effect.)
.LP
There is a direct trade-off between the frequency of redrawing and the
speed of the animation.
Refreshing \fBby frame\fP produces a smooth
animation but runs the slowest.
On the other hand, \fBno refresh\fP
produces a quick animation possibly with many holes in the images
(created when one image passes over another).
Try the \fBno refresh\fP
mode when initially building an animation.
Then switch to \fBby scene\fP
to iron out the minor bugs.
Finally use the \fBby frame\fP mode when an
animation is complete.
.LP
To change the refresh mode, select the \fBmode\fP pushbutton either
before running an animation or during the animation.
A menu will
appear containing the three choices and the current choice will be
indicated with a checkmark.
After selecting the appropriate choice,
the new refresh mode will take effect immediately.
.LP
Select the \fBrefresh\fP pushbutton located next to the \fBquit\fP
pushbutton to force a refresh at any time.
.LP
.B
\*zSlowing Down a Running Animation
.LP
Use the vertical scrollbar located along the right-hand edge of the
animation window to control the amount of time between animation frames.
The animation runs fastest when the scrollbar's \*(lqthumb\*(rq (the black,
movable rectangle) is positioned at the top of the scrollbar; the
animation becomes progressively slower as the thumb is moved towards
the bottom of the scrollbar.
.LP
To change the position of the thumb, either select the thumb and drag
it to a new location, select a location within the scrollbar (and the
thumb will jump there), or select one of the arrows located at the ends
of the scrollbar (and the thumb will step in the desired direction).
.SH "INTERFACE PROCEDURES"
.LP
Animations operate on an infinite plane having a real-valued coordinate system.
A rectangular portion of the plane,
initially the area having x and y values between 0.0 and 1.0,
is displayed on the screen.
Graphical objects can be created and placed within the coordinate
system, and then moved or altered
to depict the operations and actions of a computer algorithm.
The SR programmer specifies a unique integer \fIid\fP when creating
an object, then uses this \fIid\fP to designate the object in
subsequent operations.
.LP
The individual animation commands are described below.
Most commands and parameters should be self-explanatory.
Arguments named \fIsteps\fP, \fIcentered\fP,
and \fIsteptime\fP are of integer type.
Arguments named \fIxpos\fP, \fIypos\fP, \fIxsize\fP, \fIysize\fP, \fIradius\fP,
\fIlx\fP, \fIby\fP, \fIrx\fP, \fIty\fP, \fIry\fP are real
or floating point numbers.
The argument \fIfillval\fP should be one of the following strings:
\fBoutline\fP, \fBlight\fP, \fBhalf\fP, \fBheavy\fP, or \fBsolid\fP.
The argument \fIwidthval\fP should be one of the following strings:
\fBthin\fP, \fBmedthick\fP, or \fBthick\fP.
The argument \fIstyleval\fP should be one of the following strings:
\fBdotted\fP, \fBdashed\fP, or \fBsolid\fP.
The argument \fIarrows\fP should be one of the following strings:
\fBnone\fP, \fBforward\fP, \fBbackward\fP, or \fBbidirectional\fP.
The parameter \fIcolorval\fP can be any color specification acceptable to X.
Note that the
\fIcolor\fP command only supports a subset of all these possible colors:
\fBwhite\fP, \fBblack\fP, \fBred\fP, \fBorange\fP, \fByellow\fP,
\fBgreen\fP, \fBblue\fP, and \fBmaroon\fP.
The parameter \fIfontname\fP can be any font specification acceptable to X.
.LP
.B
\*zGeneral Procedures
.LP
.HP
.B A_bg
(colorval : string[*])
.HQ
Change the background to the given color.
The default background is white.
.HP
.B A_coords
(lx, by, rx, ty : real)
.HQ
Change the displayed coordinates
to the given values (left-bottom is (\fIlx\fP,\fIby\fP) and
right-top is (\fIrx\fP,\fIty\fP)).
You can use repeated applications of this command to pan or
zoom the animation view.
.HP
.B A_delay
(steps : int)
.HQ
Generate the given number of animation frames with
no changes in them.
.HP
.B A_zoom
(id : int; rx, ry: real)
.HQ
Zoom in to (positive values of \fIrx\fP and \fIry\fP)
or out from (negative values) the object given by \fIid\fP.
Values close to 0.0 zoom a little, absolute values close to 1.0 zoom a lot.
.HP
.B A_end
()
.HQ
Terminate the animation.
.LP
.LP
.B
\*zDrawing Procedures
.LP
.HP
.B A_line
(id : int; xpos, ypos, xsize, ysize : real; colorval, widthval, styleval, arrows : string[*])
.HQ
Create a line with one endpoint at the given
position and of the given size.
The line can be dotted, dashed, or solid and can optionally have arrows on
either or both ends.
Note that lines are moved
(move, jump, and exchange commands) relative to their centers.
.HP
.B A_rectangle
(id : int; xpos, ypos, xsize, ysize : real; colorval, fillval : string[*])
.HQ
Create
a rectangle with lower left corner at the
given position and of the given size
(size must be positive).
.HP
.B A_circle
(id : int; xpos, ypos, radius : real; colorval, fillval : string[*])
.HQ
Create
a circle centered at the given position.
.HP
.B A_triangle
(id : int; v1x, v1y, v2x, v2y, v3x, v3y : real; colorval, fillval : string[*])
.HQ
Create
a triangle whose three vertices are located
at the given three coordinates.
Note that triangles are moved
(move, jump, and exchange commands) relative to the centers of their
bounding boxes.
.HP
.B A_text
(id : int; xpos, ypos : real; centered : int; colorval, str : string[*])
.HQ
Create text \fIstr\fP with lower left corner at the given
position if \fIcentered\fP is 0.
If \fIcentered\fP is 1, the position
arguments denote the place where the center of the text is put.
The text string is allowed to have blank spaces included in it but you
should make sure it includes at least one non-blank character.
.HP
.B A_bigtext
(id : int; xpos, ypos : real; centered : int; colorval, str : string[*])
.HQ
This works
just like the text command except that
this text is in a much larger font.
.HP
.B A_fonttext
(id : int; xpos, ypos : real; centered : int; colorval, fontname, str : string[*])
.HQ
This works
just like the text command except that
this text is in the specified font.
.LP
.LP
.B
\*zImage Manipulation Procedures
.LP
.HP
.B A_move
(id : int; xpos, ypos : real)
.HQ
Smoothly move,
via a sequence of intermediate steps,
the object with the given id to the specified position.
.HP
.B A_moverelative
(id : int; xdelta, ydelta : real)
.HQ
Smoothly move,
via a sequence of intermediate steps,
the object with the given id by the given relative distance.
.HP
.B A_moveto
(id1, id2 : int)
.HQ
Smoothly move,
via a sequence of intermediate steps,
the object with the first id to the current position of the object
with the second id.
.HP
.B A_jump
(id : int; xpos, ypos : real)
.HQ
Move the object with the given id
to the designated position in a one frame jump.
.HP
.B A_jumprelative
(id : int; xdelta, ydelta : real)
.HQ
Move the object with the given id
by the provided relative distance in one jump.
.HP
.B A_jumpto
(id1, id2 : int)
.HQ
Move the object with the given id to the current
position of the object with the second id in a one frame jump.
.HP
.B A_stepjump
(id : int; xpos, ypos : real; steps, steptime : int)
.HQ
Move the object with the
given id to the designated position in a multiple frame jump.
The steps of the jump are done at the specified millisecond intervals.
.HP
.B A_stepjumpto
(id1, id2 : int; steps, steptime : int)
.HQ
Move the object with the
given id to the current
position of the object with the second id in a multiple frame jump.
The steps of the jump are done at the specified millisecond intervals.
.HP
.B A_color
(id : int; colorval : string[*])
.HQ
Change the color of the object
with the given id to the specified color value.
Only the colors white, black, red, green,
blue, orange, maroon, and yellow are valid for this command.
.HP
.B A_delete
(id : int)
.HQ
Permanently remove the object with the given id from
the display, and remove any association of this id number with the object.
.HP
.B A_fill
(id : int; fillval : string[*])
.HQ
Change the object with the given id to the
designated fill value.
This has no effect on lines and text.
.HP
.B A_vis
(id : int)
.HQ
Toggle the visibility of the object with the given id.
.HP
.B A_lower
(id : int)
.HQ
Push the object with the given id backward to the viewing
plane farthest from the viewer.
.HP
.B A_raise
(id : int)
.HQ
Pop the object with the given id forward to the
viewing plane closest to the viewer.
.HP
.B A_exchangepos
(id1, id2 : int)
.HQ
Make the two objects specified by the given ids
smoothly exchange positions.
.HP
.B A_switchpos
(id1, id2 : int)
.HQ
Make the two objects specified by the given ids
exchange positions in one instantaneous jump.
.HP
.B A_swapid
(id1, id2 : int)
.HQ
Exchange the ids used to designate the two given
objects.
.HP
.B A_resize
(id : int; rx, ry: real)
.HQ
The circle, line, rectangle, or triangle is resized as follows.
The radius of a circle has \fIrx\fP added to it,
the endpoint of a line has (\fIrx\fP,\fIry\fP) added to it,
the lower-right corner of a rectangle is dragged by amount (\fIrx\fP,\fIry\fP),
and the first vertex of a triangle is dragged by amount (\fIrx\fP,\fIry\fP).
.SH FILES
.LP
.ta 27n
SRanimator.sr	SR animator global resource
.br
animator.o	compiled C language animator commands
.br
xtango.o	compiled C language XTANGO library
.br
xtango.res	XTANGO widget resources for X11 resources database
.SH SEE ALSO
.LP
sr(1), srl(1)
.LP
Stephen J. Hartley,
\fIAnimating Operating Systems Algorithms with XTANGO.\fP
ACM SIGCSE Bulletin 26, 1 (March, 1994).
.LP
Stephen J. Hartley,
\fIIntegrating XTANGO's Animator into the SR Concurrent Programming Language.\fP
Submitted for publication, 1994;
included in the SR distribution.
.LP
John T. Stasko,
\fITango: A Framework and System for Algorithm Animation.\fP
IEEE Computer 23, 9 (September, 1990), 27-39.
.LP
Doug Hayes,
\fIThe XTANGO Environment and Differences from TANGO.\fP
John T. Stasko and Doug Hayes,
\fIXTANGO Algorithm Animation Designer's Package.\fP
These two papers are provided as part of the XTANGO package.
.SH CAVEATS
.LP
Bracketing blocks of animation code with
\fIsetpriority(1)\fP and \fIsetpriority(0)\fP
may improve the animation.
.LP
The XTANGO package must be obtained and built separately
in order to use the SR animator. 
XTANGO is available by anonymous FTP from \fBpar.cc.gatech.edu\fP.
The SR library must be built (or rebuilt) after in\%stalling XTANGO.
.SH AUTHOR
.LP
Stephen J. Hartley
.SH ACKNOWLEDGMENTS
.LP
SRanimator was inspired by SRWin, written by Qiang A. Zhao,
and by the \fBanimator\fP interpreter program,
included with XTANGO, written by John T. Stasko
and Doug Hayes.
