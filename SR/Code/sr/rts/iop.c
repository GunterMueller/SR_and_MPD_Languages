/*  iop.c -- runtime support of the input statement  */

#include "rts.h"

static void move_in ();



/*
 *  Append an invocation block to the
 *  end of the specified invocation list.
 *  Since the only place append, appendlist, and delete_inv are used
 *  is where exclusive access to a class has been obtained, we don't
 *  need to grab the queue's lock.
 */
#define append(ibp, ilist)	{ \
    if (((ilist).head) == NULL) { \
	(ilist).head = (ibp); \
	(ilist).tail = (ibp); \
	(ibp)->last = NULL; \
    } else { \
	(ilist).tail->next = (ibp); \
	(ibp)->last = (ilist).tail; \
	(ilist).tail = (ibp); \
    } \
}



/*
 *  Append an invocation list to the
 *  end of the specified invocation list.
 */
#define appendlist(ibl, ilist)	{ \
    if (((ilist).head) == NULL) \
	(ilist) = (ibl); \
    else { \
	(ilist).tail->next = (ibl).head; \
	(ibl).head->last = (ilist).tail; \
	(ilist).tail = (ibl).tail; \
    } \
}



/*  Remove a node from a single-headed doubly-linked list.  */

#define delete_inv(node, queue, next, last) { \
    if (node->last == NULL) \
	if (((queue).head = node->next) != NULL) \
	    (queue).head->last = NULL; \
	else \
	    (queue).tail = NULL; \
    else if ((node->last->next = node->next) != NULL) \
	node->next->last = node->last; \
    else \
	(queue).tail = node->last; \
}



/*
 *  Invoke an input operation.  Place the invocation block on the appropriate
 *  list.  Wake up process if new invocation could possibly satisfy guard in
 *  its input statement.
 */
void
sr_invk_iop (ibp, clap)
Invb ibp;
Class clap;
{
    Proc pr;
    Oper op;

    ibp->next = NULL;
    LOCK (clap->clmutex, "sr_invk_iop");
    if (clap->inuse)
	append (ibp, clap->new_in)
    else {
	op = (Oper) ibp->opc.oper_entry;
	LOCK (op->omutex, "sr_invk_iop");
	op->pending++;
	UNLOCK (op->omutex, "sr_invk_iop");
	append (ibp, clap->old_in)
	/* keep blocked_on accurate! */
	LOCK_QUEUE ("sr_invk_iop");
	for (pr = clap->old_pr.head; pr; pr = pr->next)
	    pr->blocked_on = &clap->new_pr;
	UNLOCK_QUEUE ("sr_invk_iop");

	if (clap->new_pr.head == NULL)
	    clap->new_pr.head = clap->old_pr.head;
	else
	    clap->new_pr.tail->next = clap->old_pr.head;
	clap->new_pr.tail = clap->old_pr.tail;
	clap->old_pr.head = NULL;
	clap->old_pr.tail = NULL;
	
	if (clap->new_pr.head != NULL) {
	    clap->inuse = TRUE;
	    clap->new_pr.head->next_inv = clap->old_in.head;
	    if (clap->new_pr.head->else_leg) {
		clap->else_pr = clap->else_pr->next_else;
		if (clap->else_pr == NULL)
		    clap->else_tailpr = NULL;
	    }
	    awaken (clap->new_pr);
	}
    }
    UNLOCK (clap->clmutex, "sr_invk_iop");
}



/*
 *  Gain initial access to an input operation class.
 *  Allows GC to start searching for a valid invocation.
 */
void
sr_iaccess (clap, else_present)
Class clap;
Bool else_present;
{

    sr_check_stk (CUR_STACK);

    LOCK (clap->clmutex, "sr_iaccess");
    CUR_PROC->else_leg = else_present;
    if (clap->inuse) {
	LOCK_QUEUE ("sr_iaccess");	/* protect block-scheduler */
	block (&clap->new_pr);
	if (CUR_PROC->else_leg) {
	    if (clap->else_pr == NULL) {
		clap->else_pr = CUR_PROC;
		clap->else_tailpr = CUR_PROC;
	    } else {
		clap->else_tailpr->next_else = CUR_PROC;
		clap->else_tailpr = CUR_PROC;
	    }
	    clap->else_tailpr->next_else = NULL;
	}
	UNLOCK (clap->clmutex, "sr_iaccess");
	sr_scheduler ();
    } else {
	clap->inuse = TRUE;
	CUR_PROC->next_inv = clap->old_in.head;
	UNLOCK (clap->clmutex, "sr_iaccess");
    }
}



/*
 *  Regain subsequent access to an input operation class.
 */
void
sr_reaccess (clap)
Class clap;
{
    LOCK (clap->clmutex, "sr_reaccess");
    if (clap->new_in.head == NULL) {
	/*
	 *	No new invocations for me, so block.  Give access
	 *	to class if any other process wants to see it.
	 */
	LOCK_QUEUE ("sr_reaccess #1");/* protect block-awaken-scheduler */
	block (&clap->old_pr);
	if (clap->new_pr.head == NULL)
	    clap->inuse = FALSE;
	else {
	    clap->new_pr.head->next_inv = clap->old_in.head;
	    if (clap->new_pr.head->else_leg) {
		clap->else_pr = clap->else_pr->next_else;
		if (clap->else_pr == NULL)
		    clap->else_tailpr = NULL;
	    }
	    awaken (clap->new_pr);
	}
	UNLOCK (clap->clmutex, "sr_reaccess");
	sr_scheduler ();

    } else {
	/*
	 *	New invocations have arrived since the last time
	 *	this process searched the invocation queue.
	 *	Oldest process doing a reaccess () gets to look at
	 *	next invocation, proceeding in FCFS order until it's
	 *	my turn again.  If no older processes, just go around.
	 */
	
	move_in (clap);

	if ((clap->old_pr.head) != NULL) {
	    Proc pr;
	    /* protect block-awaken-scheduler, plus blocked_on */
	    LOCK_QUEUE ("sr_reaccess #2");
	    block (&clap->old_pr);
	    for (pr = clap->old_pr.head; pr; pr = pr->next)
		pr->blocked_on = &clap->new_pr;

	    clap->old_pr.tail->next = clap->new_pr.head;
	    clap->new_pr.head = clap->old_pr.head;
	    if (clap->new_pr.tail == NULL)
		clap->new_pr.tail = clap->old_pr.tail;
	    clap->old_pr.head = NULL;
	    clap->old_pr.tail = NULL;
	    clap->new_pr.head->next_inv = clap->old_in.head;

	    if (clap->new_pr.head->else_leg) {
		clap->else_pr = clap->else_pr->next_else;
		if (clap->else_pr == NULL)
		    clap->else_tailpr = NULL;
	    }
	    awaken (clap->new_pr);
	    UNLOCK (clap->clmutex, "sr_reaccess");
	    sr_scheduler ();
	} else {
	    CUR_PROC->next_inv = clap->old_in.head;
	    UNLOCK (clap->clmutex, "sr_reaccess");
	}
    }
}



/*
 *  Check that o is a legal, local op cap for use in an input statement.
 *  Return TRUE if it matches the op entry oentry.
 */
Bool
sr_cap_ck (locn, oentry, o)
char *locn;
Ptr oentry;
Ocap o;
{
    sr_check_stk (CUR_STACK);
    if (o.oper_entry == 0) {
	if (o.seqn == NOOP_SEQN)
	    return FALSE;	/* noop is legal but never matches */
	else
	    sr_loc_abort (locn, "input from null capability");
    }
    if (o.vm != sr_my_vm) {		/* if remote */
	if (o.vm <= 0 || o.vm > MAX_VM || !sr_exec_up)
	    sr_loc_abort (locn, "invalid operation capability");
	sr_loc_abort (locn, "illegal use of remote capability");
    }
    return oentry == o.oper_entry;
}



/*
 *  Remove an invocation block from the specified input
 *  operation queue.  The GC can service the invocation now.
 */
void
sr_rm_iop (locn, cp, ibp)
char *locn;
Ptr cp;
Invb ibp;
{
    Class clap;
    Oper op;

    sr_check_stk (CUR_STACK);
    DEBUG (D_INVOKE, "%-12.12s in      %08lX c%08lX", CUR_PROC->pname, ibp, cp);
    clap = (Class) cp;
    LOCK (clap->clmutex, "sr_rm_iop");

    if (!ibp)
	TRACE ("ARM", locn, CUR_PROC);		/* leg */
    else if (ibp->forwarded)
	TRACE ("ARM", locn, ibp->forwarder);	/* forward request */
    else
	TRACE ("ARM", locn, ibp->invoker);		/* normal request */

    /*
     *  remove the invocation, unless this is the "else" leg and there isn't one
     */
    if (ibp != NULL) {
	op = (Oper) ibp->opc.oper_entry;
	LOCK (op->omutex, "sr_rm_iop");
	op->pending--;
	delete_inv (ibp, clap->old_in, next, last);
	UNLOCK (op->omutex, "sr_rm_iop");
    }

    /*
     *  Handle any other processes waiting for this class.
     */
    if (clap->new_in.head == NULL) {
	/*
	 * No new invocations.  Give access to class if any other process
	 * wants to see it.
	 */
	if (clap->old_in.head == NULL) {
	    if (clap->else_pr == NULL) {
		if (clap->new_pr.head) {
		    Proc pr;
		    /* keep blocked_on accurate! */
		    LOCK_QUEUE ("sr_rm_iop");
		    for (pr = clap->new_pr.head; pr; pr = pr->next)
			pr->blocked_on = &clap->old_pr;
		    UNLOCK_QUEUE ("sr_rm_iop");

		    if (clap->old_pr.head)
			clap->old_pr.tail->next = clap->new_pr.head;
		    else
			clap->old_pr.head = clap->new_pr.head;
		    clap->old_pr.tail = clap->new_pr.tail;
		    clap->new_pr.head = NULL;
		    clap->new_pr.tail = NULL;
		}
		clap->inuse = FALSE;

	    } else {
		/*
		 * NEVER AN ELSE ON THE OLD_PR
		 *	move processes from new process list to old process
		 *	list until reach a blocked process with an else
		 *	be sure to keep blocked_on accurate!
		 */
		Proc pr;
		pr = clap->new_pr.head;
		if (!pr->else_leg) {
		    LOCK_QUEUE ("sr_rm_iop");
		    while ((pr->next) &&  pr->next->else_leg == FALSE) {
			pr->blocked_on = &clap->old_pr;
			pr = pr->next;		
		    }
		    pr->blocked_on = &clap->old_pr;
		    UNLOCK_QUEUE ("sr_rm_iop");

		    if (clap->old_pr.head) {
			clap->old_pr.tail->next = clap->new_pr.head;
			clap->old_pr.tail = pr;
		    } else {
			clap->old_pr.head = clap->new_pr.head;
			clap->old_pr.tail = pr;
		    }
		    clap->new_pr.head = pr->next;
		    pr->next = NULL;
		}

		/* first proc with an else is now in clap->new_pr.head */
		clap->else_pr = clap->else_pr->next_else;
		if (clap->else_pr == NULL)
			clap->else_tailpr = NULL;
		clap->new_pr.head->next_inv = NULL;
		awaken (clap->new_pr);
	    }

	} else {
	    if (clap->new_pr.head == NULL)
		clap->inuse = FALSE;
	    else {
		clap->new_pr.head->next_inv = clap->old_in.head;
		if (clap->new_pr.head->else_leg) {
		    clap->else_pr = clap->else_pr->next_else;
		    if (clap->else_pr == NULL)
			clap->else_tailpr = NULL;
		}
		awaken (clap->new_pr);
	    }
	}

    } else {
	/*
	 *  New invocations have arrived. Does any process need to look at them?
	 */

	move_in (clap);

	if (clap->old_pr.head) {
	    Proc pr;
	    /* keep blocked_on accurate! */
	    LOCK_QUEUE ("sr_rm_iop");
	    for (pr = clap->old_pr.head; pr; pr = pr->next)
		pr->blocked_on = &clap->new_pr;
	    UNLOCK_QUEUE ("sr_rm_iop");

	    if (clap->new_pr.head)
		clap->old_pr.tail->next = clap->new_pr.head;
	    else
		clap->new_pr.tail = clap->old_pr.tail;

	    clap->new_pr.head = clap->old_pr.head;
	    clap->old_pr.head = NULL;
	    clap->old_pr.tail = NULL;
	}

	if (clap->new_pr.head) {
	    clap->new_pr.head->next_inv = clap->old_in.head;
	    if (clap->new_pr.head->else_leg) {
		    clap->else_pr = clap->else_pr->next_else;
		    if (clap->else_pr == NULL)
			clap->else_tailpr = NULL;
	    }
	    awaken (clap->new_pr);
	} else
	    clap->inuse = FALSE;
    }
    UNLOCK (clap->clmutex, "sr_rm_iop");
}


/*
 *  Move all invocations of the specified class from class's new invocation
 *  list to its old invocation list.
 *  For each invocation on the new list, update its operation's pending.
 */
static void
move_in (clap)
Class clap;
{
    Invb ibp;
    Oper op;

    for (ibp = clap->new_in.head; ibp; ibp = ibp->next) {
	op = (Oper) ibp->opc.oper_entry;
	LOCK (op->omutex, "move_in");
	op->pending++;
	UNLOCK (op->omutex, "move_in");
    }
    appendlist (clap->new_in, clap->old_in);
    clap->new_in.head = NULL;
    clap->new_in.tail = NULL;
}
