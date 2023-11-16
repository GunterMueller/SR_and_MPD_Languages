/*  invoke.c -- invocation and proc termination routines  */

#include "rts.h"

static void op_spawn ();
static void semarray ();



/*
 *  Invoke a proc or input operation by either call or send.
 */
Ptr
sr_invoke (locn, ibp)
char *locn;
Invb ibp;
{
    Oper op;
    Pach ph;
    Sem ibpsem;
    Invb newibp;

    if (locn)
	CUR_PROC->locn = locn;		/* remember source location */
    sr_check_stk (CUR_STACK);
    DEBUG (D_INVOKE, "%-12.12s invoke%ld %08lX", CUR_PROC->pname,ibp->type,ibp);

    ibp->ph.priority = CUR_PROC->priority;  /* copy invokers's priority */

    switch (ibp->type) {

	case CALL_IN:
	case COCALL_IN:
	case REM_COCALL_IN:
	    TRACE ("CALL", locn, 0);
	    ibp->replied = FALSE;	/* have not replied, will need to */
	    ibp->discard = FALSE;	/* caller needs the ibp back */
	    break;

	case COSEND_IN:
	    sr_co_send (ibp);		/* copy block */
	    ibp->type = SEND_IN;	/* convert to SEND */
	    /*FALLTHROUGH*/
	case SEND_IN:
	    TRACE ("SEND", locn, 0);
	    ibp->replied = TRUE;	/* that is, needs no further reply */
	    ibp->discard = TRUE;	/* discard ibp when op is done */
	    break;

	default:
	    sr_malf ("unexpected ibp type in invoke");
    }

    ibp->forwarded = FALSE;		/* this was not forwarded */
    ibp->wait = NULL;			/* ensure null so can be tested */
    ibp->ibpret = NULL;			/* will be set if needed */
    if (locn) {
	ibp->invoker = CUR_PROC;	/* save the invoker's proc id */
	ibp->forwarder = CUR_PROC;	/* save the forwarder's proc id */
    }

    /*
     *	Check for null or noop capability.
     */
    if (ibp->opc.oper_entry == 0) {
	if (ibp->opc.seqn == NOOP_SEQN) {
	    if (ibp->type == SEND_IN) {
		sr_free ((Ptr) ibp);
		return NULL;
	    }
	    if (ibp->type == COCALL_IN) {
		/* Simulate the normal actions for a co call. */
		sr_co_call (ibp);
		sr_co_call_done (ibp);
	    }
	    return (Ptr) ibp;
	} else
	    sr_loc_abort (locn, "invalid operation capability");
    }

    if (ibp->opc.vm != sr_my_vm) {

	if (ibp->opc.vm <= 0 || ibp->opc.vm > MAX_VM || !sr_exec_up)
	    sr_loc_abort (locn, "invalid operation capability");

	/* Send invocation request to remote machine.  */

	if (ibp->type == COCALL_IN) {
	    sr_co_call (ibp);
	    ibp->type = REM_COCALL_IN;
	}

	ph = (Pach) ibp;
	ibp = (Invb) sr_remote (ibp->opc.vm, REQ_INVOKE, ph, ph->size);
	sr_free ((Ptr) ph);
	return (Ptr) ibp;
    }

    op = (Oper) ibp->opc.oper_entry;
    if (ibp->opc.seqn != op->seqn)
	sr_loc_abort (locn,
	    "attempting to invoke operation that no longer exists");

    if (op->type == INPUT_OP || op->type == DYNAMIC_OP)

	switch (ibp->type) {
	    case CALL_IN:
	    case REM_COCALL_IN:
		ibpsem = ibp->wait = sr_make_sem (0);
		ibp->ibpret = &newibp;
		sr_invk_iop (ibp, op->u.clap);
		P ((char *) NULL, ibpsem);
		sr_kill_sem (ibpsem);
		ibp = newibp;
		break;
		
	    case SEND_IN:
		sr_invk_iop (ibp, op->u.clap);
		break;
		
	    case COCALL_IN:
		sr_co_call (ibp);
		sr_invk_iop (ibp, op->u.clap);
		break;
	}

    else				/* PROC_OP or PROC_SEP_OP */
	switch (ibp->type) {

	    case CALL_IN:
	    case REM_COCALL_IN:
		/* Do direct call if proc does not require separate context */
		if (op->type != PROC_SEP_OP) {
		    enum in_type old_itype = CUR_PROC->itype;
		    CUR_PROC->itype = CALL_IN;
		    (*op->u.code) (op->res->crb_addr,
			op->res->rv_base, ibp, RTS_CALL);
		    CUR_PROC->itype = old_itype;
		} else {
		    ibpsem = ibp->wait = sr_make_sem (0);
		    ibp->ibpret = &newibp;
		    op_spawn (ibp, op, CALL_IN);
		    P ((char *) NULL, ibpsem);
		    sr_kill_sem (ibpsem);
		    ibp = newibp;
		}
		break;

	    case SEND_IN:
		op_spawn (ibp, op, SEND_IN);
		break;

	    case COCALL_IN:
		sr_co_call (ibp);
		op_spawn (ibp, op, COCALL_IN);
		break;
	}

    return (Ptr) ibp;
}



/*
 *  Forward the responsibility for a reply (and an argument list) to another op.
 */
Ptr
sr_forward (locn, obp, ibp)
char *locn;
Invb obp;	/* old invocation block */
Invb ibp;	/* new invocation block */
{
    Oper op;

    sr_check_stk (CUR_STACK);
    DEBUG (D_INVOKE, "%-12.12s forward %08lX -> %08lX",
	CUR_PROC->pname, obp, ibp);

    ibp->ph = obp->ph;				/* copy packet header */
    ibp->ph.priority = CUR_PROC->priority;	/* but use curr priority */
    ibp->type = obp->type;
    ibp->forwarded = TRUE;
    ibp->forwarder = CUR_PROC;
    ibp->invoker = obp->invoker;
    ibp->co = obp->co;
    ibp->wait = obp->wait;
    ibp->ibpret = obp->ibpret;
    ibp->replied = obp->replied;	/* transfer responsibility for reply */
    ibp->discard = obp->discard;	/* and for disposal */
    obp->replied = TRUE;
    obp->discard = TRUE;
    obp->ph.rem = NULL;
    obp->ibpret = NULL;

    TRACE ("FORWARD", locn, obp->invoker);
    DEBUG (D_INVOKE, "%-12.12s %08lX forward %08lX", CUR_PROC->pname, obp, ibp);

    if (ibp->opc.oper_entry == 0) {
	/*
	 *  Forward to null or noop capability
	 */
	if (ibp->opc.seqn == NOOP_SEQN) {
	    if (ibp->ibpret)
		*ibp->ibpret = ibp;
	    if (!ibp->replied)
		V (ibp->wait);
	    if (ibp->discard)
		sr_free ((Ptr) ibp);
	    return NULL;
	} else
	    sr_loc_abort (locn, "attempting to forward to null operation");
    }

    if (ibp->opc.vm != sr_my_vm) {
	/*
	 * Forward to remote vm
	 */
	if (ibp->opc.vm <= 0 || ibp->opc.vm > MAX_VM || !sr_exec_up)
	    sr_loc_abort (locn, "invalid operation capability");
	sr_remote (ibp->opc.vm, REQ_INVOKE, (Pach) ibp, ibp->ph.size);
	sr_free ((Ptr) ibp);

    } else {
	/*
	 * Forward locally
	 */
	op = (Oper) ibp->opc.oper_entry;
	if (ibp->opc.seqn != op->seqn)
	    sr_loc_abort (locn,
		"attempting to forward to operation that no longer exists");

	if (op->type == INPUT_OP || op->type == DYNAMIC_OP)
	    sr_invk_iop (ibp, op->u.clap);		/* to input op */
	else
	    op_spawn (ibp, op, ibp->type);		/* to proc */
    }

    return NULL;
}



/*
 *  sr_init_semop (locn, dest, src, ndim) - initialize non-optimized semaphores.
 *
 *  dest is the address of an opcap or array of opcaps.
 *  src  is the address of an integer or array of integers.
 *  ndim is the number of dimensions.
 */
void
sr_init_semop (locn, dest, src, ndim)
char *locn;
Ptr dest, src;
int ndim;
{
    Invb ibp;
    int n;

    if (ndim > 0) {
	semarray (locn, (Array *) dest, (Array *) src);
	return;
    }

    n = * (Int *) src;				/* get initial value */
    if (n < 0)					/* must be nonnegative */
	sr_runerr (locn, E_SEMV, n);
    while (--n >= 0) {
	/* construct invocation block */
	ibp = (Invb) sr_alc (sizeof (struct invb_st), 1);
	ibp->ph.size = sizeof (*ibp);
	ibp->type = SEND_IN;
	ibp->opc = * (Ocap *) dest;
	sr_invoke (locn, ibp);			/* issue the send */
    }
}

/*
 * semarray (locn, dstp, srcp) -- initialize array and advance pointers.
 */
static void
semarray (locn, dstp, srcp)
char *locn;
Array *dstp, *srcp;
{
    Ocap *d = (Ocap *) ADATA (dstp);
    Int *s = (Int *) ADATA (srcp);
    int i;

    for (i = 0; i < dstp->ndim; i++)
	if (UB (dstp, i) - LB (dstp, i) != UB (srcp, i) - LB (srcp, i))
	    sr_runerr (locn, E_ASIZ, ((Dim*)(d+1))[i], ((Dim*)(s+1))[i]);

    for (i = sr_acount (dstp); i--; )
	sr_init_semop (locn, (Ptr) d++, (Ptr) s++, 0);
}



/*
 *  Create a new process to service a proc operation invocation.
 */
static void
op_spawn (ibp, op, type)
Invb ibp;
Oper op;
enum in_type type;
{
    Proc pr;

    /*  We must grab the rmutex first, since we have to grab the queue
     *  mutex.  We need this since sr_spawn puts the new proc on
     *  its resource list.  However, a sr_destroy could try to kill
     *  the new thread before sr_activate below puts it on the
     *  ready queue.  This would be a problem, since sr_kill would not
     *  find it there and would abort.
     */

    if (op->res != (Rinst) NULL)
	LOCK (op->res->rmutex, "op_spawn");
    LOCK_QUEUE ("op_spawn");

    pr = sr_spawn (op->u.code, ibp->ph.priority, op->res, TRUE,
	    (long) op->res->crb_addr, (long) op->res->rv_base,
	    (long) ibp, (long) RTS_CALL);
    pr->ptype = PROC_PR;
    pr->itype = type;
    pr->pname = "[baby proc]";		/* GC will reset */
    sr_activate (pr);

    UNLOCK_QUEUE ("op_spawn");
    if (op->res != (Rinst) NULL)
	UNLOCK (op->res->rmutex, "op_spawn");
}



/*
 *  Send an early reply to the invoker of an operation.
 *  Copy invocation block so invoker and invokee do not share
 *  the same argument area.  Return pointer to new copy.
 *  Also does replies for initial/final code (indicated by null
 *  ibp).  Copy resource capability when replying in initial.
 */
Invb
sr_reply (locn, ibp)
char *locn;
Invb ibp;
{
    Ptr src, dest;
    Oper op;
    Invb new_ibp;

    sr_check_stk (CUR_STACK);

    if (ibp == NULL) {
	/*
	 *  The reply is in initialization or finalization code.
	 *  Act like sr_finished_init () or sr_finished_final ().
	 */
	DEBUG (D_INVOKE, "%-12.12s reply   %08lX", CUR_PROC->pname, ibp, 0);
	if (CUR_PROC->ptype == INITIAL_PR) {

	    LOCK (CUR_RES->rmutex, "sr_reply");
	    if (CUR_RES->status & INIT_REPLY) {
		UNLOCK (CUR_RES->rmutex, "sr_reply");
		RTS_WARN ("ignoring extra reply in body");
		return NULL;
	    }

	    if ((dest = (Ptr) CUR_RES->rcp) != NULL) {
		src = CUR_RES->rv_base;
		while (CUR_RES->rc_size--)
		    *dest++ = *src++;
	    }

	    CUR_RES->status |= INIT_REPLY;
	    UNLOCK (CUR_RES->rmutex, "sr_reply");
	    V (CUR_PROC->wait);
	    return NULL;

	} else if (CUR_PROC->ptype == FINAL_PR) {

	    RTS_WARN ("reply in final ignored");
	    return NULL;

	} else
	    sr_malf ("no ibp for reply");
    }

    /*
     *  It's a normal reply from a proc.
     */
    TRACE ("REPLY", locn, ibp->invoker);

    /*
     *  If no reply is desired (send invocation | already replied) just return.
     */
    if (ibp->replied) {
	DEBUG (D_INVOKE, "%-12.12s reply   %08lX ignored",
	    CUR_PROC->pname, ibp, 0);
	return ibp;
    }

    /*
     *	Create a new ibp and copy the old invocation to the new.
     *  Make it look like a send invocation now that we've replied.
     *  This will get it automatically freed later.
     */
    ibp->replied = TRUE;
    new_ibp = sr_dup_invb (ibp);
    new_ibp->type = SEND_IN;		/* make it look like a send */
    new_ibp->discard = TRUE;
    new_ibp->wait = NULL;
    new_ibp->ibpret = NULL;
    new_ibp->invoker = CUR_PROC;

    /*  there is no need to lock this op, since we just read something
     *  that will not change.
     */
    DEBUG (D_INVOKE, "%-12.12s reply   %08lX -> %08lX",
	CUR_PROC->pname, ibp, new_ibp);

    op = (Oper) ibp->opc.oper_entry;

    switch (op->type) {
	case INPUT_OP:
	case DYNAMIC_OP:
	case PROC_SEP_OP:
	    switch (ibp->type) {
		case CALL_IN:
		case REM_COCALL_IN:
		    if (ibp->ibpret)
			*ibp->ibpret = ibp;
		    V (ibp->wait);
		    break;
		case COCALL_IN:
		    sr_co_call_done (ibp);
		    break;
		case SEND_IN:
		    sr_malf ("reply found a waiting send");
	    }
	    break;
	case PROC_OP:
	    sr_malf ("unexpected reply in supposedly safe proc");
	default:
	    sr_malf ("invalid operation type in reply");
    };
    return new_ibp;
}



/*
 *	An input operation has finished.  Clean up.
 */
void
sr_finished_input (locn, ibp)
char *locn;
Invb ibp;
{
    sr_check_stk (CUR_STACK);
    DEBUG (D_INVOKE, "%-12.12s fin_inp %08lX", CUR_PROC->pname, ibp, 0);
    TRACE ("NI", locn, ibp->invoker);
    switch (ibp->type) {

	case CALL_IN:
	case REM_COCALL_IN:
	    if (ibp->ibpret)
		*ibp->ibpret = ibp;
	    if (! ibp->replied)
		V (ibp->wait);
	    if (ibp->discard)
		sr_free ((Ptr) ibp);
	    return;
	
	case SEND_IN:
	    sr_free ((Ptr) ibp);
	    return;
	
	case COCALL_IN:
	    if (! ibp->replied)
		sr_co_call_done (ibp);
	    return;
    }
}



/*
 *  A proc operation has finished.  Release the invocation block if appropriate.
 *  If the proc was called, notify the invoker.  Commit suicide.
 */
void
sr_finished_proc (ibp)
Invb ibp;
{
    Oper op;

    sr_check_stk (CUR_STACK);
    DEBUG (D_INVOKE, "             fin_prc %08lX", ibp, 0, 0);

    switch (CUR_PROC->itype) {
	case CALL_IN:
	case REM_COCALL_IN:
	    op = (Oper) ibp->opc.oper_entry;
	    if (op->type != PROC_SEP_OP && !ibp->forwarded)
		return;			/* was run in same process */
	    if (ibp->ibpret)
		*ibp->ibpret = ibp;
	    if (! ibp->replied)
		V (ibp->wait);		/* reply now if didn't before */
	    if (ibp->discard)
		sr_free ((Ptr) ibp);	/* free if unwanted */
	    break;

	case SEND_IN:
	    sr_free ((Ptr) ibp);
	    break;

	case COCALL_IN:
	    if (! ibp->replied)
		sr_co_call_done (ibp);
	    break;
    }

    sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Reject an invocation because the operation was killed
 *  before the invocation was accepted.
 *  Must be called when we possess the res mutex (this is true
 *  with all present RTS calls).
 */
void
sr_rej_inv (ibp)
Invb ibp;
{
    Oper op;

    op = (Oper) ibp->opc.oper_entry;
    if (op->type == PROC_OP || op->type == PROC_SEP_OP)
	sr_malf ("rejecting a proc op");

    switch (ibp->type) {
	case CALL_IN:
	case REM_COCALL_IN:
	    /* indicate rejection in status field */
	    if (ibp->ibpret)
		*ibp->ibpret = ibp;
	    V (ibp->wait);
	    return;

	case SEND_IN:
	    /* we already possess the res rmutex */
	    sr_locked_free ((Ptr) ibp);
	    return;

	case COCALL_IN:
	    /* indicate rejection in status field */
	    sr_co_call_done (ibp);
	    return;
    }
}


/*
 *  Duplicate an invocation block and return the address of the copy.
 */
Invb
sr_dup_invb (ibp)
Invb ibp;
{
    Invb new;
    int n;

    n = ibp->ph.size;
    new = (Invb) sr_alc (n, 1);
    memcpy ((Ptr) new, (Ptr) ibp, n);
    return new;
}
