/*  co.c -- runtime support of the co statement  */

#include "rts.h"

static void init_cob (), re_init_cob ();

static Pool cob_pool;		/* pool of co block descriptors */



/*
 *  Get a new co block for the start of a co statement
 *  and link it to the current process.
 */
void
sr_co_start (locn)
char *locn;
{
    Cob cobp;

    sr_check_stk (CUR_STACK);
    TRACE ("CO", locn, 0);

    cobp = (Cob) sr_addpool (cob_pool);
    cobp->next = CUR_PROC->cob_list;
    CUR_PROC->cob_list = cobp;

    cobp->pending = 0;
    cobp->done = sr_make_sem (0);
}



/*
 *  Set things up for a call within a co statement.
 */
void
sr_co_call (ibp)
Invb ibp;
{
    Cob cobp;

    cobp = ibp->co.cobp = CUR_PROC->cob_list;
    LOCK (cobp->cobmutex, "sr_co_call");
    ibp->co.seqn = cobp->seqn;
    cobp->pending++;
    UNLOCK (cobp->cobmutex, "sr_co_call");
}



/*
 *  A call invocation from a co statement has returned.
 *  If the invoker is still interested in this event
 *  notify him.
 */
void
sr_co_call_done (ibp)
Invb ibp;
{
    Cob cobp;

    cobp = ibp->co.cobp;
    LOCK (cobp->cobmutex, "sr_co_call_done");

    if (cobp->seqn == ibp->co.seqn) {
	ibp->co.next = cobp->done_list;
	cobp->done_list = ibp;
	cobp->pending--;
	UNLOCK (cobp->cobmutex, "sr_co_call_done");
	V (cobp->done);
    } else {
	UNLOCK (cobp->cobmutex, "sr_co_call_done");
	sr_free ((Ptr) ibp);
    }
}


/*
 *  Handle a send within a co statement.
 *    Make a copy of the invocation block to return to invoker.
 *    Then, simulate a sr_co_call and sr_co_call_done.
 */
void
sr_co_send (ibp)
Invb ibp;
{
    Invb new_ibp;

    new_ibp = sr_dup_invb (ibp);
    sr_co_call (new_ibp);
    sr_co_call_done (new_ibp);
}



/*
 *  Wait for a co invocation to terminate.  Return a pointer to the original
 *  invocation block so that the GC can copy result parameters and
 *  find out which arm terminated.
 */
Ptr
sr_co_wait (locn)
char *locn;
{
    Cob cobp;
    Invb ibp;

    CUR_PROC->locn = locn;		/* add locn to CUR_PROC structure */

    sr_check_stk (CUR_STACK);

    cobp = CUR_PROC->cob_list;
    LOCK (cobp->cobmutex, "sr_co_wait");

    if (cobp->pending == 0 && cobp->done_list == NULL)
	ibp = NULL;
    else {
	UNLOCK (cobp->cobmutex, "sr_co_wait");
	P ((char *) 0, cobp->done);

	LOCK (cobp->cobmutex, "sr_co_wait");

	/*  Very rarely, done_list will be empty here.
	 *  This should not happen, but the bug is elusive.  */
	if (cobp->done_list == NULL) 
	    sr_malf ("intermittent sr_co_wait bug---rerun program");
	ibp = cobp->done_list;
	cobp->done_list = ibp->co.next;
    }

    UNLOCK (cobp->cobmutex, "sr_co_wait");
    return (Ptr) ibp;
}



/*
 *  A co statement has terminated.  Release the co block.
 */
void
sr_co_end (locn)
char *locn;
{
    Cob cobp;
    Invb ibp;

    sr_check_stk (CUR_STACK);

    cobp = CUR_PROC->cob_list;
    CUR_PROC->cob_list = cobp->next;

    TRACE ("OC", locn, 0);

    /*  this is the only place that there is a nesting between any co
     *  locks, so any order we choose here is fine */
    LOCK (cobp->cobmutex, "sr_co_end");
    cobp->seqn++;
    sr_kill_sem (cobp->done);

/*
 *  Free any invocation blocks that were returned after
 *  the last sr_co_wait () was done.
 */
    while (ibp = cobp->done_list) {
	cobp->done_list = ibp->co.next;
	/* note this ipb can never belong to a resource. */
	sr_free ((Ptr) ibp);
    }

    UNLOCK (cobp->cobmutex, "sr_co_end");
    sr_delpool (cob_pool, (Ptr) cobp);
}


/*
 *  Initialize co statement part of SR RTS.
 */
void
sr_init_co ()
{
    cob_pool = sr_makepool ("co-block descriptors", sizeof (struct cob_st),
				sr_max_co_stmts, init_cob, re_init_cob);
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

/*ARGSUSED*/ /*(under MultiSR they are)*/
static void
re_init_cob (cb)
Cob cb;
{
    RESET_LOCK (cb->cobmutex);
}
