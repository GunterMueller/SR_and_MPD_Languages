/*  event.c -- event waiting and handling  */

#include "rts.h"

#ifdef __PARAGON__
#include "nx.h"
extern local_message_type;	/* message type == ptype of current VM */
#endif

static void update_clock ();
static int wake_sleepers (), iocheck (), fdmerge ();

struct napper {		/* napping job information */
    struct napper *next;	/* next list entry */
    Sem sp;			/* semaphore to tickle */
    int wakeup;			/* wakeup time */
};

struct iowait {		/* i/o blocked job information */
    struct iowait *next;	/* next list entry */
    Sem sp;			/* semaphore to tickle */
    fd_set *query;		/* where to find set of wanted files */
    fd_set *results;		/* where to put the results */
    enum io_type inout;		/* INPUT or OUTPUT wait */
};

enum fdset_op { FDSET_AND, FDSET_OR };



static fd_set zeroset;



/*  nap_list and io_list are protected by sr_queue_mutex. */

static struct napper *nap_list = NULL;	/* napping jobs (unordered list) */
static struct iowait *io_list = NULL;	/* I/O blocked jobs (in order blocked)*/
static struct iowait *io_tail;		/* end of the list */

static struct timeval start;	/* system time when we started executing */
static int msclock;		/* elapsed real time, in milliseconds */



/*
 *  sr_nap_list_empty() and sr_evio_list_empty() are used for idleness checking.
 *  The caller already holds the queue_mutex.
 */

Bool sr_nap_list_empty ()
{
    return nap_list == NULL;
}

Bool sr_evio_list_empty ()
{
    return io_list == NULL || (sr_exec_up && io_list->next == NULL);
}



/*
 *  Initialize the clock so we can measure a program's age.
 *  Should be called before job servers are created, since it
 *  contains a system call gettimeofday ().
 */
void
sr_init_event ()
{
    struct timezone tz;

    gettimeofday (&start, &tz);
}



/*
 *  Return the elapsed time in milliseconds.
 */
int
sr_age ()
{
    update_clock ();			/* get the clock */
    wake_sleepers ();			/* having done so, check wakeups */
    return msclock;			/* return the age */
}



/*
 *  Delay a process a specified number of milliseconds.
 */
void
sr_nap (locn, msec)
char *locn;
{
    Sem sp;
    struct napper *np;

    if (msec <= 0) {			/* if no delay, just reschedule */
	sr_loop_resched (locn);
	return;
    }

    update_clock ();			/* get clock reading */
    wake_sleepers ();			/* awaken all whose clocks have rung */

    sp = sr_make_sem (0);
    np = (struct napper *) sr_alc (-sizeof (struct napper), 1);
    np->sp = sp;
    np->wakeup = msclock + msec;

    LOCK_QUEUE ("sr_nap");
    np->next = nap_list;		/* add to nap list */
    nap_list = np;
    UNLOCK_QUEUE ("sr_nap");

    P ((char *) 0, sp);				/* wait for wakeup call */
    sr_kill_sem (sp);
}



/*
 *  Wait for input or activity on a set of files.
 *  "query" is not dereferenced here yet, but later when blocking on I/O.
 *
 *  One place this is called is from sr_net_recv () in socket.c, with the
 *  first two parameters needing to be protected for MultiSR.  Fortunately,
 *  the only place in this file they are assigned to (indirectly, through
 *  the io_list) is protected by a mutex on the io_list.
 */
void
sr_iowait (query, results, inout)
fd_set *query, *results;
enum io_type inout;
{
    Sem sp;
    struct iowait *ip;

    sp = sr_make_sem (0);
    ip = (struct iowait *) sr_alc (-sizeof (struct iowait), 1);
    ip->next = NULL;
    ip->sp = sp;
    /* since query and results are to be protected in MultiSR, any
     * assignment to ip->query or ip->results must be protected. */
    ip->query = query;
    ip->results = results;
    ip->inout = inout;

    /* add to end of list, with interlock */
    LOCK_QUEUE ("sr_iowait");
    if (io_list == NULL)
	io_list = ip;
    else
	io_tail->next = ip;
    io_tail = ip;
    UNLOCK_QUEUE ("sr_iowait");

    P ((char *) 0, sp);			/* wait for file ready signal */
    sr_kill_sem (sp);
}



/*
 *  Check for I/O or timer event; return nonzero if any processes were awakened.
 *  Time out after t milliseconds, or never if t < 0.
 */
int
sr_evcheck (t)
int t;
{
    int n, w;

    w = 0;
    if (nap_list != NULL) {
	update_clock ();		/* update clock reading */
	n = wake_sleepers ();		/* try to awaken napping jobs */
	if (n == 0)
	    w = 1;			/* if we awakened somebody */
#ifndef __PARAGON__
	if ((t < 0 && n < MAX_INT) || (t > n))
	    t = n;			/* shorten timeout until wakeup due */
#endif
    }

#ifdef __PARAGON__
    if (t != 0)
	t = 1;				/* on Paragon, just busy-wait */
#endif
    
    return w | iocheck (t);
}



/*
 *  Check I/O and return nonzero if any processes were awakened.
 *  Time out after t milliseconds, or never if t < 0.
 */
static int
iocheck (t)
int t;
{
    struct iowait *ip, *ip2, **ipp;
    struct timeval tv, *tp;
    static struct timeval zerotime;
    fd_set inset, outset, readyset;
    int n;

    LOCK_QUEUE ("iocheck");
    if (io_list == NULL && t == 0) {
	UNLOCK_QUEUE ("iocheck");
	return 0;
    }

    n = 0;
    inset = outset = zeroset;

#ifndef __PARAGON__
    for (ip = io_list; ip != NULL; ip = ip->next)
	if (ip->inout == INPUT)
	    n = fdmerge (&inset, &inset, ip->query, FDSET_OR);
	else
	    n = fdmerge (&outset, &outset, ip->query, FDSET_OR);

#else
    ip = io_list; 
    ip2 = NULL;
    while (ip != NULL) { 
	if (ip->query != NULL) {	/* if not receive of Paragon message */
	    if (ip->inout == INPUT)       
		n = fdmerge (&inset, &inset, ip->query, FDSET_OR);
	    else
		n = fdmerge (&outset, &outset, ip->query, FDSET_OR);
	    ip2 = ip;
	    ip = ip->next;
	} 
	else {
	    /*
	     * Check if a message on with correct message type is waiting.
	     * If so, remove entry from list and restart waiting process.
	     */
	    DEBUG (D_SELECT, "Network check for msg_type %u",
	        local_message_type, 0, 0);
	    if (iprobex (local_message_type, -1, -1, msginfo) != 0) {
		DEBUG (D_SELECT, "Network traffic", 0, 0, 0);
		V (ip->sp);
		if (ip == io_list) {
		    io_list = io_list->next;
		    ip2 = NULL;
		}
		else {
		    ip2->next = ip->next;
		}
		if (io_tail == ip)
		    io_tail = ip2;
		sr_free ((Ptr) ip);
		UNLOCK_QUEUE ("iocheck");
	        return 1;
	    }
	    ip2 = ip;
	    ip = ip->next;
	}
    }
#endif

    UNLOCK_QUEUE ("iocheck");

    DEBUG (D_SELECT, "select: %08lX %08lX t=%ld",
	inset.fds_bits[0], outset.fds_bits[0], t);
    if (t < 0)
	tp = NULL;			/* if not to timeout */
    else {
	tv = zerotime;			/* set timeout value */
	tv.tv_usec = 1000 * (t % 1000);
	tv.tv_sec = t / 1000;
	tp = &tv;
    }

    BEGIN_IO (NULL);
    n = select (n, &inset, &outset, (fd_set *) 0, tp);
    END_IO (NULL);

    DEBUG (D_SELECT, "selectd %08lX %08lX n=%ld",
	inset.fds_bits[0], outset.fds_bits[0], n);

    if (n < 0)  {
	perror ("select failure");
	sr_abort ("I/O error");
    }

    if (n == 0)				/* if no I/O ready */
	return 0;

    /* one or more files are ready for I/O */
    LOCK_QUEUE ("iocheck");
    for (ipp = &io_list; (ip = *ipp) != NULL;) {
	if (ip->inout == INPUT)
	    n = fdmerge (&readyset, &inset, ip->query, FDSET_AND);
	else
	    n = fdmerge (&readyset, &outset, ip->query, FDSET_AND);
	if (n > 0) {
	    *ip->results = readyset;
	    V (ip->sp);
	    *ipp = ip->next;

	    /*  Important note: this ip cannot belong to any resource.
	     *  Otherwise sr_free would try to lock the resource mutex,
	     *  violating the rule that it must be grabbed before queue_mutex,
	     *  which we already hold.
	     */
	    sr_free ((Ptr) ip);

	} else {
	    ipp = &ip->next;
	    io_tail = ip;
	}
    }
    UNLOCK_QUEUE ("iocheck");

    return 1;				/* indicate we accomplished something */
}



/*
 *  Merge two fd_set structures to create a third.
 *
 *  fdmerge (result, set1, set2, op)
 *	first three parameters are pointers to fd_set structs.
 *	op is FDSET_AND or FDSET_OR.
 *  return value is 1 + highest fd set in result, or 0 if none.
 */
static int
fdmerge (result, set1, set2, op)
fd_set *result, *set1, *set2;
enum fdset_op op;
{
    int i, n;
    fd_set tempset;

    tempset = zeroset;
    for (i = n = 0; i < FD_SETSIZE; i++) {
	if ((op == FDSET_AND)
	    ? (FD_ISSET (i, set1) && FD_ISSET (i, set2))
	    : (FD_ISSET (i, set1) || FD_ISSET (i, set2))) {
		FD_SET (i, &tempset);
		n = i + 1;
	}
    }
    *result = tempset;
    return n;
}



/*
 *  Put the current elapsed time in msclock.
 */
static void
update_clock ()
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday (&tv, &tz);
    msclock = 1000 * (tv.tv_sec - start.tv_sec)
		    + tv.tv_usec / 1000 - start.tv_usec / 1000;
    DEBUG (D_CLOCK, "clock %ld", msclock, 0, 0);
}



/*
 *  Awaken napping jobs whose wakeup times have arrived.
 *  Called every time we've read the system clock into msclock.
 *
 *  Returns timeout in msec until next wakeup.
 *  This is MAX_INT if the queue is empty and 0 if somebody was awakened.
 */
static int
wake_sleepers ()
{
    struct napper **ptr, *np;
    int t, tmin;

    tmin = MAX_INT;
    if (nap_list != NULL) {
	LOCK_QUEUE ("wake_sleepers");
	for (ptr = &nap_list; (np = *ptr) != NULL;) {
	    t = np->wakeup - msclock;
	    if (t <= 0 || !np->sp->blocked.head) { /* if wakeup time, or gone */
		V (np->sp);			/* awaken process */
		*ptr = np->next;		/* remove from list */
		sr_free ((Ptr) np);		/* free the entry */
		tmin = 0;
	    } else {
		ptr = &np->next;
		if (tmin > t)
		    tmin = t;		/* remember max remaining time */
	    }
	}
	UNLOCK_QUEUE ("wake_sleepers");
    }
    return tmin;
}



#ifdef sysV68
/* gettimeofday () is not supplied by the v68 library; fake it. */
int
gettimeofday (tp, tzp)
struct timeval *tp;
struct timezone *tzp;
{
    time (&tp->tv_sec);
    tp->tv_usec = 0;
    tzp->tz_minuteswest = 0;
    tzp->tz_dsttime = 0;
}
#endif
