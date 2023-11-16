/*  remote.c -- remote request processing  */

#include "rts.h"

static void contact ();

static Mutex remote_mutex;	  /* protect started, waiting[] in contact()*/



/*
 *  Initialize free list of pending remote request message descriptors.
 */
void
mpd_init_rem ()
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
mpd_remote (dest, type, ph, size)
Vcap dest;
enum ms_type type;
Pach ph;
int size;
{
    struct remd_st rem;
    Invb ibp;

    if (! (mpd_net_known ((int) dest)))
	contact ((int) dest);		/* establish contact if none yet made */

    ph->priority = CUR_PROC->priority;

    ibp = (Invb) ph;
    if (type == REQ_INVOKE && (ibp->replied || ibp->type == REM_COCALL_IN)) {

	/* sends and forwards are not ack'd, and we don't wait on a cocall */
	mpd_net_send (dest, type, ph, size);
	return NULL;

    } else {

	/* for all other messages, await acknowledgement */

	ph->rem = &rem;
	rem.wait = mpd_make_sem (0);
	
	mpd_net_send (dest, type, ph, size);
	P ((char *) NULL, rem.wait);		/* wait for reply */
	mpd_kill_sem (rem.wait);

	return rem.reply;
    }
}



/*
 *  Service a request to establish a connection.
 *  Executes as a separate process.
 */
void
mpd_rmt_callme (ph)
Pach ph;
{
    int n = ((struct num_st *) ph) ->num;
    contact (n);
    mpd_net_send (n, ACK_CALLME, ph, PACH_SZ);
    mpd_free ((Ptr) ph);
    mpd_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to create a resource from a remote machine.
 *  Executes as separate process.
 */
void
mpd_rmt_create (ph)
Pach ph;
{
    int size;
    struct crb_st *crbp;
    struct rres_st *reply;

    crbp = (struct crb_st *) ph;
    size = sizeof (struct rres_st) - sizeof (Rcap) + crbp->rc_size;
    reply = (struct rres_st *) mpd_alc (-size, 1);

    crbp->rcp = & reply->rc;
    mpd_create_res (crbp);

    reply->ph.rem    = ph->rem;
    mpd_net_send (ph->origin, ACK_CREATE, &reply->ph, size);

    mpd_free ((Ptr) reply);
    /* ph freed when resource destroyed */

    mpd_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to query a pending op count from a remote machine.
 *  Executes as a separate process.
 */
void
mpd_rmt_query (ph)
Pach ph;
{
    ((struct num_st *) ph)->num = 
	mpd_query_iop ((char *) NULL, ((struct ropc_st *) ph)->oc);
    mpd_net_send (ph->origin, ACK_COUNT, ph, ph->size);
    mpd_free ((Ptr) ph);
    mpd_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to destroy an op from a remote machine.
 *  Executes as separate process.
 */
void
mpd_rmt_destop (ph)
Pach ph;
{
    mpd_dest_op ((char *) NULL, ((struct ropc_st *) ph)->oc);
    mpd_net_send (ph->origin, ACK_DESTOP, ph, ph->size);
    mpd_free ((Ptr) ph);
    mpd_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to destroy a resource from a remote machine.
 *  Executes as separate process.
 */
void
mpd_rmt_destroy (ph)
Pach ph;
{
    mpd_destroy ((char *) NULL, ((struct rres_st *) ph)->rc);
    mpd_net_send (ph->origin, ACK_DESTROY, ph, ph->size);
    mpd_free ((Ptr) ph);
    mpd_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request from mpdx to destroy this virtual machine.
 *  Executes as a separate process.
 */
void
mpd_rmt_destvm (ph)
Pach ph;
{
    mpd_dest_all ();		/* destroy all non-global resources */
    mpd_destroy_globals ();	/* destroy all global resources */

    mpd_net_send (ph->origin, ACK_DESTVM, ph, ph->size);  /* ack destroy */
    mpd_net_send (MPDX_VM, MSG_IDLE,			/* send msg counts */
	&mpd_msg_counts.ph, sizeof (mpd_msg_counts));
    EXIT (0);						/* kill self */
}



/*
 *  Service a request to receive an operation from a remote machine.
 *  Executes as separate process.
 */
void
mpd_rmt_rcv (ph)
Pach ph;
{
    Sem sp;
    Invb ibp, *ret;
    struct ropc_st *p;
    struct remd_st rem;

    p = (struct ropc_st *) ph;
    ibp = (Invb) mpd_receive ((char *) 0, p->oc, p->elseflag);

    if (ibp == NULL) {
	/*
	 * No invocation available. Return original packet.
	 */
	mpd_net_send (ph->origin, ACK_RECEIVE, ph, ph->size);
	mpd_free ((Ptr) ph);
	mpd_kill (CUR_PROC, (Rinst) NULL);
    }

    /*
     * Got an invocation.  If it needs no reply, send it and disappear.
     */
    if (ibp->discard) {
	ibp->ph.rem = ph->rem;
	mpd_net_send (ph->origin, ACK_RECEIVE, (Pach) ibp, ibp->ph.size);
	mpd_free ((Ptr) ibp);
	mpd_free ((Ptr) ph);
	mpd_kill (CUR_PROC, (Rinst) NULL);
    }

    /*
     * The invocation needs a reply.  Send MSG_RCVCALL and wait.
     */
    rem.wait = mpd_make_sem (0);		/* set up reply block */
    ibp->ph.rem = &rem;			/* put addr in invocation block */
    ibp->next = (Invb) ph->rem;		/* pass origl reply addr via "next" */
    sp = ibp->wait;			/* invoker's semaphore */
    ret = ibp->ibpret;			/* invoker's ibp pointer */

    mpd_net_send (ph->origin, MSG_RCVCALL, (Pach) ibp, ibp->ph.size);
    mpd_free ((Ptr) ph);
    mpd_free ((Ptr) ibp);

    P ((char *) NULL, rem.wait);	/* wait for reply */
    mpd_kill_sem (rem.wait);

    ph = rem.reply;			/* get reply address */
    ibp = (Invb) ph;

    if (ret)
	*ret = ibp;			/* tell invoker new ibp address */
    V (sp);				/* awaken invoker */

    mpd_kill (CUR_PROC, (Rinst) NULL);	/* kill self */
}



/*
 *  Service a MSG_RCVCALL received in response to a REQ_RECEIVE that found
 *  a call needing a reply.  Reply to the RECEIVE requestor and pass it the
 *  invocation block; wait for the call to reply; then pass back the reply
 *  to the original VM.  Executes as a separate process.
 */
void
mpd_rcv_call (ph)
Pach ph;
{
    Invb ibp;
    Remd r;

    ibp = (Invb) ph;
    ibp->ibpret = &ibp;			/* set to update ibp if block moves */
    ibp->wait = mpd_make_sem (0);	/* sem will tell us when rcv done */

    r = (Remd) ibp->next;		/* original rem passed via "next" */
    r->reply = ph;			/* pass ibp addr back to receiver */
    V (r->wait);			/* start it up */

    P ((char *) NULL, ibp->wait);	/* wait for reply */
    ph = (Pach) ibp;			/* inv block may have moved */
    mpd_net_send (ph->origin, ACK_INVOKE, ph, ph->size);
					/* send reply */
    mpd_kill_sem (ibp->wait);
    mpd_free ((Ptr) ibp);
    mpd_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Service a request to invoke an operation from a remote machine.
 *  Executes as separate process.
 */
void
mpd_rmt_invk (ph)
Pach ph;
{
    Invb ibp = (Invb) ph;
    CUR_PROC->res = CUR_RES = ((Oper) ibp->opc.oper_entry) -> res;
    ibp = (Invb) mpd_invoke ((char *) 0, ibp);	/* ibp can move if forwarded */
    ph = (Pach) ibp;
    if (ibp->type != SEND_IN && ibp->type != COSEND_IN) {
	mpd_net_send (ph->origin, ACK_INVOKE, ph, ph->size);
	mpd_free ((Ptr) ph);
    }
    CUR_PROC->res = CUR_RES = NULL;
    mpd_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Establish contact with machine n, ensuring that only one connection is
 *  made.  A higher numbered machine never calls mpd_connect to a lower machine,
 *  but instead asks the lower machine to connect to it, by passing a message
 *  through mpdx.
 *
 *  This routine is never called with a mutex held.  It is only called
 *  from mpd_remote and mpd_rmt_callme in this file, and none of the
 *  routines that call either of these [mpd_invoke, mpd_forward,
 *  mpd_net_interface, mpd_create_res, mpd_destroy, mpd_locate, mpd_crevm,
 *  and mpd_destvm] hold a lock while calling them.  Thus, we can make
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

    if (n == mpd_my_vm)
	mpd_malf ("trying to connect to self");

    /* connecting to a lower machine is simple; ask it to do the work */
    /* (no problem if this happens more than once) */
    if (n < (int) mpd_my_vm)  {		/* cast prevents Sun acc complaints */
	npk.num = n;
	npk.ph.priority = 0;
	ph = mpd_remote (MPDX_VM, REQ_CALLME, (Pach) &npk, sizeof (npk));
	return;
    }

    /* get interlock and check again that we're not connected */
    LOCK (remote_mutex, "contact");
    if (mpd_net_known (n)) {
	UNLOCK (remote_mutex, "contact");
	return;
    }

    /* if we've already requested a connection, just wait until it's complete */
    if (started[n]) {

	LOCK_QUEUE ("contact");		/* protect block-scheduler */
	block (&waiting[n]);		/* put self on wait list */
	UNLOCK (remote_mutex, "contact");
	mpd_scheduler ();		/* block */
	return;
    }

    /* must be the first time. set flag, then release interlock */
    started[n] = TRUE;
    /*  release lock for now -- we may block in mpd_remote.  This would not
     *  seem to hurt anything, but it is not a good idea to let threads
     *  block while they hold RTS mutexes.  */
    UNLOCK (remote_mutex, "contact");

    /* establish the connection */
    npk.num = n;
    ph = mpd_remote (MPDX_VM, REQ_FINDVM, (Pach) &npk, sizeof (npk));
    mpd_net_connect (n, ((struct saddr_st *) ph) -> addr);

    /* release everybody else who may now be waiting */
    LOCK (remote_mutex, "contact");
    while (waiting[n].head != NULL)
	awaken (waiting[n]);		/* unblock everybody */
    UNLOCK (remote_mutex, "contact");
}
