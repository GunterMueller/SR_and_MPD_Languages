/*  mpdwin.c -- C functions for MPDWin (Window/Graphics Package for MPD)  */

#include	"mpdwin.h"
#include	"mpdwin.icon"

/* ---------------------------------------------------------------------- */

static PtWinList
findWinFromX (dispw, xwin)
PtDispWinRec    dispw;
Window          xwin;
{
    PtWinList       tmpw = &(dispw->wlist);

    while ((tmpw->nextw) && (tmpw->xw != xwin))
	tmpw = tmpw->nextw;

    if (tmpw->xw == xwin)
	return tmpw;
    else
	return NULL;
}

/* ---------------------------------------------------------------------- */

static void
deleteNextWinStruct (dispw, wstruct)
PtDispWinRec    dispw;
PtWinList       wstruct;
{
    PtWinList       tmpw = wstruct->nextw;
    PtWinList       tmpw2;

    /* try to find and delete all subwindows first */
    while (tmpw->nextw) {
	tmpw2 = tmpw;
	tmpw = tmpw->nextw;
	if (tmpw->xparent == wstruct->nextw->xw)
	    deleteNextWinStruct (dispw, tmpw2);
    }

    tmpw2 = wstruct->nextw;
    tmpw = tmpw2->nextw;
    XFreeGC (dispw->display, ((PtContextRec) tmpw2->mpdw)->gc);
    XFreePixmap (dispw->display, tmpw2->pm);
    XDestroyWindow (dispw->display, tmpw2->xw);
    XFree ((char *) tmpw2);
    wstruct->nextw = tmpw;
    return;
}

/* ---------------------------------------------------------------------- */

Int
MPDWin_DestroyWindow (mpdw)
PtContextRec    mpdw;
{
    PtDispWinRec    dispw = mpdw->dispw;
    PtWinList       tmpw = &(dispw->wlist);
    PtWinList       tmpw2 = tmpw;

    while ((tmpw->nextw) && (tmpw->xw != mpdw->xw)) {
	tmpw2 = tmpw;
	tmpw = tmpw->nextw;
    }

    if ((tmpw->xw != mpdw->xw) || (tmpw == tmpw2))
	return False;

    deleteNextWinStruct (mpdw->dispw, tmpw2);
    return True;
}

/* ---------------------------------------------------------------------- */

Int
MPDWin_Open (mpdw, display, w, h)
PtContextRec    mpdw;
char           *display;
Int             w, h;
{
    PtDispWinRec    dispw = mpdw->dispw;
    XWMHints        wmhints;
    XClassHint      classhints;
    unsigned long   blackp, whitep;
    int             screen, depth;
    XSizeHints      sh;
    XGCValues       values;
    unsigned int    attrvm;
    Pixmap          wicon;
    XSetWindowAttributes attr;

#if defined(sequent)
    /* MPDWin will not work under Multi-MPD */
    int             mpdp;
    char           *mmpd = (char *) getenv ("MPD_PARALLEL");
    if ((mmpd != NULL) && (sscanf (mmpd, "%d", &mpdp) > 0) && (mpdp > 1)) {
	fprintf (stderr,"MPDWin cannot work under MultiMPD on this machine.\n");
	return False;
    }
#endif

    if ((dispw->display = XOpenDisplay (display)) == NULL) {
	return False;		/* error */
    }
    screen = DefaultScreen (dispw->display);
    blackp = BlackPixel (dispw->display, screen);
    whitep = WhitePixel (dispw->display, screen);
    depth = DefaultDepth (dispw->display, screen);

    /* default settings */
    attrvm = CWBorderPixel | CWBackPixel | CWEventMask | CWDontPropagate;
    attr.border_pixel = whitep;
    attr.background_pixel = blackp;
    attr.event_mask = ALWAYS_MASKS | StructureNotifyMask;
    attr.do_not_propagate_mask = DONTPROPAGATE;

    /* check map_depth to create on/off screen window */
    mpdw->xw = XCreateWindow (dispw->display,
	DefaultRootWindow (dispw->display), 0, 0, w, h, 0,
	depth, InputOutput, CopyFromParent, attrvm, &attr);
    if (mpdw->xw == 0) {
	XCloseDisplay (dispw->display);
	return False;		/* error */
    }

    /* create backup pixmap */
    mpdw->pm = XCreatePixmap (dispw->display, mpdw->xw, w, h, depth);
    if (mpdw->pm == 0) {
	XCloseDisplay (dispw->display);
	return False;		/* error */
    }

    dispw->wlist.xw = mpdw->xw;
    dispw->wlist.pm = mpdw->pm;
    dispw->wlist.mpdw = (Ptr) mpdw;
    dispw->wlist.xparent = DefaultRootWindow (dispw->display);
    dispw->wlist.nextw = NULL;

    wicon = XCreateBitmapFromData (dispw->display, mpdw->xw,
	(char *) wicon_bits, wicon_width, wicon_height);

    /* can't be resized */
    dispw->wlist.w = w;
    dispw->wlist.h = h;
    sh.flags = PSize | PMinSize | PMaxSize;
    sh.width = (sh.min_width = (sh.max_width = w));
    sh.height = (sh.min_height = (sh.max_height = h));

    /* WM hints */
    wmhints.initial_state = NormalState;
    wmhints.input = True;
    wmhints.icon_pixmap = wicon;
    wmhints.flags = StateHint | InputHint | IconPixmapHint;
    classhints.res_name = display;
    classhints.res_class = "MPDWin";

    XSetWMProperties (dispw->display,
	mpdw->xw, NULL, NULL, NULL, 0, &sh, &wmhints, &classhints);

    /* get ready to receive WM_DELETE_WINDOW */
    dispw->delw = XInternAtom (dispw->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols (dispw->display, mpdw->xw, &dispw->delw, 1);

    /* set up a GC for copy-back 
     * remember to set fill-stuff of blkgc if window has background pixmap
     */
    values.foreground = blackp;
    values.background = blackp;
    values.graphics_exposures = False;
    dispw->blkgc = DefaultGC (dispw->display, screen);
    XChangeGC (dispw->display, dispw->blkgc,
		GCForeground | GCBackground | GCGraphicsExposures, &values);
    XFillRectangle (dispw->display, mpdw->pm, dispw->blkgc, 0, 0, w, h);

    return True;
}

/* ---------------------------------------------------------------------- */

PtWinList
MPDWin_CreateSubwindow (mpdw, x, y, w, h, nullec)
PtContextRec    mpdw;
Int             x, y, w, h;
Int             nullec;
{
    PtDispWinRec    dispw = mpdw->dispw;
    Window          oldxw;
    unsigned long   blackp, whitep;
    int             screen, depth;
    unsigned int    attrvm;
    XSetWindowAttributes attr;
    PtWinList       wstruct, tmpw;

    screen = DefaultScreen (dispw->display);
    blackp = BlackPixel (dispw->display, screen);
    whitep = WhitePixel (dispw->display, screen);
    depth = DefaultDepth (dispw->display, screen);

    /* default settings */
    attrvm = CWBorderPixel | CWBackPixel | CWEventMask | CWDontPropagate;
    attr.border_pixel = whitep;
    attr.background_pixel = blackp;
    attr.event_mask = ALWAYS_MASKS;
    if (nullec)
	attr.do_not_propagate_mask = DONTPROPAGATE;
    else
	attr.do_not_propagate_mask = 0;

    /* old window is passed in as mpdw->xw */
    oldxw = mpdw->xw;
    mpdw->xw = XCreateWindow (dispw->display, oldxw, x, y, w, h, 0,
	CopyFromParent, InputOutput, CopyFromParent, attrvm, &attr);
    if (mpdw->xw == 0) {
	return NULL;		/* error */
    }

    /* create backup pixmap */
    mpdw->pm = XCreatePixmap (dispw->display, mpdw->xw, w, h, depth);
    if (mpdw->pm == 0) {
	XDestroyWindow (dispw->display, mpdw->xw);
	return NULL;		/* error */
    }

    if ((wstruct = (PtWinList) malloc (sizeof (struct winList))) == NULL) {
	XDestroyWindow (dispw->display, mpdw->xw);
	XFreePixmap (dispw->display, mpdw->pm);
	return NULL;		/* error */
    }

    tmpw = &(dispw->wlist);
    while (tmpw->nextw) tmpw = tmpw->nextw;
    tmpw->nextw = wstruct;

    wstruct->xw = mpdw->xw;
    wstruct->pm = mpdw->pm;
    wstruct->mpdw = (Ptr) mpdw;
    wstruct->xparent = oldxw;
    wstruct->nextw = NULL;
    wstruct->w = w;
    wstruct->h = h;

    XFillRectangle (dispw->display, mpdw->pm, dispw->blkgc, 0, 0, w, h);

    return wstruct;
}

/* ---------------------------------------------------------------------- */

GC
MPDWin_NewGC (mpdw)
PtContextRec    mpdw;
{
    Display        *display = mpdw->dispw->display;
    int             screen;
    XGCValues       values;

    /* defaults other than those in X window? */
    screen = DefaultScreen (display);
    values.background = BlackPixel (display, screen);
    values.foreground = WhitePixel (display, screen);
    values.graphics_exposures = False;
    return XCreateGC (display, mpdw->xw, GCForeground |
	GCBackground | GCGraphicsExposures, &values);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_Close (mpdw)
PtContextRec    mpdw;
{
    PtWinList       tmpw = mpdw->dispw->wlist.nextw, tmp2;
    Display        *display = mpdw->dispw->display;

    XSync (display, True);
    XCloseDisplay (display);

    while (tmpw) {
	tmp2 = tmpw;
	tmpw = tmpw->nextw;
	XFree ((char *) tmp2);
    }
}

/* ---------------------------------------------------------------------- */

static int
getbuttonkeys (state)
unsigned int    state;
{
    int             rval, i;
    unsigned int    help;

    rval = 0;			/* BK_None */
    help = 1;
    for (i = 1; i <= SupportedButtonKeys; i++) {
	if (state & ButtonKeys[i])
	    rval |= help;
	help <<= 1;
    }
    return rval;
}

/* ---------------------------------------------------------------------- */

PtWinList
MPDWin_NextEvent (dispw, ptev)
PtDispWinRec    dispw;
_Event         *ptev;
{
    PtWinList       wstruct;
    XEvent          ev;
    int             x, y, w, h, i;
    unsigned int    help;
    char            buf;

    ptev->event_type = 0;	/* no event */
    ptev->bk_status = 0;
    ptev->data = 0;
    ptev->window = (PtContextRec) dispw->wlist.mpdw;
    wstruct = &(dispw->wlist);

    XNextEvent (dispw->display, &ev);

    /* fill the fields of ptev */
    switch (ev.type) {
    case Expose:
	/* refresh -- need to minimize "copy" operations */
	if ((wstruct = findWinFromX (dispw, ev.xexpose.window)) == NULL)
	    return NULL;
	ptev->window = (PtContextRec) wstruct->mpdw;
	x = ev.xexpose.x;
	y = ev.xexpose.y;
	w = ev.xexpose.width;
	h = ev.xexpose.height;
	XCopyArea (dispw->display, wstruct->pm, wstruct->xw,
	    dispw->blkgc, x, y, w, h, x, y);

	/*
	 * MPD event handler should call this function repeatedly until no
	 * event pending
	 */
	if (ev.xexpose.count == 0)
	    XFlush (dispw->display);
	break;
    case ButtonPress:		/* 1 */
    case ButtonRelease:		/* 2 */
	if ((wstruct = findWinFromX (dispw, ev.xbutton.window)) == NULL)
	    return NULL;
	ptev->window = (PtContextRec) wstruct->mpdw;
	ptev->event_type = (ev.type == ButtonPress ? 1 : 2);
	ptev->x = ev.xbutton.x;
	ptev->y = ev.xbutton.y;
	ptev->bk_status = getbuttonkeys (ev.xbutton.state);
	help = 1;
	i = 1;
	while ((ev.xbutton.button != Buttons[i]) && (i <= SupportedButtons)) {
	    i++;
	    help <<= 1;
	}
	ptev->data = help;
	break;
    case KeyPress:		/* 4 */
    case KeyRelease:		/* 8 */
	if ((wstruct = findWinFromX (dispw, ev.xkey.window)) == NULL)
	    return NULL;
	ptev->window = (PtContextRec) wstruct->mpdw;
	ptev->event_type = (ev.type == KeyPress ? 4 : 8);
	ptev->x = ev.xkey.x;
	ptev->y = ev.xkey.y;
	ptev->bk_status = getbuttonkeys (ev.xkey.state);
	XLookupString (&ev.xkey, &buf, 1, (KeySym *) &ptev->keysym, NULL);
	ptev->data = buf;
	break;
    case MotionNotify:		/* 16 */
	if ((wstruct = findWinFromX (dispw, ev.xmotion.window)) == NULL)
	    return NULL;
	ptev->window = (PtContextRec) wstruct->mpdw;
	ptev->event_type = 16;
	ptev->x = ev.xmotion.x;
	ptev->y = ev.xmotion.y;
	ptev->bk_status = getbuttonkeys (ev.xmotion.state);
	ptev->data = ev.xmotion.same_screen;
	break;
    case EnterNotify:		/* 32 */
    case LeaveNotify:		/* 64 */
	if ((wstruct = findWinFromX (dispw, ev.xcrossing.window)) == NULL)
	    return NULL;
	ptev->window = (PtContextRec) wstruct->mpdw;
	ptev->event_type = (ev.type == EnterNotify ? 32 : 64);
	ptev->x = ev.xcrossing.x;
	ptev->y = ev.xcrossing.y;
	ptev->bk_status = getbuttonkeys (ev.xcrossing.state);
	ptev->data = ev.xcrossing.focus;
	break;
    case MapNotify:
	if ((wstruct = findWinFromX (dispw, ev.xmap.window)) == NULL)
	    return NULL;
	if (wstruct == &dispw->wlist) {
	    dispw->mapped = True;
	    dispw->draw2win = dispw->enabled;
	}
	break;
    case UnmapNotify:
	if ((wstruct = findWinFromX (dispw, ev.xunmap.window)) == NULL)
	    return NULL;
	if (wstruct == &dispw->wlist) {
	    dispw->mapped = False;
	    dispw->draw2win = False;
	}
	break;
    case DestroyNotify:	/* 128 */
	if ((wstruct = findWinFromX (dispw, ev.xdestroywindow.window)) == NULL)
	    return NULL;
	ptev->window = (PtContextRec) wstruct->mpdw;
	ptev->event_type = 128;
	break;
    case ClientMessage:	/* 128 */
	if (ev.xclient.data.l[0] == dispw->delw)
	    ptev->event_type = 128;
	break;
    default:
	break;
    }

    return wstruct;
}

/* ---------------------------------------------------------------------- */

void
MPDWin_DrawPixel (mpdw, x, y)
PtContextRec    mpdw;
Int             x, y;
{
    PtDispWinRec    dispw = mpdw->dispw;

    XDrawPoint (dispw->display, mpdw->pm, mpdw->gc, x, y);
    if (dispw->draw2win)
	XDrawPoint (dispw->display, mpdw->xw, mpdw->gc, x, y);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_DrawLine (mpdw, pt1, pt2)
PtContextRec    mpdw;
_Point         *pt1, *pt2;
{
    PtDispWinRec    dispw = mpdw->dispw;

    XDrawLine (dispw->display, mpdw->pm, mpdw->gc,
	pt1->x, pt1->y, pt2->x, pt2->y);
    if (dispw->draw2win)
	XDrawLine (dispw->display, mpdw->xw, mpdw->gc,
	    pt1->x, pt1->y, pt2->x, pt2->y);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_DrawPolyline (mpdw, pts, n, polygon)
PtContextRec    mpdw;
_Point         *pts;
Int             n, polygon;
{
    PtDispWinRec    dispw = mpdw->dispw;
    XPoint         *xpts;
    int             i;

    if ((xpts = (XPoint *) calloc (n + 1, sizeof (XPoint))) == NULL) {
	/* error */
    } else {
	/* convert int to short */
	for (i = 0; i < n; i++) {
	    xpts[i].x = pts[i].x;
	    xpts[i].y = pts[i].y;
	}
	if (polygon) {
	    xpts[n].x = pts[0].x;
	    xpts[n].y = pts[0].y;
	    n++;
	}
	XDrawLines (dispw->display, mpdw->pm, mpdw->gc,
		xpts, n, CoordModeOrigin);
	if (dispw->draw2win)
	    XDrawLines (dispw->display, mpdw->xw, mpdw->gc,
		xpts, n, CoordModeOrigin);
	XFree ((char *) xpts);
    }
}

/* ---------------------------------------------------------------------- */

void
MPDWin_FillPolygon (mpdw, pts, n)
PtContextRec    mpdw;
_Point         *pts;
Int             n;
{
    XPoint         *xpts;
    int             i;
    PtDispWinRec    dispw = mpdw->dispw;

    if ((xpts = (XPoint *) calloc (n, sizeof (XPoint))) == NULL) {
	/* error */
    } else {
	/* convert int to short */
	for (i = 0; i < n; i++) {
	    xpts[i].x = pts[i].x;
	    xpts[i].y = pts[i].y;
	}
	XFillPolygon (dispw->display, mpdw->pm, mpdw->gc, xpts, n,
	    Complex, CoordModeOrigin);
	if (dispw->draw2win)
	    XFillPolygon (dispw->display, mpdw->xw, mpdw->gc, xpts, n,
		Complex, CoordModeOrigin);
	XFree ((char *) xpts);
    }
}

/* ---------------------------------------------------------------------- */

void
MPDWin_Rectangle (mpdw, x, y, w, h, fill)
PtContextRec    mpdw;
Int             x, y, w, h, fill;
{
    PtDispWinRec    dispw = mpdw->dispw;

    if (fill) {
	XFillRectangle (dispw->display, mpdw->pm, mpdw->gc,
		x, y, w, h);
	if (dispw->draw2win)
	    XFillRectangle (dispw->display, mpdw->xw, mpdw->gc,
			x, y, w, h);
    } else {
	XDrawRectangle (dispw->display, mpdw->pm, mpdw->gc,
		x, y, w, h);
	if (dispw->draw2win)
	    XDrawRectangle (dispw->display, mpdw->xw, mpdw->gc,
		x, y, w, h);
    }
}

/* ---------------------------------------------------------------------- */

void
MPDWin_Arc (mpdw, x, y, w, h, a1, a2, fill)
PtContextRec    mpdw;
Int             x, y, w, h, a1, a2, fill;
{
    PtDispWinRec    dispw = mpdw->dispw;

    if (fill) {
	XFillArc (dispw->display, mpdw->pm, mpdw->gc,
		x, y, w, h, a1 * 64, a2 * 64);
	if (dispw->draw2win)
	    XFillArc (dispw->display, mpdw->xw, mpdw->gc,
		x, y, w, h, a1 * 64, a2 * 64);
    } else {
	XDrawArc (dispw->display, mpdw->pm, mpdw->gc,
		x, y, w, h, a1 * 64, a2 * 64);
	if (dispw->draw2win)
	    XDrawArc (dispw->display, mpdw->xw, mpdw->gc,
		x, y, w, h, a1 * 64, a2 * 64);
    }
}

/* ---------------------------------------------------------------------- */

void
MPDWin_ClearArea (mpdw, x, y, w, h)
PtContextRec    mpdw;
Int             x, y, w, h;
{
    PtDispWinRec    dispw = mpdw->dispw;

    XFillRectangle (dispw->display, mpdw->pm, dispw->blkgc, x, y, w, h);
    if (dispw->draw2win)
	XFillRectangle (dispw->display, mpdw->xw, dispw->blkgc, x, y, w, h);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_EraseArea (mpdw, x, y, w, h)
PtContextRec    mpdw;
Int             x, y, w, h;
{
    XGCValues       tmp;
    PtDispWinRec    dispw = mpdw->dispw;
    int             func;	/* saved function */
    unsigned long   pm;		/* saved plane mask */
    unsigned long   fg;		/* saved foreground */
    unsigned long   vm = GCForeground | GCBackground | GCFunction | GCPlaneMask;

    if (!XGetGCValues (dispw->display, mpdw->gc, vm, &tmp)) {
	/* error */
	return;
    }
    /* change GC so it will draw in background */
    fg = tmp.foreground;	tmp.foreground = tmp.background;
    func = tmp.function;	tmp.function = GXcopy;
    pm = tmp.plane_mask;	tmp.plane_mask = ~(0L);
    XChangeGC (dispw->display, mpdw->gc, GCForeground, &tmp);
    if (dispw->draw2win)
	XFillRectangle (dispw->display, mpdw->xw, mpdw->gc, x, y, w, h);
    XFillRectangle (dispw->display, mpdw->pm, mpdw->gc, x, y, w, h);
    /* restore old GC values */
    tmp.foreground = fg;
    tmp.function = func;
    tmp.plane_mask = pm;
    XChangeGC (dispw->display, mpdw->gc, vm, &tmp);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_CopyArea (srcw, destw, src_rect, dest)
PtContextRec    srcw, destw;
_Rectangle     *src_rect;
_Point         *dest;
{
    PtDispWinRec    dispw = destw->dispw;

    XCopyArea (dispw->display, srcw->pm, destw->pm, destw->gc,
	src_rect->x, src_rect->y, src_rect->w, src_rect->h,
	dest->x, dest->y);
    if (dispw->draw2win)
	XCopyArea (dispw->display, destw->pm, destw->xw,
		dispw->blkgc, dest->x, dest->y, src_rect->w,
		src_rect->h, dest->x, dest->y);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_SetEventMask (mpdw, em)
PtContextRec    mpdw;
Int             em;
{
    PtDispWinRec    dispw = mpdw->dispw;
    int             i;
    long            eres;

    eres = EventMasks[0];
    for (i = 1; i <= SupportedEvents; i++) {
	if (em % 2)
	    eres |= EventMasks[i];
	em >>= 1;
    }
    if (mpdw->xw == dispw->wlist.xw) {
	eres |= StructureNotifyMask;
    }

    /* set input - include refreshing-related, and mapping on top-level */
    XSelectInput (dispw->display, mpdw->xw, ALWAYS_MASKS | eres);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_SetLabels (dispw, wlab, ilab)
PtDispWinRec    dispw;
char           *wlab, *ilab;
{
    if (wlab != NULL)
	XStoreName (dispw->display, dispw->wlist.xw, wlab);
    if (ilab != NULL)
	XSetIconName (dispw->display, dispw->wlist.xw, ilab);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_UpdateWindow (dispw)
PtDispWinRec    dispw;
{
    PtWinList       tmpw = &(dispw->wlist);

    do {
	XCopyArea (dispw->display, tmpw->pm, tmpw->xw,
	    dispw->blkgc, 0, 0, tmpw->w, tmpw->h, 0, 0);
	tmpw = tmpw->nextw;
    } while (tmpw);

    XFlush (dispw->display);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_SetDashes (mpdw, dash_offset, dash_list, n)
PtContextRec    mpdw;
Int             dash_offset, n;
char           *dash_list;
{
    XSetDashes (mpdw->dispw->display, mpdw->gc, dash_offset, dash_list, n);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_SetLineAttr (mpdw, line_width, line_style, cap_style, join_style)
PtContextRec    mpdw;
Int             line_width, line_style, cap_style, join_style;
{
    XSetLineAttributes (mpdw->dispw->display, mpdw->gc, line_width,
	LineStyles[line_style], CapStyles[cap_style], JoinStyles[join_style]);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_SetFillAttr (mpdw, fill_style, fill_rule)
PtContextRec    mpdw;
Int             fill_style, fill_rule;
{
    PtDispWinRec    dispw = mpdw->dispw;

    XSetFillStyle (dispw->display, mpdw->gc, FillStyles[fill_style]);
    XSetFillRule (dispw->display, mpdw->gc, FillRules[fill_rule]);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_SetArcMode (mpdw, arc_mode)
PtContextRec    mpdw;
Int             arc_mode;
{
    PtDispWinRec    dispw = mpdw->dispw;

    XSetArcMode (dispw->display, mpdw->gc, ArcModes[arc_mode]);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_SetClipRectangles (mpdw, ox, oy, rects, n)
PtContextRec    mpdw;
Int             ox, oy, n;
_Rectangle      rects[];
{
    PtDispWinRec    dispw = mpdw->dispw;
    XRectangle     *xrects;
    int             i;

    if ((xrects = (XRectangle *) calloc (n, sizeof (XRectangle))) == NULL) {
	/* error */
    } else {
	/* convert int to short */
	for (i = 0; i < n; i++) {
	    xrects[i].x = rects[i].x;
	    xrects[i].y = rects[i].y;
	    xrects[i].width = rects[i].w;
	    xrects[i].height = rects[i].h;
	}
	XSetClipRectangles (dispw->display, mpdw->gc,
		ox, oy, xrects, n, Unsorted);
	XFree ((char *) xrects);
    }
}

/* ---------------------------------------------------------------------- */

XFontStruct    *
MPDWin_DefaultFont (dispw)
PtDispWinRec    dispw;
{
    return XQueryFont (dispw->display, XGContextFromGC (dispw->blkgc));
}

/* ---------------------------------------------------------------------- */

XFontStruct    *
MPDWin_LoadFont (dispw, fontname)
PtDispWinRec    dispw;
char           *fontname;
{
    return XLoadQueryFont (dispw->display, fontname);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_SetFont (mpdw, font)
PtContextRec    mpdw;
XFontStruct    *font;
{
    XSetFont (mpdw->dispw->display, mpdw->gc, font->fid);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_FreeFont (dispw, font)
PtDispWinRec    dispw;
XFontStruct    *font;
{
    XFreeFont (dispw->display, font);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_DrawString (mpdw, x, y, str, len)
PtContextRec    mpdw;
Int             x, y, len;
char           *str;
{
    PtDispWinRec    dispw = mpdw->dispw;

    XDrawString (dispw->display, mpdw->pm, mpdw->gc, x, y, str, len);
    if (dispw->draw2win)
	XDrawString (dispw->display, mpdw->xw, mpdw->gc, x, y, str, len);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_DrawImageString (mpdw, x, y, str, len)
PtContextRec    mpdw;
Int             x, y, len;
char           *str;
{
    PtDispWinRec    dispw = mpdw->dispw;

    XDrawImageString (dispw->display, mpdw->pm, mpdw->gc, x, y, str, len);
    if (dispw->draw2win)
	XDrawImageString (dispw->display, mpdw->xw, mpdw->gc, x, y, str, len);
}

/* ---------------------------------------------------------------------- */

static Bool
get_color (disp, raw, color)
Display        *disp;
char           *raw;
XColor         *color;
{
    int             screen;
    Colormap        cmap;

    screen = DefaultScreen (disp);
    cmap = DefaultColormap (disp, screen);

    if (!XParseColor (disp, cmap, raw, color)) {
	/* error */
	return False;
    }
    if (!XAllocColor (disp, cmap, color)) {
	/* error */
	return False;
    }
    return True;
}

/* ---------------------------------------------------------------------- */

unsigned long
MPDWin_SetColor (mpdw, raw, is_fg)
PtContextRec    mpdw;
char           *raw;
Int             is_fg;
{
    XColor          color;
    PtDispWinRec    dispw = mpdw->dispw;

    if (!get_color (dispw->display, raw, &color)) {
	return -1L;					/* error */
    }
    if (is_fg)
	XSetForeground (dispw->display, mpdw->gc, color.pixel);
    else
	XSetBackground (dispw->display, mpdw->gc, color.pixel);

    return color.pixel;
}

/* ---------------------------------------------------------------------- */

unsigned long
MPDWin_SetBorder (mpdw, w, raw)
PtContextRec    mpdw;
char           *raw;
Int             w;
{
    PtDispWinRec    dispw = mpdw->dispw;
    XColor          color;

    XSetWindowBorderWidth (dispw->display, mpdw->xw, w);
    if (!get_color (dispw->display, raw, &color)) {
	return -1L;					/* error */
    }
    XSetWindowBorder (dispw->display, mpdw->xw, color.pixel);
    return color.pixel;
}

/* ---------------------------------------------------------------------- */

Cursor
MPDWin_CreateCursor (dispw, sc)
PtDispWinRec    dispw;
Int             sc;
{
    return XCreateFontCursor (dispw->display, StdCursors[sc]);
}

/* ---------------------------------------------------------------------- */

Int
MPDWin_SetCursor (mpdw, cur, fg, bg)
PtContextRec    mpdw;
Cursor          cur;
char           *fg, *bg;
{
    XColor          xfg, xbg;
    PtDispWinRec    dispw = mpdw->dispw;

    if ((!get_color (dispw->display, fg, &xfg)) ||
	(!get_color (dispw->display, bg, &xbg))) {
	/* error */
	return False;
    }
    XDefineCursor (dispw->display, mpdw->xw, cur);
    XRecolorCursor (dispw->display, cur, &xfg, &xbg);

    return True;
}

/* ---------------------------------------------------------------------- */

void
MPDWin_FreeCursor (dispw, cursor)
PtDispWinRec    dispw;
Cursor          cursor;
{
    XFreeCursor (dispw->display, cursor);
}

/* ---------------------------------------------------------------------- */

XImage *
MPDWin_CreateImage (dispw, d, w, h)
PtDispWinRec    dispw;
Int             d, w, h;
{
    XImage         *res;
    Screen         *screen = DefaultScreenOfDisplay (dispw->display);
    unsigned int    size;

    if (d <= (Int) 0)
	d = DefaultDepthOfScreen (screen);

    res = XCreateImage (dispw->display,
			DefaultVisualOfScreen (screen),
			d, ZPixmap, 0, (char *) NULL, w, h,
			BitmapPad (dispw->display), 0);
    if (res == (XImage *) NULL)
	return (XImage *) NULL;

    /* allocate X image pixel data */
    size = (unsigned int) (res->bytes_per_line + res->bitmap_pad / 8) *
		res->height;
    if (res->format == XYPixmap)
    	size *= res->depth;

    if ((res->data = (char *) calloc (1, size)) == (char *) NULL) {
	XDestroyImage (res);
	return (XImage *) NULL;
    }
    return res;
}

/* ---------------------------------------------------------------------- */

void
MPDWin_GetImage (mpdw, im, src_rect, dest)
PtContextRec    mpdw;
XImage         *im;
_Rectangle     *src_rect;
_Point         *dest;
{
    XGetSubImage (mpdw->dispw->display, mpdw->pm,
	src_rect->x, src_rect->y, src_rect->w, src_rect->h,
	AllPlanes, ZPixmap, im, dest->x, dest->y);
}

/* ---------------------------------------------------------------------- */

void
MPDWin_PutImage (mpdw, im, src_rect, dest)
PtContextRec    mpdw;
XImage         *im;
_Rectangle     *src_rect;
_Point         *dest;
{
    PtDispWinRec    dispw = mpdw->dispw;

    XPutImage (dispw->display, mpdw->pm, mpdw->gc, im,
	src_rect->x, src_rect->y, dest->x, dest->y, src_rect->w, src_rect->h);
    if (dispw->draw2win)
	XCopyArea (dispw->display, mpdw->pm, mpdw->xw,
	    dispw->blkgc, dest->x, dest->y, src_rect->w, src_rect->h,
	    dest->x, dest->y);
}

/* ---------------------------------------------------------------------- */

Int
WinFontAscent (f)
XFontStruct * f;
{
    return f->ascent;
}

/* ---------------------------------------------------------------------- */

Int
WinFontDescent (f)
XFontStruct * f;
{
    return f->descent;
}

/* ---------------------------------------------------------------------- */

void
MPDWin_SetDrawOp (mpdw, dop)
PtContextRec    mpdw;
int             dop;
{
    PtDispWinRec    dispw = mpdw->dispw;

    XSetFunction (dispw->display, mpdw->gc, dop);
}

/* ---------------------------------------------------------------------- */
