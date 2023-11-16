/*  oper.c -- runtime support of operations  */

#include "rts.h"

static Pool oper_pool;		/* pool of operation descriptors */
static Pool class_pool;		/* pool of class descriptors */

static void init_oper (), re_init_oper (), free_oper (), purge ();
static void init_class (), re_init_class ();
static void kill_inop ();



/* make_op (t, f, v, opcp) is used directly for making a local op
 * and indirectly for making a resource op.
 *
 * note that f is a field name so this can't be trivially made a function.
 *
 * This must be "called" with CUR_RES->rmutex already possessed.
 */
#define make_op(t, f, v, opcp) { \
    Oper op; \
    \
    (opcp)->vm = sr_my_vm; \
    op = (Oper) sr_addpool (oper_pool); \
    (opcp)->oper_entry = (Ptr) op; \
    (opcp)->seqn = op->seqn; \
    \
    op->type = (t); \
    op->f = (v); \
    op->pending = 0; \
    \
    op->res  = CUR_RES; \
    op->next = CUR_RES->oper_list; \
    CUR_RES->oper_list = op; \
}



/*
 *  Add new resource proc operation.  Called during resource initialization.
 */
void
sr_make_proc (opcp, type, ept)
Ocap *opcp;
enum op_type type;
Func ept;
{
    sr_check_stk (CUR_STACK);
    LOCK (CUR_RES->rmutex, "sr_make_proc");
    make_op (type, u.code, ept, opcp);
    UNLOCK (CUR_RES->rmutex, "sr_make_proc");
}



/*
 *  sr_new_op (locn, clap) -- make a single op for new(op(...)).
 */
/*ARGSUSED*/
Ocap
sr_new_op (locn, clap)
char *locn;
Class clap;
{
    Ocap c;

    LOCK (CUR_RES->rmutex, "sr_new_op");
    LOCK (clap->clmutex, "sr_new_op");
    make_op (DYNAMIC_OP, u.clap, clap, &c);
    UNLOCK (CUR_RES->rmutex, "sr_new_op");
    UNLOCK (clap->clmutex, "sr_new_op");
    return c;
}



/*
 *  sr_make_inops (addr, clap, ndim, type)
 *
 *  Make a set of new input operations.
 *  Called during initialization, and also later for local ops and new([]ops).
 *  Returns addr.
 */
Ptr
sr_make_inops (addr, clap, ndim, type)
Ptr addr;
Class clap;
int ndim, type;
{
    Array *a;
    Ocap *o;
    int n;

    sr_check_stk (CUR_STACK);
    LOCK (CUR_RES->rmutex, "sr_make_inops");
    LOCK (clap->clmutex, "sr_make_inops");

    if (ndim == 0) {
	make_op ((enum op_type) type, u.clap, clap, (Ocap *) addr);
	clap->numops += 1;
    } else {
	a = (Array *) addr;
	o = (Ocap *) ADATA (a);
	n = sr_acount (a);
	clap->numops += n;
	while (n--) {
	    make_op ((enum op_type) type, u.clap, clap, o);
	    o++;		/* don't use o++ in macro call above! */
	    }
    }

    UNLOCK (CUR_RES->rmutex, "sr_make_inops");
    UNLOCK (clap->clmutex, "sr_make_inops");
    return addr;
}



/*
 *  sr_make_arraysem (locn, addr, ndim)
 *
 *  Make an array of sem operations.
 *  Called only during initialization.
 *  (local ops and new([]ops) cannot be sems.)
 *  Returns addr.
 *  Code is similar to sr_make_inops.
 */
Ptr
sr_make_arraysem (locn, addr, ndim)
char *locn;
Ptr addr;
int ndim;
{
    Array *a;
    Sem *s;
    int n;

    sr_check_stk (CUR_STACK);
    LOCK (CUR_RES->rmutex, "sr_make_arraysem");

    if (ndim == 0)
	sr_loc_abort (locn, "cannot make 0 dimension array of sems");

    a = (Array *) addr;
    s = (Sem *) ADATA (a);
    n = sr_acount (a);
    while (n--) {
      *s++ = (Sem) sr_make_semop (locn);
    }

    UNLOCK (CUR_RES->rmutex, "sr_make_arraysem");
    return addr;
}



/*
 *  sr_dest_array (locn, addr)
 *
 *  Destroys an array of operations and frees its memory.
 */
void
sr_dest_array (locn, addr)
char *locn;
Ptr addr;
{
    Array *a = (Array *) addr;
    Ocap *o = (Ocap *) ADATA (a);
    int n = sr_acount (a);
    while (--n)
	sr_dest_op (locn, *o++);
}



/*
 *  sr_dest_op (locn, ocap)
 *
 *  Destroys a single input op which may be remote.
 */
void
sr_dest_op (locn, o)
char *locn;
Ocap o;
{
    struct ropc_st pkt;
    Pach ph;
    Oper op, rop;

    op = (Oper) o.oper_entry;
    if (op == NULL) {			/* if not a real capability */
	if (o.seqn == NOOP_SEQN)
	    return;			/* if noop, nothing to do */
	else if (o.seqn == NULL_SEQN)
	    sr_loc_abort (locn, "null operation capability");
	else
	    sr_loc_abort (locn, "invalid operation capability");
    }

    if (o.vm != sr_my_vm) {		/* if remote */
	if (o.vm <= 0 || o.vm > MAX_VM || !sr_exec_up)
	    sr_loc_abort (locn, "invalid operation capability");
	pkt.oc = o;
	ph = (Pach) &pkt;
	ph = sr_remote (o.vm, REQ_DESTOP, ph, sizeof (pkt));
	sr_free ((Ptr) ph);
	return;
    }

    if (o.seqn != op->seqn)
	sr_loc_abort (locn, "op no longer exists");
    if (op->type != DYNAMIC_OP)
	sr_loc_abort (locn, "not a dynamic op");

    /* Kill the op, but leave the class; another may be created later */
    LOCK (op->res->rmutex, "sr_dest_op");
    LOCK (op->u.clap->clmutex, "sr_dest_op");
    LOCK (op->omutex, "sr_dest_op");
    op->seqn++;				/* invalidate entry */

    if ((rop = op->res->oper_list) == op)
	op->res->oper_list = op->next;
    else {
	while (rop->next != op)  rop = rop->next;
	rop->next = op->next;
    }
    UNLOCK (op->res->rmutex, "sr_dest_op");
    UNLOCK (op->u.clap->clmutex, "sr_dest_op");
}



/*
 *  sr_kill_inops (addr, ndim)
 *
 *  Remove local operations from the operation table.  Purge any pending
 *  invocations from the queues.  If a killed operation was the last of
 *  its class, free the class as well.
 */
void
sr_kill_inops (addr, ndim)
Ptr addr;
int ndim;
{
    Array *a;
    Ocap *o;
    Oper op;
    int n;

    sr_check_stk (CUR_STACK);
    if (ndim == 0)
	o = (Ocap *) addr;
    else {
	a = (Array *) addr;
	o = (Ocap *) ADATA (a);
	}
    op = (Oper) o->oper_entry;

    LOCK (op->res->rmutex, "sr_kill_inops");
    LOCK (op->u.clap->clmutex, "sr_kill_inops");

    if (ndim == 0)
	kill_inop (addr);
    else {
	n = sr_acount (a);
	while (--n)
	    kill_inop ((Ptr) o++);
    }

    UNLOCK (op->res->rmutex, "sr_kill_inops");
    UNLOCK (op->u.clap->clmutex, "sr_kill_inops");
}

static void
kill_inop (addr)				/* kill one op */
Ptr addr;
{
    Ocap *o;
    Oper op, rop;
    Class clap;

    o = (Ocap *) addr;
    op = (Oper) o->oper_entry;
    clap = op->u.clap;

    LOCK (op->omutex, "sr_kill_inops");
    op->seqn++;

    if ((rop = op->res->oper_list) == op)
	op->res->oper_list = op->next;
    else {
	while (rop->next != op)  rop = rop->next;
	rop->next = op->next;
    }

    if (clap->old_in.head != NULL)
	purge ((Oper) o->oper_entry, &clap->old_in);
    if (clap->new_in.head != NULL)
	purge ((Oper) o->oper_entry, &clap->new_in);

    if (--clap->numops == 0)
	sr_delpool (class_pool, (Ptr) clap);
    op->next = NULL;
    free_oper (op);
    UNLOCK (op->omutex, "sr_kill_inops");

}



/*
 *  Kill all resource operations for the named resource.
 *  Purge any pending input invocations.
 *  This must be called only when we possess res->mutex.
 */
void
sr_kill_resops (res)
Rinst res;
{
    Oper op;
    Class clap;
    Class clapp;

    /* deallocate classes that were never used */

    clap = res->class_list;
    
    while (clap != NULL) {
	clapp = clap->next;
	if (clap->numops == 0) {
	    sr_delpool (class_pool, (Ptr) clap);
	}
	clap = clapp;
    }
    
    for (op = res->oper_list; op; op = op->next) {
	if (op->type == INPUT_OP || op->type == DYNAMIC_OP) {
	    clap = op->u.clap;

	    LOCK (clap->clmutex, "sr_kill_resops");
	    LOCK (op->omutex, "sr_kill_resops");

	    op->seqn++;
	    if (clap->old_in.head != NULL)
		purge (op, &clap->old_in);
	    if (clap->new_in.head != NULL)
		purge (op, &clap->new_in);
	    if (--clap->numops == 0)
		sr_delpool (class_pool, (Ptr) clap);

	    UNLOCK (clap->clmutex, "sr_kill_resops");
	    UNLOCK (op->omutex, "sr_kill_resops");

	} else if (op->type == SEMA_OP) {

	    LOCK (op->omutex, "sr_kill_resops");
	    op->seqn++;
	    sr_kill_sem (op->u.sema);
	    UNLOCK (op->omutex, "sr_kill_resops");

	} else { /* if (op->type == PROC_OP || op->type == PROC_SEP_OP) */

	    LOCK (op->omutex, "sr_kill_resops");
	    op->seqn++;
	    UNLOCK (op->omutex, "sr_kill_resops");
	}
    }

    if (res->oper_list != NULL)
	free_oper (res->oper_list);
}



/*
 *  Remove all invocations of the specified operation from an invocation list.
 *  Operations are represented only by index numbers since the machine and
 *  sequence numbers have been checked when the invocation was done.
 */
static void
purge (op, ilist)
Oper op;
Invq *ilist;
{
    Invb ibp, ribp, last;

    last = NULL;
    ibp = (*ilist).head;
    while (ibp) {
	ribp = ibp;
	ibp = ibp->next;
	if ((Oper) ribp->opc.oper_entry == op) {
	    if (last == NULL)
		(*ilist).head = ibp;
	    else
		last->next = ibp;
	    if (ibp != NULL)
		ibp->last = ribp->last;
	    else
		(*ilist).tail = last;
	    sr_rej_inv (ribp);
	}
	else
	    last = ribp;
    }
}



/*
 *  Returns pointer to the next eligible invocation block for the GC to check in
 *  processing an input statement.  Process must have access to the operation
 *  class.  If no invocations are available, wait until more arrive.
 *  We do not protect CUR_PROC since we're the only one who can have
 *  a handle on it (at least to manipulate these fields).
 */
Ptr
sr_get_anyinv (locn, clap)
char *locn;
Class clap;
{
    Invb ibp;

    CUR_PROC->locn = locn;		/* add locn to CUR_PROC structure */
    TRACE ("IN", locn, 0);

    sr_check_stk (CUR_STACK);
    if (CUR_PROC->next_inv == NULL)  {
	if (CUR_PROC->else_leg)
	    return NULL;
	else
	    sr_reaccess (clap);
    }
    ibp = CUR_PROC->next_inv;
    CUR_PROC->next_inv = ibp->next;
    return (Ptr) ibp;
}



/*
 *  Returns pointer to next eligible invocation of the specified operation.
 *  If none are available, return NULL.
 */
Ptr
sr_chk_myinv (opc)
Ocap opc;
{
    Oper op;
    Invb ibp;

    sr_check_stk (CUR_STACK);
    op = (Oper) opc.oper_entry;
    if (op == NULL)		/* if noop capability, return NULL */
	return NULL;
    while (ibp = CUR_PROC->next_inv) {
	CUR_PROC->next_inv = ibp->next;
	if (op == (Oper) ibp->opc.oper_entry)
	    break;
    }
    return (Ptr) ibp;
}



/*
 *  Receive an invocation.  May be remote.
 */
Ptr
sr_receive (locn, opc, else_present)
char *locn;
Ocap opc;
Bool else_present;
{
    struct ropc_st pkt;
    Pach ph;
    Oper op;
    Class clap;
    Invb ibp;
    Sem sp;

    CUR_PROC->locn = locn;
    sr_check_stk (CUR_STACK);
    TRACE ("IN", locn, 0);

    op = (Oper) opc.oper_entry;
    if (op == NULL) {		/* if not a real capability */
	if (opc.seqn == NOOP_SEQN)	/* if noop */
	    if (else_present)
		return NULL;		/* exec "else" arm */
	    else {
		sp = sr_make_sem (0);
		P (locn, sp);		/* else hang forever */
	    }
	else if (opc.seqn == NULL_SEQN)
	    sr_loc_abort (locn, "null operation capability");
	else
	    sr_loc_abort (locn, "invalid operation capability");
    }

    if (opc.vm != sr_my_vm) {		/* if remote */
	if (opc.vm <= 0 || opc.vm > MAX_VM || !sr_exec_up)
	    sr_loc_abort (locn, "invalid operation capability");
	pkt.oc = opc;
	pkt.elseflag = else_present;
	ph = (Pach) &pkt;
	ph = sr_remote (opc.vm, REQ_RECEIVE, ph, sizeof (pkt));
	if (((Pach) ph)->size > sizeof (pkt)){	/* if bigger, got invocation */
	    return (Ptr) ph;
	} else {
	    sr_free ((Ptr) ph);			/* no invocation */
	    return NULL;			/* execute "else" arm */
	}
    }

    /*  The op is local.  Be sure it's still valid  */
    op = (Oper) opc.oper_entry;
    clap = op->u.clap;
    if (opc.seqn != op->seqn)
	sr_loc_abort (locn, "op no longer exists");
    if (op->type != INPUT_OP && op->type != DYNAMIC_OP)
	sr_loc_abort (locn, "cannot input from a proc operation");

    /*  Get and return the next invocation of the desired op.  */
    sr_iaccess (clap, else_present);
    for (;;) {
	while (ibp = CUR_PROC->next_inv) {
	    CUR_PROC->next_inv = ibp->next;
	    if (opc.oper_entry == ibp->opc.oper_entry) {
		sr_rm_iop (locn, (Ptr) clap, ibp);
		return (Ptr) ibp;
		}
	}
	if (else_present) {
	    sr_rm_iop (locn, (Ptr) clap, (Invb) 0);
	    return NULL;
	} else
	    sr_reaccess (clap);
    }
    /*NOTREACHED*/
}



/*
 *  Create an operation to act as a semaphore (i.e., a non-exported,
 *  parameterless, operation in its own class.  This is an optimization.
 */
Ptr
sr_make_semop (locn)
char *locn;
{
    Oper op;
    Sem sp;

    sr_check_stk (CUR_STACK);

    op = (Oper) sr_addpool (oper_pool);
    LOCK (op->omutex, "sr_make_semop");
    op->type = SEMA_OP;
    op->u.sema = sp = sr_make_sem (0);

    TRACE ("CREATES", locn, sp); 

    op->next = CUR_RES->oper_list;
    UNLOCK (op->omutex, "sr_make_semop");
    CUR_RES->oper_list = op;  /* no protection on CUR_RES needed */
    return (Ptr) sp;
}



/*
 *  Initialize RTS operation table.
 */
void
sr_init_oper ()
{
    oper_pool = sr_makepool ("operations", sizeof (struct oper_st),
			    sr_max_operations, init_oper, re_init_oper);

    sr_no_ocap.seqn = NOOP_SEQN;		/* other fields zero */
    sr_nu_ocap.seqn = NULL_SEQN;		/* other fields zero */
}


static void
init_oper (op)
Oper op;
{
    static unsigned short sn = INIT_SEQ_OP;

    /* called under oper_pool lock */
    op->seqn = sn++;
    op->type = END_OP;
    INIT_LOCK (op->omutex, "op->omutex");
}


/*ARGSUSED*/ /*(under MultiSR they are)*/
static void
re_init_oper (op)
Oper op;
{
    RESET_LOCK (op->omutex);
}



/*
 *  Return a non-empty list of operation table entries to the free list.
 *  The list that this is attached to must be protected a mutex
 *  that the caller holds.
 */
static void
free_oper (op)
Oper op;
{
    Oper opp;

    for (opp = op; opp->next != NULL; opp = opp->next)
	sr_delpool (oper_pool, (Ptr) opp);
}



/*
 *  Initialize the operation class table.
 */
void
sr_init_class ()
{
    class_pool = sr_makepool ("operation classes", sizeof (struct class_st),
				sr_max_classes, init_class, re_init_class);
}



static void
init_class (c)
struct class_st *c;
{
    c->numops = 0;
    INIT_LOCK (c->clmutex, "class->clmutex");
}


/*ARGSUSED*/ /*(under MultiSR they are)*/
static void
re_init_class (c)
struct class_st *c;
{
    RESET_LOCK (c->clmutex);
}



/*
 *  Give the GC a new operation class.
 */
Ptr
sr_make_class ()
{
    Class clap;

    sr_check_stk (CUR_STACK);

    clap = (Class) sr_addpool (class_pool);
    clap->inuse  = FALSE;
    clap->old_in.head  = clap->new_in.head = NULL;
    clap->old_in.tail  = clap->new_in.tail = NULL;
    clap->old_pr.head  = clap->new_pr.head = NULL;
    clap->old_pr.tail  = clap->new_pr.tail = NULL;
    clap->else_pr = NULL;
    clap->else_tailpr = NULL;

    clap->next = CUR_RES->class_list;
    CUR_RES->class_list = clap;
    
    return (Ptr) clap;
}



/*
 *  Return number of pending invocations for an input operation.
 *
 *  The count may change, but we assume that it changes atomically,
 *  so we don't lock it.
 */
int
sr_query_iop (locn, o)
char *locn;
Ocap o;
{
    int count;
    Oper op;
    Pach ph;
    union {
	struct ropc_st r;
	struct num_st n;
    } pkt;

    sr_check_stk (CUR_STACK);

    op = (Oper) o.oper_entry;
    if (op == NULL) {			/* if not a real capability */
	if (o.seqn == NOOP_SEQN)
	    return 0;			/* count 0 for noop */
	else if (o.seqn == NULL_SEQN)
	    sr_loc_abort (locn, "null operation capability");
	else
	    sr_loc_abort (locn, "invalid operation capability");
    }

    if (o.vm == sr_my_vm) {		/* if local */
	if (op->type == INPUT_OP || op->type == DYNAMIC_OP)
	    return op->pending;
	else
	    return 0;			/* procs are immediate, so 0 pending */
	}

    /* must be remote. */
    if (o.vm <= 0 || o.vm > MAX_VM || !sr_exec_up)
	sr_loc_abort (locn, "invalid operation capability");
    pkt.r.oc = o;
    ph = (Pach) &pkt;
    ph = sr_remote (o.vm, REQ_COUNT, ph, sizeof (pkt));
    count = ((struct num_st *) ph)->num;
    sr_free ((Ptr) ph);
    return count;
}
