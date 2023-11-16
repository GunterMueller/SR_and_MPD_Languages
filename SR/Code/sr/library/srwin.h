/*
 * srwin_c.h
 *
 * C header file for SRWin (Window/Graphics Package for SR)
 *
 * Qiang A. Zhao
 * Department of Computer Science
 * University of Arizona
 *
 * Jan 1993
 */

/* header files */

#include	<stdio.h>

/* X related header files */

#include	<X11/Xlib.h>
#include	<X11/Xutil.h>
#include	<X11/Xatom.h>
#include	<X11/cursorfont.h>
#include	<X11/keysym.h>
#include	<X11/Xos.h>

#ifndef	X_NOT_STDC_ENV
#include	<stdlib.h>
#else
char           *malloc (), *realloc (), *calloc ();
#endif
#if	defined(macII) && !defined(__STDC__)
char           *malloc (), *realloc (), *calloc ();
#endif	/* macII */

/*
 * mirrors from "sr.h" -- KEEP CONSISTENT WITH SR !!!
 *
 * Don't want to include "srmulti.h" and "sr.h" -- collide with X names
 */

typedef int	Int;
typedef char   *Ptr;
typedef unsigned short Vcap;	/* virtual machine capability */
typedef struct {
    Vcap	vm;
    short	seqn;
    Ptr		oper_entry;
} Ocap;				/* operation capability */

/*
 * constants
 *
 * [Note] the order of the items in each constant-lists must be exactly
 *        the same as that of their counterparts in "SRWin.sr"
 */

#define		ALL_EVENTS	(~(0L))
#define         DONTPROPAGATE   (KeyPressMask | KeyReleaseMask |	\
	ButtonPressMask | ButtonReleaseMask | PointerMotionMask |	\
	ButtonMotionMask | Button1MotionMask | Button2MotionMask |	\
	Button3MotionMask | Button4MotionMask | Button5MotionMask)
#define		ALWAYS_MASKS	(ExposureMask)

static int      LineStyles[] = {
    LineSolid,
    LineDoubleDash,
    LineOnOffDash
};

static int      CapStyles[] = {
    CapNotLast,
    CapButt,
    CapRound,
    CapProjecting
};

static int      JoinStyles[] = {
    JoinMiter,
    JoinRound,
    JoinBevel
};

static int      FillStyles[] = {
    FillSolid,
    FillTiled,
    FillOpaqueStippled,
    FillStippled
};

static int      FillRules[] = {
    EvenOddRule,
    WindingRule
};

static int      ArcModes[] = {
    ArcChord,
    ArcPieSlice
};

#define		SupportedEvents	9
static long	EventMasks[] = {
    NoEventMask,		/* Ev_None */
    ButtonPressMask,		/* Ev_ButtonDown */
    ButtonReleaseMask,		/* Ev_ButtonUp */
    KeyPressMask,		/* Ev_KeyDown */
    KeyReleaseMask,		/* Ev_KeyUp */
    PointerMotionMask,		/* Ev_PointerMove */
    EnterWindowMask,		/* Ev_EnterWindow */
    LeaveWindowMask,		/* Ev_ExitWindow */
    StructureNotifyMask		/* Ev_DeleteWindow */
};

#define		SupportedButtonKeys	13
static unsigned int ButtonKeys[] = {
    0,
    Button1Mask,
    Button2Mask,
    Button3Mask,
    Button4Mask,
    Button5Mask,
    ShiftMask,
    LockMask,
    ControlMask,
    Mod1Mask,
    Mod2Mask,
    Mod3Mask,
    Mod4Mask,
    Mod5Mask
};

#define		SupportedButtons	5
static unsigned int Buttons[] = {
    0,
    Button1,
    Button2,
    Button3,
    Button4,
    Button5
};

static unsigned int StdCursors[] = {
    XC_X_cursor, XC_arrow, XC_based_arrow_down, XC_based_arrow_up,
    XC_boat, XC_bogosity, XC_bottom_left_corner,
    XC_bottom_right_corner, XC_bottom_side, XC_bottom_tee,
    XC_box_spiral, XC_center_ptr, XC_circle, XC_clock, XC_coffee_mug,
    XC_cross, XC_cross_reverse, XC_crosshair, XC_diamond_cross, XC_dot,
    XC_dotbox, XC_double_arrow, XC_draft_large, XC_draft_small,
    XC_draped_box, XC_exchange, XC_fleur, XC_gobbler, XC_gumby,
    XC_hand1, XC_hand2, XC_heart, XC_icon, XC_iron_cross, XC_left_ptr,
    XC_left_side, XC_left_tee, XC_leftbutton, XC_ll_angle, XC_lr_angle,
    XC_man, XC_middlebutton, XC_mouse, XC_pencil, XC_pirate, XC_plus,
    XC_question_arrow, XC_right_ptr, XC_right_side, XC_right_tee,
    XC_rightbutton, XC_rtl_logo, XC_sailboat, XC_sb_down_arrow,
    XC_sb_h_double_arrow, XC_sb_left_arrow, XC_sb_right_arrow,
    XC_sb_up_arrow, XC_sb_v_double_arrow, XC_shuttle, XC_sizing,
    XC_spider, XC_spraycan, XC_star, XC_target, XC_tcross,
    XC_top_left_arrow, XC_top_left_corner, XC_top_right_corner,
    XC_top_side, XC_top_tee, XC_trek, XC_ul_angle, XC_umbrella,
    XC_ur_angle, XC_watch, XC_xterm, None
};

/*
 * internal types
 */

typedef struct {
    Int             x, y;
}               _Point;

typedef struct {
    Int             x, y, w, h;
}               _Rectangle;

typedef struct winList {	/* SR: winList */
    Int             w, h;
    Window          xw, xparent;
    Pixmap          pm;		/* backup image */
    Ptr             srw;	/* to winContextRec */
    Ocap            evchan;	/* SR: cap winEventChannel */
    struct winList *nextw;
}              *PtWinList;	/* SR: PtWinList */

typedef struct winDispWinRec {	/* SR: winDispWinRec */
    Display        *display;
    Int             poll;
    Int             stay;
    GC              blkgc;	/* for refreshing */
    Colormap        cmap;
    Atom            delw;	/* get to know when it is about to be killed */
    Int             enabled;
    Int             mapped;
    Int             draw2win;
    struct winList  wlist;
}              *PtDispWinRec;	/* SR: PtDispWinRec */

typedef struct winContextRec {	/* SR: winContextRec */
    PtDispWinRec    dispw;
    Window          xw;
    Pixmap          pm;
    GC              gc;
}              *PtContextRec;

typedef struct {
    Int             event_type;
    PtContextRec    window;
    Int             x, y;
    Int             bk_status;
    Int             data;
    Int             keysym;
}               _Event;		/* SR: winEvent */
