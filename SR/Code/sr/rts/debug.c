/*  debug.c -- common debugging routines for runtime system and SRX  */

#include "rts.h"

int sr_dbg_flags = 0;		/* currently enabled debugging options */

static Mutex debug_mutex;	/* for coordinating debug outputs */

static char status_tag[MAX_STATUS+1];	/* mnemonics for status values */



/*  sr_init_debug (s) - set debug flags according to hex string s.
 *  In MultiSR, must be called before job servers are created.
 */
void
sr_init_debug (s)
char *s;
{
#ifndef NDEBUG
    int i;
				/*SUPPRESS 442*/ /* (for Saber C) */
    static char xci[] =		/* maps hex characters into integer values */
	"\0\12\13\14\15\16\17\0\0\0\0\0\0\0\0\0\0\1\2\3\4\5\6\7\10\11";

#ifdef MULTI_SR
    /* init mutex without using a macro that would call DEBUG */
    multi_alloc_lock (debug_mutex.lock);
    multi_reset_lock (debug_mutex.lock);
#endif

    if (!s)				/* if s is null, try env SR_DEBUG */
	s = getenv (ENV_DEBUG);
    if (s)
	while (*s)
	    sr_dbg_flags = (sr_dbg_flags << 4) | xci[*s++ & 037];

    for (i = 0; i <= MAX_STATUS; i++)
	status_tag[i] = '#';
    status_tag[ACTIVE]		= 'A';
    status_tag[READY]		= 'R';
    status_tag[BLOCKED]		= 'B';
    status_tag[INFANT]		= 'I';
    status_tag[FREE]		= 'F';
    status_tag[DOING_IO]	= 'D';
    status_tag[BLOCKED_DOING_IO]	= 'S';
    status_tag[TO_BE_FREED]	= 'T';

    if (sr_my_vm == MAIN_VM)
	DEBUG (D_ALL_FLAGS, "debug flags set to %lX", sr_dbg_flags, 0, 0);
#endif /* NDEBUG */
}



#ifndef NDEBUG

/*  sr_bugout(f,v1,v2,v3,v4,v5) -- print debugging value v under format f.
 *
 *  We build the output line (boilerplate + formatted arguments) in an
 *  internal buffer, then issue a single unbuffered write() call.
 *  This simplifies synchronization and ensures a contiguous message.
 */
int
sr_bugout (f, v1, v2, v3, v4, v5)
char *f;
long v1, v2, v3, v4, v5;
{
    char obuf[300];
    int n;

#ifdef SRX
    Proc cur = NULL;
#else
    Proc cur = CUR_PROC;
#endif

#ifdef MULTI_SR

    /*  format: [V.J] (T:qQ:IS) M
     *
     *  upper case letters are substituted as indicated below
     *  lower case letters are printed verbatim as mnemonics
     *
     *  V is VM number
     *  J is jobserver number
     *  T is current thread
     *  Q is the depth that this jobserver holds sr_queue_mutex
     *  I is I if the current thread is an idle thread and N otherwise
     *  S is a code for the current thread's status.  See array
     *		status_tag[] above for the code.
     *  M is debug message
     */
     
    if (cur == NULL)
	sprintf (obuf, "[%d.0] (------------) ", sr_my_vm);
    else {
	sprintf (obuf, "[%d.%d] (%06lX:q%d:%c%c) ",
	    sr_my_vm, PRIV (my_jobserver_id),
	    (long) cur, PRIV (js_queue_depth),
	    (cur->ptype == IDLE_PR ? 'I' : 'N'),
	    status_tag[cur->status]);
    }

#else /* no MULTI_SR */

    /*  format: [V] (T) M
     *
     *	V is VM number
     *  T is current thread
     *  M is debug message
     */
    sprintf (obuf, "[%d] (%06lX) ", sr_my_vm, (long) cur);

#endif /* MULTI_SR */

    n = strlen (obuf);
    sprintf (obuf + n, f, v1, v2, v3, v4, v5);
    n += strlen (obuf + n);
    obuf[n++] = '\n';

    /*
     * Write output, synchronized with debugs on other processors.
     * Avoid LOCK/UNLOCK macros because they call debug recursively!
     */
#ifdef MULTI_SR
    multi_lock (debug_mutex.lock);
    write (STDERR, obuf, n);
    multi_unlock (debug_mutex.lock);
#else
    write (STDERR, obuf, n);
#endif

    return 0;
}



/*
 *  sr_msgname (t) -- return a printable message type name.
 */
char *
sr_msgname (t)
enum ms_type t;
{
    static char buf[20];	/* not locked in MultiSR; we'll chance it. */

    switch (t) {
	case MSG_BAD:		return "MSG_BAD";
	case MSG_NONE:		return "MSG_NONE";
	case MSG_SEOF:		return "MSG_SEOF";
	case MSG_HELLO:		return "MSG_HELLO";
	case MSG_STOP:		return "MSG_STOP";
	case MSG_IDLE:		return "MSG_IDLE";
	case MSG_QUIT:		return "MSG_QUIT";
	case MSG_EXIT:		return "MSG_EXIT";
	case MSG_RCVCALL:	return "MSG_RCVCALL";
	case REQ_LOCVM:		return "REQ_LOCVM";
	case ACK_LOCVM:		return "ACK_LOCVM";
	case REQ_CREVM:		return "REQ_CREVM";
	case ACK_CREVM:		return "ACK_CREVM";
	case REQ_FINDVM:        return "REQ_FINDVM";
	case ACK_FINDVM:        return "ACK_FINDVM";
	case REQ_CREATE:        return "REQ_CREATE";
	case ACK_CREATE:        return "ACK_CREATE";
	case REQ_DESTROY:       return "REQ_DESTROY";
	case ACK_DESTROY:       return "ACK_DESTROY";
	case REQ_COUNT:		return "REQ_COUNT";
	case ACK_COUNT:		return "ACK_COUNT";
	case REQ_INVOKE:        return "REQ_INVOKE";
	case ACK_INVOKE:        return "ACK_INVOKE";
	case REQ_RECEIVE:	return "REQ_RECEIVE";
	case ACK_RECEIVE:	return "ACK_RECEIVE";
	case REQ_DESTOP:	return "REQ_DESTOP";
	case ACK_DESTOP:	return "ACK_DESTOP";
	case REQ_DESTVM:        return "REQ_DESTVM";
	case ACK_DESTVM:        return "ACK_DESTVM";
	case REQ_CALLME:        return "REQ_CALLME";
	case ACK_CALLME:        return "ACK_CALLME";
	default:	
	    sprintf (buf, "[?msg %d?]", t);
	    return buf;
    }
}



#endif /* NDEBUG */



/*
 *   sr_debug lets an SR program print a debugging message.
 *
 *   sample usage:
 *      external sr_debug (s: string[*]; n1,n2,n3: int)
 *	sr_debug ("i,j,k: %d %d %d", i, j, k)
 */
void
sr_debug (s, n1, n2, n3)
char *s;
int n1, n2, n3;
{
    DEBUG (D_PROG, s, n1, n2, n3);
}



/*  sr_get_debug returns the current debug value  */

int
sr_get_debug ()
{
    return (int) sr_dbg_flags;
}


/*  sr_set_debug (n) -- set debugging value  */

void
sr_set_debug (n)
int n;
{
    sr_dbg_flags = n;
    DEBUG (D_ALL_FLAGS, "sr_set_debug(%lX)", n, 0, 0);
}
