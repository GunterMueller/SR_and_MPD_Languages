.TH SRWIN 3 "7 October 1994" "University of Arizona" "SR Library"
.SH NAME
srwin \- X-Windows graphics interface for SR
.SH SYNOPSIS
\fI(in SR program:)\fP\h`|18n`\fBimport SRWin\fP
.br
\fI(to build:)\fP\h`|18n`\fBsr \fRprogram.sr\fP srwin.o \-lX11\fP
.br
\fI(with srm:)\fP\h`|18n`\fBsrm \fRprogram.sr\fP srwin.o \-lX11\fP
.SH DESCRIPTION
.ds b \h'-.25i'\z\(bu\h'+.25i'
.ds z \h'-.25i'
.de NP
.sp .4v
..
.de HQ
.IP "" 3n
..
SRWin provides a set of window interface operations to the SR programmer.
It is based on the Xlib interface to the X window system.
The application imports a global \fBSRWin\fP and links with a file of
associated C functions and the X library.
The library is often found simply by linking \fB\-lX11\fP,
but some systems may require a different specification.
.NP
The package supports five basic types of objects: windows, events, fonts,
cursors, and images.
Windows are primary objects for input and output.
Cursors and fonts are resources of the underlying X Window System,
which define the visible shape of the pointing device and the
appearance of textual output respectively.
Window events are queued in the order of their occurrence and
handled later either by the SRWin package or by the application program.
.IP Windows
.NP
Windows can be displayed on a X server or they can be off-screen pixmaps.
To the programmer they are obscured data structures.
They reside in the X server's memory, which means drawings on windows must
be packed as requests and sent to the X server (probably through the network).
A window cannot be shared among processes on different virtual machines.
.NP
A backup copy of each window's contents is maintained in the package
for automatic refreshing when needed.
A window can be treated as an abstract drawing surface
without regard to overlap or visibility considerations.
.NP
Usually a graphics primitive does not contain all the information
needed to draw a particular thing.
The X server maintains resources called \fIgraphics contexts (GCs)\fP
that specify many attributes that apply to each graphic request.
GCs reduce the traffic between the X server and the application programs
(\fIclients\fP of the X server) in two ways.
First, the GC information is maintained by the X server and only needs
to be sent once, and later drawing requests use the id of the GC to draw.
In the case of changes, only the selected few fields need to be sent.
Second, multiple GCs can be created so clients can
switch among different graphic bindings with ease.
.NP
SRWin treats GCs as parts of a window.
The application program creates
a new GC binding by calling \fBWinNewContext\fP() to get a new window
structure with the new GC, while it still refers to the old window.
.NP
A window can have many \fIsubwindows\fP as its children.
All children are displayed on top of their parents when mapped (which means
``ready for display'').
When the extent of a window intersects with that
of a sibling window, these two windows can be displayed in any order.
SRWin provides a few primitives for managing subwindows.
.IP Events
.NP
All inputs from a window are modeled as events.
Events that require a part of the screen to be redrawn
are handled automatically by the SRWin package.
Other events can be processed by the application program.
Unlike most other X Window packages, with which application programs play a
passive role responding to events, SRWin lets an application program
manage the control flow directly.
.NP
To receive events on a window, the application program selects
events of interest by specifying an event mask.
When an event occurs, it is checked with the window's mask;
if the particular event not selected, it is propagated
to the parent window, and so on, until it reaches a window
for which the event is selected or ignored
(in which case the event is discarded).
.br
.ne 5
.IP Images
.NP
Images are rectangular areas of pixels; unlike windows, they reside on
the client instead of the server, and they are always invisible.
Images support only a limited set of operations, but image modification
operations can be accomplished very quickly because they require no
communication with the server.
The typical use is to constuct an image locally, possibly a pixel at a time,
and then copy it to the screen as a unit.
.IP "Fonts and Cursors"
.NP
A cursor is a transient shape that moves on the screen as the mouse moves
on its pad to indicate where the mouse is pointing.
Different windows may have different cursor shapes.
.NP
A font is a set of bitmaps
representing characters, cursor shapes, or other small patterns.
In SRWin, fonts and cursors come from a predefined set
supported by the underlying X Window System.
.SH OUTPUT MODEL
.LP
All window output operations are defined in terms of an integer
coordinate system with its origin at the top-left corner of the window.
.LP
Lines, ellipses, texts, and individual pixels can be drawn.
Multiple characteristics of drawing and filling can be controlled.
These include line width, line style, cap style, join style,
dashes and dash offset, fill style and fill rule.
.LP
To further reduce the communication traffic between the X server and
the application program, drawing can be temporarily disabled (on a per
top-level window basis).
Then the client must explicitly tell the X server to update the window contents.
Alternatively the application can choose to construct an image
and send the image as a whole to the X server for display.
.SH INPUT MODEL
.LP
All input accepted by the package is represented by a uniform event
record data structure, which includes information about:
.RS
\*btype of the event
.br
\*bwindow in which the event occurred
.br
\*bcoordinates of the pointer within the window when the event occurred
.br
\*bthe status of the mouse buttons (up or down)
.br
\*bthe status of the SHIFT, CONTROL, and META keys
.br
\*bspecific values associated with the particular event type (e.g.
button number when it is a button press or release event; ASCII code of
keypress; etc.)
.RE
.LP
Each event from a window is dispatched to the event-message channel of
that window.
This channel is provided by the user program when it calls
SRWin to create a window.
If the user program doesn't provide an event
channel, the window can be used only for output.
.LP
The user program can determine whether there is event pending by
checking the number of messages in the event channel, or it can wait on
the event channel until an event happens.
The application can specify the event types of interest
and have all other events discarded automatically.
A process is generated for each window to poll for events.
.SH "EVENT CHANNEL"
.LP
The
.I SRWin
global exports
.br
	\fBoptype\fP \fIwinEventChannel\fP(\fIwinEvent\fP) \fB{send}\fP
.br
The application program can use this optype to declare a message channel.
.LP
When a selected event occurs, SRWin sends a \fIwinEvent\fP record
to the registered input channel for the window.
The \fIwinEvent\fP record contains the following fields:
.IP \fIevent_type\fP 16n
Type of the event.
.IP \fIwindow\fP
The window in which the event occurred.
.IP "\fIx, y\fP"
Coordinates of the pointer within the window when the event occurred.
.IP \fIbk_status\fP
Inclusive OR of flags indicating the currently pressed buttons and keys:
.br
.ft I
.nf
.ta .5i 2i 3.5i
	BK_None	BK_Button1	BK_Mod1
	BK_CNTRL	BK_Button2	BK_Mod2
	BK_LOCK	BK_Button3	BK_Mod3
	BK_SHIFT	BK_Button4	BK_Mod4
		BK_Button5	BK_Mod5
.fi
.ft P
.IP \fIdata\fP
For a key event, \fIdata\fP specifies which key or button was pressed
or released, and it can be converted to the corresponding character.
For a mouse button event, \fIdata\fP is one of the button masks listed above.
For a enter/leave window event, \fIdata\fP can be converted to
a boolean value indicating whether the window has the focus or not.
For all other events, this field is undefined.
.IP \fIkeysym\fP
Numerical value of standard X KeySym as defined in C header file
\fI<X11/keysymdef.h>\fP and the KeySym database
\fI/usr/lib/X11/XKeysymDB\fP. It is useful for detecting keys that do
not have corresponding ASCII character representations, such as function
keys and arrow keys.
.SH OTHER DATA STRUCTURES
.IP \fIwinWindow\fP 16n
Pointer to a record structure that holds all information for a window.
.IP \fIwinInitialState\fP
Integer value specifying the initial state of an object when creating it.
.IP \fIwinError\fP
Integer value that is zero in the case of an error or nonzero
if successful.
.IP \fIwinStdCursor\fP
An enumeration of the set of defined cursor shapes.
.IP \fIwinCursor\fP
Pointer representing the handle of a cursor.
.IP \fIwinColor\fP
String containing a color name or a numerical color specification.
.IP \fIwinPixel\fP
Pointer representing the handle of a colormap entry.
.IP \fIwinFont\fP
Pointer to a structure that holds information for a loaded font.
.IP \fIwinImage\fP
Pointer to an image structure.
.IP \fIwinPoint\fP
Record of (\fIx\fP, \fIy\fP) coordinates.
.IP \fIwinRectangle\fP
Record of (\fIx\fP, \fIy\fP, \fIw\fP, \fIh\fP) for the coordinates
of the top-left corner of the rectangle and its width and height.
.IP \fIwinLineStyle\fP
Enumeration of valid line styles:
\fILineSolid, LineDoubleDash, LineOnOffDash\fP
.IP \fIwinCapStyle\fP
Enumeration of valid cap styles:
\fICapNotLast, CapButt, CapRound, CapProjecting\fP
.IP \fIwinJoinStyle\fP
Enumeration of valid join styles:
\fIJoinMiter, JoinRound, JoinBevel\fP
.IP \fIwinFillStyle\fP
Enumeration of valid fill styles:
\fIFillSolid, FillTiled, FillOpaqueStippled, FillStippled\fP
.IP \fIwinFillRule\fP
Enumeration of valid fill rules:
\fIFillEvenOddRule, FillWindingRule\fP
.IP \fIwinArcMode\fP
Enumeration of valid arc modes:
\fIArcChord, ArcPieSlice\fP
.IP \fIwinDrawOp\fP
Enumeration of valid drawing operations:
these control how the source pixel values generated by a graphics request
are combined with the old destination pixel values already on the screen
to produce the final destination pixel values.
The operations are:
.br
.ft I
.if t .ta 1.5i 2.7i 3.9i
.if n .ta 18n 36n 54n
Op_Clear	Op_And	Op_AndReverse	Op_Copy
Op_AndInverted	Op_Noop	Op_Xor	Op_Or
Op_Nor	Op_Equiv	Op_Invert	Op_OrReverse
Op_CopyInverted	Op_OrInverted	Op_Nand	Op_Set
.ft P
.SH NAMING AND ARGUMENT CONVENTIONS
.LP
SRWin follows a set of conventions for the naming and syntax of the functions:
.RS
\*bThe names of all SRWin functions begin with \fIWin\fP followed by
compound words which are constructed by capitalizing the first letter
of each word.
.br
\*bNames of user-visible data structures and types begin with
\fIwin\fP.
Names of all members of data structures use lower case.
.br
\*bThe window argument, where used, is always first in the argument
list.
The image argument, where used, is always right after the window
argument when there is one, or the first when there is no window argument.
.br
\*bSource arguments always precede destination arguments in an
argument list.
.br
\*bAn \fIx\fP argument always precedes a \fIy\fP argument in an argument list.
.br
\*bA \fIwidth\fP argument always precedes a \fIheight\fP argument in an
argument list.
.br
\*bIf \fIx\fP, \fIy\fP, \fIwidth\fP, and \fIheight\fP arguments are
used together, the \fIx\fP and \fIy\fP arguments always precede the
\fIwidth\fP and \fIheight\fP arguments.
.br
\*bIf a procedure returns an integer, a value of zero serves as an
error indicator.
If a procedure returns a pointer, \fBnull\fP indicates an error.
Not all errors are reported in this manner;
some (especially those that cannot be detected immediately)
abort the program.
.RE
.SH FUNCTIONS
.NP
.B
\*zGeneral Functions
.NP
.HP
.B WinOpen
(display: string[*]; title: string[*]; evchannel: cap winEventChannel;
state: winInitialState; w, h: int) returns win: winWindow
.PD 0
.HP
.B WinCreateSubwindow
(oldwin: winWindow; evchannel: cap winEventChannel; state:
winInitialState; x, y, w, h: int) returns newwin: winWindow
.PD
.HQ
\fBWinOpen\fP() opens and initializes a top-level window of width
\fIw\fP pixels, height \fIh\fP pixels, and with the same depth of
the root window, with white foreground and black background.
If \fBWinOpen\fP() can't open such a window, a \fBnull\fP pointer is
returned.
\fBWinCreateSubwindow\fP() creates a subwindow as
\fIoldwin\fP's child.
The subwindow begins at \fI(x, y)\fP relative to its parent's top-left corner.
.NP
The initialization includes opening a connection to the X server,
creating a window, creating a backing store, allocating a graphics
context and a colormap, loading default font, setting default window
attributes, etc.
If the \fIdisplay\fP argument is a null string, SRWin
then tries to open that window on the screen specified
by the environment variable \fBDISPLAY\fP.
.NP
When \fIstate\fP equals to \fBUseDefault\fP, the created window is
displayed on screen (at a position determined by the window manager for
the top-level window case), and output to the window is enabled.
If \fIstate\fP equals to \fBOffScreen\fP, the window is off screen and can
be made visible by calling \fBWinMapWindow\fP(), while direct output is
initially disabled.
.NP
The \fIevchannel\fP argument is used to register a message channel to
receive incoming window events.
It can be \fBnull\fP if no event reporting is wanted.
If the window is on screen at the beginning and \fIevchannel\fP is not
\fBnull\fP, then all events are selected on this window.
.NP
For a subwindow, the graphics context information is inherited from its
parent, but in a different GC.
.HP
.B WinDestroyWindow
(win: winWindow)
.HQ
Destroys a window and all its subwindows, freeing contexts.
This operation has no effect on a top-level window.
.HP
.B WinClose
(win: winWindow)
.HQ
Destroys a top-level window and all its subwindows, frees the associated
X resources, and closes its connection to the X server.
.HP
.B WinNewContext
(oldwin: winWindow) returns newwin: winWindow
.HQ
Creates a new context window from an existing window.
The context window appears as a ``window'' and points to the original window
except it cannot generate any window events and it has a different
graphics context.
.HP
.B WinCopyContext
(srcwin, destwin: winWindow)
.HQ
Copies all information associated with \fIsrcwin\fP's graphics context to
that of \fIdestwin\fP's.
.HP
.B WinSetBorder
(win: winWindow; width: int; color: winColor)
.HQ
Sets the window border \fIwidth\fP and paints it using \fIcolor\fP.
The border is not included when creating a window.
.HP
.B WinSetLabels
(win: winWindow; winlab, iconlab: string[*])
.HQ
Sets the window and icon labels.
.br
.ne 6
.HP
.B WinMapWindow
(win: winWindow)
.PD 0
.HP
.B WinMapSubwindows
(win: winWindow)
.HP
.B WinUnmapWindow
(win: winWindow)
.HP
.B WinUnmapSubwindows
(win: winWindow)
.PD
.HQ
Maps or unmaps a window and/or all of its subwindows.
Mapping a window onto the screen makes it and its subwindows visible;
unmapping a window makes it and its subwindows invisible.
Output to an unmapped window is allowed; when the window is remapped,
its contents reflect such output.
.HP
.B WinMoveWindow
(win: winWindow; pt: winPoint)
.HQ
Moves the window to the given location relative to its parent.
.HP
.B WinEnableOutput
(win: winWindow)
.PD 0
.HP
.B WinDisableOutput
(win: winWindow)
.HP
.B WinUpdateWindow
(win: winWindow)
.PD
.HQ
Normally, output to an on a on-screen window is directed simultaneously
to the window and to its backing pixmap.  This can be disabled for
performance reasons so that the output goes only to the pixmap;
the window is then updated from the pixmap when
.BR WinUpdateWindow ()
is called.
.HP
.B WinFlush
(win: winWindow)
.HQ
Flushes all pending output for a window and its subwindows.
.HP
.B WinSync
(win: winWindow; discard: bool)
.HQ
Flushes the output buffer and waits for all requests to be received
and processed by the X server.
If \fIdiscard\fP is \fBtrue\fP, all pending
window events not recognized by SRWin are discarded.
.HP
.B WinBell
(win: winWindow; percent: int)
.HQ
Rings the bell on the specified window, if possible.
Volume is specified by the percentage relative to the base volume set by
\fBxset\fP(1).
\fIPercent\fP can be in the range \-100 to 100 inclusive.
If it is positive, the sound is louder than the base
volume; if it is negative, the sound is quieter.
.NP
.LP
.B
\*zCursors and Fonts
.NP
.HP
.B WinCreateCursor
(win: winWindow; stdcursor: winStdCursor) returns cur: winCursor
.HQ
Creates a standard cursor.
Valid cursors are:
.NP
.ft I
.if t .ta 1.5i 3i 4.5i
.if n .ta 18n 36n 54n
XC_X_cursor	XC_arrow	XC_based_arrow_down	XC_based_arrow_up
XC_boat	XC_bogosity	XC_bottom_left_corner	XC_bottom_right_corner
XC_bottom_side	XC_bottom_tee	XC_box_spiral	XC_center_ptr
XC_circle	XC_clock	XC_coffee_mug	XC_cross
XC_cross_reverse	XC_crosshair	XC_diamond_cross	XC_dot
XC_dotbox	XC_double_arrow	XC_draft_large	XC_draft_small
XC_draped_box	XC_exchange	XC_fleur	XC_gobbler
XC_gumby	XC_hand1	XC_hand2	XC_heart
XC_icon	XC_iron_cross	XC_left_ptr	XC_left_side
XC_left_tee	XC_leftbutton	XC_ll_angle	XC_lr_angle
XC_man	XC_middlebutton	XC_mouse	XC_pencil
XC_pirate	XC_plus	XC_question_arrow	XC_right_ptr
XC_right_side	XC_right_tee	XC_rightbutton	XC_rtl_logo
XC_sailboat	XC_sb_down_arrow	XC_sb_h_double_arrow	XC_sb_left_arrow
XC_sb_right_arrow	XC_sb_up_arrow	XC_sb_v_double_arrow	XC_shuttle
XC_sizing	XC_spider	XC_spraycan	XC_star
XC_target	XC_tcross	XC_top_left_arrow	XC_top_left_corner
XC_top_right_corner	XC_top_side	XC_top_tee	XC_trek
XC_ul_angle	XC_umbrella	XC_ur_angle	XC_watch
XC_xterm	XC_None
.ft P
.HP
.B WinSetCursor
(win: winWindow; cursor: winCursor; fg, bg: winColor) returns c: winCursor
.HQ
Sets the cursor of the specified window, returning \fBnull\fP if unsuccessful.
The cursor colors are set to \fIfg\fP (foreground) and \fIbg\fP (background).
.HP
.B WinFreeCursor
(win: winWindow; cursor: winCursor)
.HQ
Frees a cursor and reclaims any associated resources.
.HP
.B WinDefaultFont
(win: winWindow) returns font: winFont
.HQ
Returns the default font of the graphics context.
.HP
.B WinLoadFont
(win: winWindow; fontname: string[*]) returns font: winFont
.HQ
Loads a font by name.
.HP
.B WinSetFont
(win: winWindow; font: winFont)
.HQ
Sets the font for the specified window.
.HP
.B WinFreeFont
(win: winWindow; font: winFont)
.HQ
Frees a font and reclaims any associated resources.
The default font cannot be freed.
.NP
.LP
.B
\*zClipping Manipulation Function
.NP
.HP
.B WinSetClipRectangles
(win: winWindow; origin: winPoint; rects[*]: winRectangle)
.HQ
Sets the clipping region for a context window (clip rectangles are stored
on a per-context basis).
Subsequent output is clipped to be contained within the specified
nonintersecting rectangles.
The parameter \fIorigin\fP is relative to the origin of the window,
and the rectangle coordinates are relative to the \fIclip origin\fP.
.NP
.LP
.B
\*zDrawing Functions
.NP
.HP
.B WinClearArea
(win: winWindow; area: winRectangle)
.HQ
Clears a rectangular region using the \fIwindow\fP background color
(which can differ from the current value set by \fBWinSetBackground\fP()).
The clipping attributes of the context window are ignored.
.HP
.B WinEraseArea
(win: winWindow; area: winRectangle)
.HQ
Clears a rectangular area to the current graphics context background color,
which is set using \fBWinSetBackground\fP().
.HP
.B WinCopyArea
(srcw, destw: winWindow; src_rect: winRectangle; destp: winPoint)
.HQ
Copies a rectangular region between (potentially) two windows on the same
physical screen.
.HP
.B WinDrawArc
(win: winWindow; box: winRectangle; a1, a2: int)
.PD 0
.HP
.B WinFillArc
(win: winWindow; box: winRectangle; a1, a2: int)
.PD
.HQ
Draws a (filled) arc, ellipse, or circle.
The center of the circle or ellipse is the center of the rectangle.
The major and minor axes are given by
the width and height of the rectangle.
The two angles are in units of degrees.
The first angle specifies the start of the arc;
the second specifies the path and extent of the arc,
with positive values indicating a counterclockwise direction.
.HP
.B WinDrawLine
(win: winWindow; pt1, pt2: winPoint)
.PD 0
.HP
.B WinDrawPolyline
(win: winWindow; pts[*]: winPoint)
.HP
.B WinDrawPolygon
(win: winWindow; pts[*]: winPoint)
.HP
.B WinFillPolygon
(win: winWindow; pts[*]: winPoint)
.PD
.HQ
Draws a (filled) line, polyline, or polygon.
.HP
.B WinDrawPixel
(win: winWindow; pt: winPoint)
.HQ
Draws a pixel.
.HP
.B WinDrawRectangle
(win: winWindow; rect: winRectangle)
.PD 0
.HP
.B WinFillRectangle
(win: winWindow; rect: winRectangle)
.PD
.HQ
Draws a (filled) box.
.HP
.B WinDrawString
(win: winWindow; pt: winPoint; str: string[*])
.PD 0
.HP
.B WinDrawImageString
(win: winWindow; pt: winPoint; str: string[*])
.PD
.HQ
Draws a string.
.BR WinDrawString ()
alters only the pixels forming the characters of the text;
.BR WinDrawImageString ()
clears the ``extent'' of the text to the background color.
.HP
.B WinTextWidth
(font: winFont; str: string[*]) returns width: int
.HQ
Computes the pixel width of a string in that font.
.HP
.B WinFontAscent
(font: winFont) returns ascent: int
.PD 0
.HP
.B WinFontDescent
(font: winFont) returns descent: int
.PD
.HQ
Returns the ascent or descent of a font.
.NP
.LP
.B
\*zDrawing Attributes Manipulation Functions
.NP
.HP
.B WinSetLineAttr
(win: winWindow; line_width: int; l: winLineStyle;
c: winCapStyle; j: winJoinStyle)
.PD 0
.HP
.B WinSetFillAttr
(win: winWindow; fill_style: winFillStyle; fill_rule: winFillRule)
.HP
.B WinSetDashes
(win: winWindow; dash_offset: int; dash_list: string[*])
.HP
.B WinSetArcMode
(win: winWindow; arc_mode: winArcMode)
.HP
.B WinSetDrawOp
(win: winWindow; dop: winDrawOp)
.PD
.HQ
Sets the line drawing characteristics of a window.
Constants include:
.RS
.nf
.ta 15n
winLineStyle:	\fILineSolid, LineDoubleDash, LineOnOffDash\fP
winCapStyle:	\fICapNotLast, CapButt, CapRound, CapProjecting\fP
winJoinStyle:	\fIJoinMiter, JoinRound, JoinBevel\fP
winFillStyle:	\fIFillSolid, FillTiled, FillOpaqueStippled, FillStippled\fP
winFillRule:	\fIFillEvenOddRule, FillWindingRule\fP
winArcMode:	\fIArcChord, ArcPieSlice\fP
winDrawOp:
.fi
.RS
.ft I
Op_Clear, Op_And, Op_AndReverse, Op_Copy, Op_AndInverted,
Op_Noop, Op_Xor, Op_Or, Op_Nor, Op_Equiv, Op_Invert, Op_OrReverse,
Op_CopyInverted, Op_OrInverted, Op_Nand, Op_Set
.ft P
.RE
.RE
.IP
The defaults are line width 0, \fILineSolid\fP, \fICapButt\fP,
\fIJoinMiter\fP, \fIFillSolid\fP, \fIFillEvenOddRule\fP,
\fIArcPieSlice\fP, \fIOp_Copy\fP, and no dashes.
.IP
Using a line width other than zero may degrade performance
on some X servers.
.IP
Drawing operations other than \fIOp_Copy\fP are potentially nonportable
or even undefined and should be used only with a clear understanding
of the X color model.
For example, \fIOp_Xor\fP gives different results (other things being equal)
on Sun and DEC hardware.
.HP
.B WinSetForeground
(win: winWindow; foreground: winColor) returns pv: winPixel
.PD 0
.HP
.B WinSetBackground
(win: winWindow; background: winColor) returns pv: winPixel
.PD 0
.HP
.B WinSetForegroundByPixel
(win: winWindow; foreground: winPixel)
.PD 0
.HP
.B WinSetBackgroundByPixel
(win: winWindow; background: winPixel)
.PD
.HQ
Sets the foreground or background color to be used in subsequent
drawing operations.
.NP
.LP
.B
\*zEvent Handling Functions
.NP
.HP
.B WinSetPoll
(win: winWindow; msec: int)
.HQ
Sets the interval between event checks, in milliseconds.
The default interval is 100 milliseconds.
.HP
.B WinSetEventMask
(win: winWindow; em: int)
.HQ
Registers events of interest for a window.
.NP
Valid event masks are the same as the event types.
They can be
\fIor\fP'ed together to set multiple masks for a window.
The default event masks include all supported event types if an event channel is
provided at the window creation time, or nothing if no channel is
provided and the window is requested to be mapped.
.NP
There are several defined events masks:
.RS
.ft I
.nf
.ta .5i 2i 3.5i
	Ev_KeyDown	Ev_EnterWindow	Ev_All
	Ev_KeyUp	Ev_PointerMove	Ev_None
	Ev_ButtonDown	Ev_ExitWindow
	Ev_ButtonUp	Ev_DeleteWindow
.fi
.ft P
.RE
.IP
\fIEv_DeleteWindow\fP is sent whenever the window manager
issues a DELETE_WINDOW message (e.g. the user chooses \fIQuit\fP from
window manager's menu); this event can only be received in the top-level window.
\fIEv_All\fP is the combination of all possible events;
\fIEv_None\fP selects no events.
.NP
.LP
.B
\*zImage Manipulation Functions
.NP
.HP
.B WinCreateImage
(win: winWindow; depth, w, h: int) returns im: winImage
.HQ
Creates an image with width \fIw\fP and height \fIh\fP.
If \fBUseDefault\fP is passed as \fIdepth\fP, the
depth of the image is set to be the same as that of the physical display.
.HP
.B WinDestroyImage
(im: winImage)
.HQ
Destroys the image and frees the memory space it occupies.
.HP
.B WinGetPixel
(im: winImage; pt: winPoint) returns pv: winPixel
.PD 0
.HP
.B WinPutPixel
(im: winImage; pt: winPoint; pv: winPixel)
.PD
.HQ
Reads or writes a pixel value from or to the image.
The point must be inside the image.
These two functions are not protected by mutual exclusion;
the application program should be aware of potential
consistency problems when an image is shared among multiple processes.
\fBWinPutPixel\fP() is most reliable when different processes work on
different rows (and therefore different memory words).
.HP
.B WinAddPixel
(im: winImage; pv: winPixel)
.HQ
Increments each pixel in the image by the value of \fIpv\fP.
This function is not protected by mutual exclusion.
.HP
.B WinGetImage
(win: winWindow; im: winImage; src_rect: winRectangle; dest: winPoint)
.PD 0
.HP
.B WinPutImage
(win: winWindow; im: winImage; src_rect: winRectangle; dest: winPoint)
.PD
.HQ
Copies the rectangular area specified by \fIsrc_rect\fP on the window to
the image starting at point specified by \fIdest\fP, or copies
an image to a window.
The depth of the image and the window must match.
.NP
.SH SEE ALSO
.LP
sr(1), srl(1)
.LP
Qiang Alex Zhao,
.I "SRWin: A Graphics Library for SR."
TR 93-14, Dept. of Computer Science, The University of Arizona, 1993.
Included in the SR distribution.
.LP
Adrian Nye,
.I "Xlib Programming Manual, Volume One, 3rd ed."
O'Reilly & Associates, Inc., 1992, ISBN 1-56592-002-3.
.LP
Adrian Nye,
.I "Xlib Reference Manual, Volume Two, 3rd ed."
O'Reilly & Associates, Inc., 1992, ISBN 1-56592-006-6.
.LP
Robert W. Scheifler and James Gettys,
.I "X Window System: The Complete Reference to Xlib, X Protocol, ICCCM, XLFD, 3rd ed."
Digital Press, 1992, ISBN 1-55558-088-2.
.SH CAVEATS
.LP
Numerous unexplained problems have been seen on the Sequent Symmetry.
.LP
Except on an SGI Iris, SRWin does not function properly if MultiSR is enabled.
.SH AUTHOR
Qiang A. Zhao.
.SH ACKNOWLEDGMENTS
SRWin was inspired by X-Icon, an X interface developed by Clint Jeffery
for the Icon language.
Many useful ideas came from the
.I Winpack
graphics library created by Scott Hudson.
