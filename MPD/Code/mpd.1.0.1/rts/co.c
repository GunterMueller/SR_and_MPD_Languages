/*  co.c -- runtime support of the co statement  */

#include "rts.h"

static void init_cob (), re_init_cob ();

static Pool cob_pool;		/* pool of co block descriptors */



/*
 *  Get a new co block for the start of a co statement
 *  and link it to the current process.
 */
void
mpd_co_start (locn)
char *locn;
{
    Cob cobp;

    mpd_check_stk (CUR_STACK);
    TRACE ("CO", locn, 0);

    cobp = (Cob) mpd_addpool (cob_pool);
    cobp->next = CUR_PROC->cob_list;
    CUR_PROC->cob_list = cobp;

    cobp->pending = 0;
    cobp->done = mpd_make_sem (0);
}



/*
 *  Set things up for a call within a co statement.
 */
void
mpd_co_call (ibp)
Invb ibp;
{
    Cob cobp;

    cobp = ibp->co.cobp = CUR_PROC->cob_list;
    LOCK (cobp->cobmutex, "mpd_co_call");
    ibp->co.seqn = cobp->seqn;
    cobp->pending++;
    UNLOCK (cobp->cobmutex, "mpd_co_call");
}



/*
 *  A call invocation from a co statement has returned.
 *  If the invoker is still interested in this event
 *  notify him.
 */
void
mpd_co_call_done (ibp)
Invb ibp;
{
    Cob cobp;

    cobp = ibp->co.cobp;
    LOCK (cobp->cobmutex, "mpd_co_call_done");

    if (cobp->seqn == ibp->co.seqn) {
	ibp->co.next = cobp->done_list;
	cobp->done_list = ibp;
	cobp->pending--;
	UNLOCK (cobp->cobmutex, "mpd_co_call_done");
	V (cobp->done);
    } else {
	UNLOCK (cobp->cobmutex, "mpd_co_call_done");
	mpd_free ((Ptr) ibp);
    }
}


/*
 *  Handle a send within a co statement.
 *    Make a copy of the invocation block to return to invoker.
 *    Then, simulate a mpd_co_call and mpd_co_call_done.
 */
void
mpd_co_send (ibp)
Invb ibp;
{
    Invb new_ibp;

    new_ibp = mpd_dup_invb (ibp);
    mpd_co_call (new_ibp);
    mpd_co_call_done (new_ibp);
}



/*
 *  Wait for a co invocation to terminate.  Return a pointer to the original
 *  invocation block so that the GC can copy result parameters and
 *  find out which arm terminated.
 */
Ptr
mpd_co_wait (locn)
char *locn;
{
    Cob cobp;
    Invb ibp;

    CUR_PROC->locn = locn;		/* add locn to CUR_PROC structure */

    mpd_check_stk (CUR_STACK);

    cobp = CUR_PROC->cob_list;
    LOCK (cobp->cobmutex, "mpd_co_wait");

    if (cobp->pending == 0 && cobp->done_list == NULL)
	ibp = NULL;
    else {
	UNLOCK (cobp->cobmutex, "mpd_co_wait");
	P ((char *) 0, cobp->done);

	LOCK (cobp->cobmutex, "mpd_co_wait");

	/*  Very rarely, done_list will be empty here.
	 *  This should not happen, but the bug is elusive.  */
	if (cobp->done_list == NULL) 
	    mpd_malf ("intermittent mpd_co_wait bug---rerun program");
	ibp = cobp->done_list;
	cobp->done_list = ibp->co.next;
    }

    UNLOCK (cobp->cobmutex, "mpd_co_wait");
    return (Ptr) ibp;
}



/*
 *  A co statement has terminated.  Release the co block.
 */
void
mpd_co_end (locn)
char *locn;
{
    Cob cobp;
    Invb ibp;

    mpd_check_stk (CUR_STACK);

    cobp = CUR_PROC->cob_list;
    CUR_PROC->cob_list = cobp->next;

    TRACE ("OC", locn, 0);

    /*  this is the only place that there is a nesting between any co
     *  locks, so any order we choose here is fine */
    LOCK (cobp->cobmutex, "mpd_co_end");
    cobp->seqn++;
    mpd_kill_sem (cobp->done);

/*
 *  Free any invocation blocks that were returned after
 *  the last mpd_co_wait () was done.
 */
    while (ibp = cobp->done_list) {
	cobp->done_list = ibp->co.next;
	/* note this ipb can never belong to a resource. */
	mpd_free ((Ptr) ibp);
    }

    UNLOCK (cobp->cobmutex, "mpd_co_end");
    mpd_delpool (cob_pool, (Ptr) cobp);
}


/*
 *  Initialize co statement part of MPD RTS.
 */
void
mpd_init_co ()
{
    cob_pool = mpd_makepool ("co-block descriptors", sizeof (struct cob_st),
				mpd_max_co_stmts, init_cob, re_init_cob);
}


static void
init_cob (cb)
Cob cb;
{
    static unsigned short sn = INIT_SEQ_CO;

    /* called under cob_pool lock */
    cb->seqn = sn++;
    INIT_LOCK (cb->cobmutex, "cb->cobmutex");
    cb->done_list = NULL;
}

/*ARGSUSED*/ /*(under MultiMPD they are)*/
static void
re_init_cob (cb)
Cob cb;
{
    RESET_LOCK (cb->cobmutex);
}
