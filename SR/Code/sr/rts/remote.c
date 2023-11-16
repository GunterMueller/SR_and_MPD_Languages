/*  remote.c -- remote request processing  */

#include "rts.h"

static void contact ();

static Mutex remote_mutex;	  /* protect started, waiting[] in contact()*/



/*
 *  Initialize free list of pending remote request message descriptors.
 */
void
sr_init_rem ()
{
    INIT_LOCK (remote_mutex, "known_mutex");
}



/*
 *  Send a request to a remote machine and wait for the reply message.
 *  Return pointer to reply packet, or NULL if there is none.
 *
 *  This is never called with a rmutex held.
 */
Pach
sr_remote (dest, type, ph, size)
Vcap dest;
enum ms_type type;
Pach ph;
int size;
{
    struct remd_st rem;
    Invb ibp;

    if (! (sr_net_known ((int) dest)))
	contact ((int) dest);		/* establish contact if none yet made */

    ph->priority = CUR_PROC->priority;

    ibp = (Invb) ph;
    if (type == REQ_INVOKE && (ibp->replied || ibp->type == REM_COCALL_IN)) {

	/* sends and forwards are not ack'd, and we don't wait on a cocall */
	sr_net_send (dest, type, ph, size);
	return NULL;

    } else {

	/* for all other messages, await acknowledgement */

	ph->rem = &rem;
	rem.wait = sr_make_sem (0);
	
	sr_net_send (dest, type, ph, size);
	P ((char *) NULL, rem.wait);		/* wait for reply */
	sr_kill_sem (rem.wait);

	return rem.reply;
    }
}



/*
 *  Service a request to establish a connection.
 *  Executes as a separate process.
 */
void
sr_rmt_callme (ph)
Pach ph;
{
    int n = ((struct num_st *) ph) ->num;
    contact (n);
    sr_net_send (n, ACK_CALLME, ph, PACH_SZ);
    sr_free ((Ptr) ph);
    sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to create a resource from a remote machine.
 *  Executes as separate process.
 */
void
sr_rmt_create (ph)
Pach ph;
{
    int size;
    struct crb_st *crbp;
    struct rres_st *reply;

    crbp = (struct crb_st *) ph;
    size = sizeof (struct rres_st) - sizeof (Rcap) + crbp->rc_size;
    reply = (struct rres_st *) sr_alc (-size, 1);

    crbp->rcp = & reply->rc;
    sr_create_res (crbp);

    reply->ph.rem    = ph->rem;
    sr_net_send (ph->origin, ACK_CREATE, &reply->ph, size);

    sr_free ((Ptr) reply);
    /* ph freed when resource destroyed */

    sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to query a pending op count from a remote machine.
 *  Executes as a separate process.
 */
void
sr_rmt_query (ph)
Pach ph;
{
    ((struct num_st *) ph)->num = 
	sr_query_iop ((char *) NULL, ((struct ropc_st *) ph)->oc);
    sr_net_send (ph->origin, ACK_COUNT, ph, ph->size);
    sr_free ((Ptr) ph);
    sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to destroy an op from a remote machine.
 *  Executes as separate process.
 */
void
sr_rmt_destop (ph)
Pach ph;
{
    sr_dest_op ((char *) NULL, ((struct ropc_st *) ph)->oc);
    sr_net_send (ph->origin, ACK_DESTOP, ph, ph->size);
    sr_free ((Ptr) ph);
    sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to destroy a resource from a remote machine.
 *  Executes as separate process.
 */
void
sr_rmt_destroy (ph)
Pach ph;
{
    sr_destroy ((char *) NULL, ((struct rres_st *) ph)->rc);
    sr_net_send (ph->origin, ACK_DESTROY, ph, ph->size);
    sr_free ((Ptr) ph);
    sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request from srx to destroy this virtual machine.
 *  Executes as a separate process.
 */
void
sr_rmt_destvm (ph)
Pach ph;
{
    sr_dest_all ();		/* destroy all non-global resources */
    sr_destroy_globals ();	/* destroy all global resources */

    sr_net_send (ph->origin, ACK_DESTVM, ph, ph->size);	/* ack destroy */
    sr_net_send (SRX_VM, MSG_IDLE,			/* send msg counts */
	&sr_msg_counts.ph, sizeof (sr_msg_counts));
    EXIT (0);						/* kill self */
}



/*
 *  Service a request to receive an operation from a remote machine.
 *  Executes as separate process.
 */
void
sr_rmt_rcv (ph)
Pach ph;
{
    Sem sp;
    Invb ibp, *ret;
    struct ropc_st *p;
    struct remd_st rem;

    p = (struct ropc_st *) ph;
    ibp = (Invb) sr_receive ((char *) 0, p->oc, p->elseflag);

    if (ibp == NULL) {
	/*
	 * No invocation available. Return original packet.
	 */
	sr_net_send (ph->origin, ACK_RECEIVE, ph, ph->size);
	sr_free ((Ptr) ph);
	sr_kill (CUR_PROC, (Rinst) NULL);
    }

    /*
     * Got an invocation.  If it needs no reply, send it and disappear.
     */
    if (ibp->discard) {
	ibp->ph.rem = ph->rem;
	sr_net_send (ph->origin, ACK_RECEIVE, (Pach) ibp, ibp->ph.size);
	sr_free ((Ptr) ibp);
	sr_free ((Ptr) ph);
	sr_kill (CUR_PROC, (Rinst) NULL);
    }

    /*
     * The invocation needs a reply.  Send MSG_RCVCALL and wait.
     */
    rem.wait = sr_make_sem (0);		/* set up reply block */
    ibp->ph.rem = &rem;			/* put addr in invocation block */
    ibp->next = (Invb) ph->rem;		/* pass origl reply addr via "next" */
    sp = ibp->wait;			/* invoker's semaphore */
    ret = ibp->ibpret;			/* invoker's ibp pointer */

    sr_net_send (ph->origin, MSG_RCVCALL, (Pach) ibp, ibp->ph.size);
    sr_free ((Ptr) ph);
    sr_free ((Ptr) ibp);

    P ((char *) NULL, rem.wait);	/* wait for reply */
    sr_kill_sem (rem.wait);

    ph = rem.reply;			/* get reply address */
    ibp = (Invb) ph;

    if (ret)
	*ret = ibp;			/* tell invoker new ibp address */
    V (sp);				/* awaken invoker */

    sr_kill (CUR_PROC, (Rinst) NULL);	/* kill self */
}



/*
 *  Service a MSG_RCVCALL received in response to a REQ_RECEIVE that found
 *  a call needing a reply.  Reply to the RECEIVE requestor and pass it the
 *  invocation block; wait for the call to reply; then pass back the reply
 *  to the original VM.  Executes as a separate process.
 */
void
sr_rcv_call (ph)
Pach ph;
{
    Invb ibp;
    Remd r;

    ibp = (Invb) ph;
    ibp->ibpret = &ibp;			/* set to update ibp if block moves */
    ibp->wait = sr_make_sem (0);	/* sem will tell us when rcv done */

    r = (Remd) ibp->next;		/* original rem passed via "next" */
    r->reply = ph;			/* pass ibp addr back to receiver */
    V (r->wait);			/* start it up */

    P ((char *) NULL, ibp->wait);	/* wait for reply */
    ph = (Pach) ibp;			/* inv block may have moved */
    sr_net_send (ph->origin, ACK_INVOKE, ph, ph->size);
					/* send reply */
    sr_kill_sem (ibp->wait);
    sr_free ((Ptr) ibp);
    sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to invoke an operation from a remote machine.
 *  Executes as separate process.
 */
void
sr_rmt_invk (ph)
Pach ph;
{
    Invb ibp = (Invb) ph;
    CUR_PROC->res = CUR_RES = ((Oper) ibp->opc.oper_entry) -> res;
    ibp = (Invb) sr_invoke ((char *) 0, ibp);	/* ibp can move if forwarded */
    ph = (Pach) ibp;
    if (ibp->type != SEND_IN && ibp->type != COSEND_IN) {
	sr_net_send (ph->origin, ACK_INVOKE, ph, ph->size);
	sr_free ((Ptr) ph);
    }
    CUR_PROC->res = CUR_RES = NULL;
    sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Establish contact with machine n, ensuring that only one connection is
 *  made.  A higher numbered machine never calls sr_connect to a lower machine,
 *  but instead asks the lower machine to connect to it, by passing a message
 *  through srx.
 *
 *  This routine is never called with a mutex held.  It is only called
 *  from sr_remote and sr_rmt_callme in this file, and none of the
 *  routines that call either of these [sr_invoke, sr_forward,
 *  sr_net_interface, sr_create_res, sr_destroy, sr_locate, sr_crevm,
 *  and sr_destvm] hold a lock while calling them.  Thus, we can make
 *  remote_mutex at a lower mutex class (acquired first) than the
 *  queue mutex.
 */
static void
contact (n)
int n;
{
    static Bool started[MAX_VM];	/* connection requested for each vm? */
    static Procq waiting[MAX_VM];	/* processes waiting for each vm */
    struct num_st npk;			/* actual message packet */
    Pach ph;				/* packet header pointer */

    if (n == sr_my_vm)
	sr_malf ("trying to connect to self");

    /* connecting to a lower machine is simple; ask it to do the work */
    /* (no problem if this happens more than once) */
    if (n < (int) sr_my_vm)  {		/* cast prevents Sun acc complaints */
	npk.num = n;
	npk.ph.priority = 0;
	ph = sr_remote (SRX_VM, REQ_CALLME, (Pach) &npk, sizeof (npk));
	return;
    }

    /* get interlock and check again that we're not connected */
    LOCK (remote_mutex, "contact");
    if (sr_net_known (n)) {
	UNLOCK (remote_mutex, "contact");
	return;
    }

    /* if we've already requested a connection, just wait until it's complete */
    if (started[n]) {

	LOCK_QUEUE ("contact");		/* protect block-scheduler */
	block (&waiting[n]);		/* put self on wait list */
	UNLOCK (remote_mutex, "contact");
	sr_scheduler ();		/* block */
	return;
    }

    /* must be the first time. set flag, then release interlock */
    started[n] = TRUE;
    /*  release lock for now -- we may block in sr_remote.  This would not
     *  seem to hurt anything, but it is not a good idea to let threads
     *  block while they hold RTS mutexes.  */
    UNLOCK (remote_mutex, "contact");

    /* establish the connection */
    npk.num = n;
    ph = sr_remote (SRX_VM, REQ_FINDVM, (Pach) &npk, sizeof (npk));
    sr_net_connect (n, ((struct saddr_st *) ph) -> addr);

    /* release everybody else who may now be waiting */
    LOCK (remote_mutex, "contact");
    while (waiting[n].head != NULL)
	awaken (waiting[n]);		/* unblock everybody */
    UNLOCK (remote_mutex, "contact");
}
