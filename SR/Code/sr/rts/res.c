/*  res.c -- resource management  */

#include "rts.h"

static void init_res (), re_init_res ();
static void destroy_one_res ();		/* destroy a resource */
static void destroy_one_global ();	/* destroy a global */

static Pool res_pool;		/* pool of resource descriptors */
static Pool succ_pool;		/* pool of succ_st nodes */

static Bool suiciding = FALSE;	/* nonzero if destroy (myvm ()) in progress */
static Rinst destroy_list;	/* linked list of resources to destroy */



/* struct succ_st keeps a linked list of globals and global imports */
struct succ_st {	
    short rpatid;		/* index into sr_rpatt and global_status */
    struct succ_st *next;	
};

struct global_st {
    short rpatid;		/* my index */
    Bool created;		/* has this global been create yet? */
    Rinst res;			/* resource descriptor for the global */
    Sem glmutex;		/* protection for this structure */
    /* the following fields are used to help linearize the global import dag,
     *  as in Knuth vol1 (2ed) page 262. */
    int count;			/* number of globals that import me */
    struct succ_st *top;	/* list of globals I import directly */	
    struct global_st *qlink;	/* list of globals imported by no others */
};

static struct global_st *global_status = NULL;
static void add_child ();



void
sr_create_global (locn, rpatid)
char *locn;
short rpatid;
{
    Proc pr;
    Rinst res;
    Rinst creator_res = CUR_RES;
    Sem wait;
    int creator_idx = CUR_RES->rpatid;	/* NULL res can't create global*/

    CUR_PROC->locn = locn;		/* add locn to CUR_PROC structure */

    if (suiciding)
	sr_abort ("attempting to create while VM is being destroyed");

    /*
     * Interlock the record for this global.
     */

    P ((char *) 0, global_status[rpatid].glmutex);

    /*
     *  If the global is already running, there's nothing more to do.
     *  We know that the global is fully initialized.
     */

    
    TRACE ("CREATEG", locn, 0); 

    if (global_status[rpatid].created)  {
	DEBUG (D_RESOURCE, "global %s duplicate create ignored",
	    sr_rpatt[rpatid].name, 0, 0);
	V (global_status[rpatid].glmutex);
	return;
    }

    /*
     * Record the import of the global rpatid by creator_res.
     */
    if (creator_res->is_global) {
	global_status[rpatid].count++;	 /* one more imports global rpatid */
	add_child (creator_idx, rpatid); /* add child id to creator's list */
    }

    /*
     * Allocate a resource descriptor.
     */
    res = (Rinst) sr_addpool (res_pool);
    res->rpatid = rpatid;
    res->is_global = TRUE;
    global_status[rpatid].res = res;

    /*
     * Spawn the body process.
     */
    /* protect the spawn-activate region */
    LOCK (res->rmutex, "sr-create_global");
    LOCK_QUEUE ("sr_create_global");
    pr = sr_spawn (sr_rpatt[rpatid].initial, CUR_PROC->priority,
	res, TRUE, 0L, 0L, 0L, 0L);
    pr->pname = "[global init]";	/* GC will reset */
    DEBUG5 (D_RESOURCE, "global %s init %lX spawned   as p%06lX, r%06lX",
	sr_rpatt[rpatid].name, sr_rpatt[rpatid].initial, pr, res, 0);

    pr->wait = wait = sr_make_sem (0);
    pr->ptype = INITIAL_PR;

    /*
     *  Start body process and wait for it to complete or reply.
     */
    DEBUG (D_RESOURCE, "starting global %s", sr_rpatt[rpatid].name, 0, 0);
    sr_activate (pr);
    UNLOCK_QUEUE ("sr_create_global");
    UNLOCK (res->rmutex, "sr-create_global");

    P ((char *) 0, wait);

    sr_kill_sem (wait);

    /*
     *  Now, finally, the importer of the global can proceed.
     */

    /*  We must set this to TRUE only after the global is totally
     *  initialized.  This is because, to avoid possible deadlock,
     *  we test if this global is already created before we try to
     *  grab this lock, and if it is already created then we just
     *  return.  If we try to grab the lock we can hang with 1 JS
     *  and MultiSR locks; the first caller has the lock and blocks
     *  until its initial code is done, the initial code blocs for
     *  I/O, and then another importer tries to grab the lock.  Since
     *  we only have one JS, we're hung (actually, with n JSs and
     *  n+1 simultaneous global imports you hang).
     */

    global_status[rpatid].created = TRUE;
    V (global_status[rpatid].glmutex);
}



/*
 *  Create new instance of a resource (called from generated code).
 *
 *  crbp has the resource parameters filled in.  We fill in the rest
 *  of the crb fields with the parameters.  The GC allocates the
 *  crbp block and the RTS frees it, while the RTS allocates an
 *  Rcap and the GC frees it (we return a pointer to this capability).
 */
Ptr
sr_create_resource (locn, rpatid, vm, crbp, rcapsize)
char *locn;
int rpatid;
Vcap vm;
CRB *crbp;
int rcapsize;
{
    if (locn)
	CUR_PROC->locn = locn;		/* remember caller's locn */
    sr_check_stk (CUR_STACK);
    TRACE ("CREATER", locn, 0);

    /* fill in the rest of the crbp from the parameters */
    crbp->rpatid = rpatid;
    crbp->vm = vm;
    crbp->ph.size = crbp->crb_size;
    crbp->rcp = (Rcap *) sr_alc (-rcapsize, 1);
    crbp->rc_size = rcapsize;
    sr_create_res (crbp);
    return (Ptr) crbp->rcp;
}



/*
 *  Create new instance of a resource (called internally).
 *
 *  crbp is assumed to point to an alloc'd block; the block header is altered.
 */
void
sr_create_res (crbp)
CRB *crbp;
{
    Rinst res;
    Proc pr;
    Sem wait;
    Memh mp;

    sr_check_stk (CUR_STACK);

    if (suiciding)
	sr_abort ("attempting to create while VM is being destroyed");

    if (crbp->vm == NULL_VM)
	sr_abort ("attempting to create resource on null machine");

    if (crbp->vm != sr_my_vm && crbp->vm != NOOP_VM) {
	Pach ph;
	Ptr src, dest;

	/* Send request to remote machine.  */
	crbp->ph.priority = CUR_PROC->priority;
	ph = sr_remote (crbp->vm, REQ_CREATE, &crbp->ph, crbp->ph.size);

	/* Copy back new resource capability.  */
	if ((dest = (Ptr) crbp->rcp) != NULL) {
	    src = (Ptr) & ((struct rres_st *) ph)->rc;
	    while (crbp->rc_size--)
		*dest++ = *src++;
	}

	sr_free ((Ptr) ph);
	return;
    }

    /*
     *	Allocate a new resource instance descriptor.
     */
    res = (Rinst) sr_addpool (res_pool);
    res->rpatid = crbp->rpatid;
    res->crb_addr = crbp;
    res->rc_size = crbp->rc_size;
    res->rcp = crbp->rcp;

    /*
     *	Make newly created resource owner of CRB.
     *  We're assuming here that crbp is the address of an alloc'd block.
     */
    mp = (Memh) ((Ptr) crbp - MEMH_SZ);
    mp->res = res;
    insert (mp, res->meml, rnext, rlast);

    /*
     *	Invoke UC resource entry code as a separate process.
     */
    /* protect the spawn-activate region */
    LOCK (res->rmutex, "sr_create_res");
    LOCK_QUEUE ("sr_create_res");
    pr = sr_spawn (sr_rpatt[crbp->rpatid].initial, CUR_PROC->priority,
	res, TRUE, (long) crbp, 0L, 0L, 0L);
    pr->wait = wait = sr_make_sem (0);
    pr->ptype = INITIAL_PR;

    /*
     *  Start new process;  returns when initialization finishes.
     */
    sr_activate (pr);

    UNLOCK_QUEUE ("sr_create_res");
    UNLOCK (res->rmutex, "sr_create_res");

    P ((char *) 0, wait); 

    sr_kill_sem (wait);
}



/*
 *  Add a node for child_idx to creator_idx's top list, thus indicating
 *  the fact that global creator_idx imports global child_idx.  This is
 *  done when we have global_status[creator_idx].glmutex.
 */
static void
add_child (creator_idx, child_idx)
short creator_idx, child_idx;
{
    struct succ_st *succ = (struct succ_st *) sr_addpool (succ_pool);

    succ->rpatid = child_idx;
    /* add to front of list -- order is not important */
    succ->next = global_status[creator_idx].top;
    global_status[creator_idx].top = succ;
}



/*
 *  Allocate memory for resource variables.
 *  Called by the body code immediately after entry.
 *
 *  Initialize resource ID part of myresource capability.
 *
 *  Remember the main resource so we can destroy it at termination time.
 */
Ptr
sr_alloc_rv (size)
int size;
{
    Rcap *rcp;

    sr_check_stk (CUR_STACK);
    LOCK (CUR_RES->rmutex, "sr_alloc_rv");
    CUR_RES->rv_base = sr_alc (size, 0);
    rcp = (Rcap *) CUR_RES->rv_base;
    rcp->vm = sr_my_vm;
    rcp->res = CUR_RES;
    rcp->seqn = CUR_RES->seqn;
    UNLOCK (CUR_RES->rmutex, "sr_alloc_rv");

    LOCK (sr_main_res_mutex, "sr_alloc_rv");
    if (sr_main_res.res == 0)
	memcpy ((char *) &sr_main_res, (char *) rcp, sizeof (sr_main_res));
    UNLOCK (sr_main_res_mutex, "sr_alloc_rv");
    return CUR_RES->rv_base;
}



/*
 *  Called when resource's initial code has been executed.
 *  Copy myresource of new resource to capability.
 *  Remove initial process from list of processes for resource.
 *  Notify creator.
 */
void
sr_finished_init ()
{
    Ptr src, dest;

    sr_check_stk (CUR_STACK);
    LOCK (CUR_RES->rmutex, "sr_finished_init");

    if (! (CUR_RES->status & INIT_REPLY)) {

	if ((dest = (Ptr) CUR_RES->rcp) != NULL) {
	    src = CUR_RES->rv_base;
	    while (CUR_RES->rc_size--) *dest++ = *src++;
	}

	V (CUR_PROC->wait);
	CUR_PROC->wait = NULL;
    }
    sr_kill (CUR_PROC, CUR_RES);
    UNLOCK (CUR_RES->rmutex, "sr_finished_init");
}



/*
 *  Allocate and return a null or noop resource cap.
 *
 *  size give the length of the rescap.
 *  pattern is an int array specifying layout of opcaps within the rescap:
 *	zero or more occurrences of ->  ndim, [lb1, lb2, [...]]
 *	    ndim=0 is a simple ocap
 *	    ndim=n is an n-dimensional array; need headers etc.
 *	    ndim=-1 terminates the list
 *  ocap is the initial value for each opcap in the rescap.
 */

Ptr
sr_literal_rcap (size, pattern, o)
int size, *pattern;
Ocap *o;
{
    Ptr p, q;
    int ndim;

    p = q = sr_alc (size, 1);
    if (o == &sr_nu_ocap)
	memcpy (p, (Ptr) &sr_nu_rcap, sizeof (Rcap));
    else
	memcpy (p, (Ptr) &sr_no_rcap, sizeof (Rcap));
    p += SRALIGN (sizeof (Rcap));

    while ((ndim = *pattern++) >= 0) {
	if (ndim == 0) {
	    * (Ocap *) p = *o;
	    p += SRALIGN (sizeof (Ocap));
	} else if (ndim > 3) {
	    sr_abort ("can't handle 4-D (or more) ops in null/noop rescaps");
	} else {
	    sr_init_array ((char*) 0, (Array *) p, sizeof (Ocap), (Ptr) o, ndim,
		pattern[0], pattern[1], pattern[2], pattern[3],
		pattern[4], pattern[5]);
	    pattern += 2 * ndim;
	    p += DSIZE (p);
	}
    }
    return q;
}



/*
 *  Destroy an instance of a resource.
 */
void
sr_destroy (locn, rcp)
char *locn;
Rcap rcp;
{
    Rinst res;
    Proc pr;
    Bool kill_me = FALSE;
    Sem wait;
    Sem wake_creator = NULL;

    if (locn)
	CUR_PROC->locn = locn;		/* remember source location */
    sr_check_stk (CUR_STACK);
    TRACE ("DESTROYR", locn, 0);

    /*
     *	Check for null or noop resource capability.
     */
    if (rcp.res == NULL) {
	if (rcp.seqn == NOOP_SEQN)
	    return;
	sr_abort ("attempting to destroy null resource");
    }

    if (rcp.vm != sr_my_vm) {
	/* Send destroy request to the remote machine. */
	struct rres_st dr;
	Pach ph;

	dr.rc.vm = rcp.vm;
	dr.rc.seqn = rcp.seqn;
	dr.rc.res  = rcp.res;
	dr.ph.priority = CUR_PROC->priority;

	ph = sr_remote (rcp.vm, REQ_DESTROY, (Pach) &dr, sizeof (dr));

	/* this ph does not belong to any resource */
	sr_free ((Ptr) ph);
	return;
    }

    res = rcp.res;
    DEBUG (D_RESOURCE, "sr_destroy %s %s (r%06lX) ",
	res->is_global ? "  global" : "resource",
	sr_rpatt[res->rpatid].name, res);

    LOCK (res->rmutex, "sr_destroy");

    /*
     *	Check the sequence number.  Also invalidate it
     *	so subsequent destroys of same resource fail.
     */
    if (rcp.seqn != res->seqn) {	/* check to be sure its valid */
	char buf[100];
	sprintf (buf,
	    "attempting to destroy resource (%s) that no longer exists",
	    sr_rpatt[res->rpatid].name);
	UNLOCK (res->rmutex, "sr_destroy");
	sr_abort (buf);
    }
    res->seqn += 1;			/* invalidate it */

    /*
     *	Execute finalization code if there is any.
     */
    if (sr_rpatt [res->rpatid].final != NULL) {
	DEBUG (D_RESOURCE, "%s r%06lX (%s) exec final",
	    (res->is_global ? "global  " : "resource"), res,
	    sr_rpatt[res->rpatid].name);
	/* we need to hold queue mutex to protect the spawn-activate region */
	LOCK_QUEUE ("sr_destroy");
	pr = sr_spawn (sr_rpatt[res->rpatid].final, CUR_PROC->priority,
	    res, TRUE, (long) res->crb_addr, (long) res->rv_base, 0L, 0L);
	pr->pname = "[final]";
	pr->wait = wait = sr_make_sem (0);
	pr->ptype = FINAL_PR;
	sr_activate (pr);
	UNLOCK_QUEUE ("sr_destroy");
	UNLOCK (res->rmutex, "sr_destroy");
	P ((char *) 0, wait);
	sr_kill_sem (wait);
	LOCK (res->rmutex, "sr_destroy");
	DEBUG (D_RESOURCE, "%s r%06lX (%s) final done",
	    (res->is_global ? "global  " : "resource"), res,
	    sr_rpatt[res->rpatid].name);
    } else {
	UNLOCK_QUEUE ("sr_destroy");
    }

    sr_kill_resops (res);

    /*
     *	Kill all processes belonging to the resource,
     *	but defer killing myself and notifying creator
     *	if the resource has an initial process.
     *  Note that if sr_kill finds a READY proc it tries to take it
     *  off of the ready queue, and if it doesn't find it there then
     *  it aborts.  Thus, if a proc is going to belong to a resource
     *  in the RTS we need to protect the proc (its status, and
     *  where it is blocked) with the queue mutex like this:
     *
     *		if (res != NULL) grab res->rmutex
     *		grab queue mutex
     *		pr = sr_spawn (...res, TRUE, ...)
     *		fill in pr->fields
     *		sr_activate (pr)
     *		give queue mutex
     *		if (res != NULL) give res->rmutex
     *
     *  Note that the TRUE in the sr_spawn call means that we already
     *  hold res->rmutex.  Also, must grab rmutex before queue mutex, so...
     */

    while (pr = res->procs) {

	if (pr->ptype == INITIAL_PR && ! (res->status & INIT_REPLY)) {
	    RTS_WARN ("body process destroyed");
	    wake_creator = pr->wait;
	}

	if (pr == CUR_PROC) {
	    kill_me = TRUE;
	    /* pr is first on res list -- slide past it. */
	    pr = pr->procs;
	}

	if (pr == NULL)
	    break;	/* no more processes except possibly self */
	
	sr_kill (pr, res);
	/* sr_kill removes the thread off of the res->procs list. */
    }

    /*
     *	Free memory associated with resource and
     *	return resource instance descriptor to free list.
     */
    sr_res_free (res);

    res->status = FREE_SLOT;
    UNLOCK (res->rmutex, "sr_destroy");

    /* we add the res to the free list after releasing the rmutex lock. */
    sr_delpool (res_pool, (Ptr) res);

    if (wake_creator)
	V (wake_creator);

    /*
     *	Commit suicide if the destroyer is part of the
     *	dead resource.
     */
    if (kill_me)
	sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 *  Destroy all the resource instances on this machine, *not* including
 *  any globals.
 */
void
sr_dest_all ()
{
    Rinst r;
    Rcap rc;

    if (suiciding)
	return;				/* somebody's already doing this */
    suiciding = TRUE;

    sr_eachpool (res_pool, destroy_one_res);	/* create hitlist */

    while (destroy_list != NULL) {
	r = destroy_list;
	destroy_list = destroy_list->next;
	rc.vm = sr_my_vm;
	rc.res = r;
	rc.seqn = r->seqn;
	sr_destroy ((char *) 0, rc);
    }
}



/*
 *  Put a resource (but not a global) on the destroy_list.
 */
static void
destroy_one_res (r)
Rinst r;
{
    if (! (r->status & FREE_SLOT) && ! r->is_global) {
	r->next = destroy_list;
	destroy_list = r;
    }
}



/*
 *  A resource's final code has completed.  Notify destroyer.
 */
void
sr_finished_final ()
{
    sr_check_stk (CUR_STACK);
    LOCK (CUR_RES->rmutex, "sr_finished_final");
    if (! (CUR_RES->status & FINAL_REPLY)) {
	V (CUR_PROC->wait);
	CUR_PROC->wait = NULL;
    }
    sr_kill (CUR_PROC, CUR_RES);
}


/*
 *  Initialize resource management part of SR RTS.
 */
void
sr_init_res ()
{
    int i;

    sr_nu_rcap.vm = 0;			/* create null resource capability */
    sr_nu_rcap.seqn = NULL_SEQN;		/* indicate NULL */
    sr_nu_rcap.res  = 0;			/* rin[0] is never allocated */

    sr_no_rcap.vm = 0;			/* create noop resource capability */
    sr_no_rcap.seqn = NOOP_SEQN;		/* indicate NOOP */
    sr_no_rcap.res  = 0;			/* rin[0] is never allocated */

    /*
     *	Initialize free list of resource instance descriptors.
     */
    res_pool = sr_makepool ("active resources", sizeof (struct rin_st),
			sr_max_resources, init_res, re_init_res);

    INIT_LOCK (sr_main_res_mutex, "main_res_mutex");

    /* we really need a sr_num_rpats or something from srl... */
    global_status =
	(struct global_st *) sr_alloc (sr_num_rpats * sizeof(struct global_st));

    for (i = 0; i < sr_num_rpats; i++)  {
	global_status[i].rpatid = i;
	global_status[i].created = FALSE;
	global_status[i].glmutex = sr_make_sem (1);
	global_status[i].count = 0;
	global_status[i].top = NULL;
	global_status[i].qlink = NULL;
    }

    succ_pool = sr_makepool ("successor structs", sizeof (struct succ_st),
		(int) MAX_INT, (Func) 0, (Func) 0);
}


/*
 * initialize a resource descriptor for the first time
 */
static void
init_res (r)
Rinst r;
{
    static unsigned short sn = INIT_SEQ_RES;

    /* called under res_pool lock */
    r->seqn = sn++;
    r->status = FREE_SLOT;
    INIT_LOCK (r->rmutex, "r->rmutex");
}



/*
 * initialize a resource descriptor
 */
static void
re_init_res (r)
Rinst r;
{
    r->status = 0;
    r->is_global = FALSE;
    RESET_LOCK (r->rmutex);
    r->crb_addr = NULL;
    r->rc_size = 0;
    r->oper_list = NULL;
    r->class_list = NULL;
    r->procs = NULL;
    r->meml = NULL;
    r->rcp = NULL;
}



/*
 *  destroy the globals in this vm from the top of the hierarchy
 *  to the bottom.  See Knuth, vol1 (2ed) p. 258-262.  At runtime
 *  (global create time) we have constructed the information in
 *  figure 8 (in global_status).  We now make a queue of globals
 *  that have no predecessors (globals that import them), and
 *  then just keep going until this queue is empty, following
 *  steps T6 and T7 of figure 9.
 */
void
sr_destroy_globals ()
{
    int i;
    struct global_st *q = NULL;
    struct succ_st *succ;
    Rinst r;
    Rcap rc;

    /* construct the queue */
    for (i = 0; i < sr_num_rpats; i++)
	if (global_status[i].created && global_status[i].count == 0) {
	    global_status[i].qlink = q;
	    q = &global_status[i];
	}

    /* now process guys from the queue one at a time (and in so doing
     * creating more for the queue */
    for (; q != NULL; q = q->qlink) {

	r = global_status[q->rpatid].res;
	rc.vm = sr_my_vm;
	rc.res = r;
	rc.seqn = r->seqn;
	sr_destroy ((char *) 0, rc);

	/*
	 *  for each guy on q's top list, since we have destroyed
	 *  q->rpatid we decrement this guy's count of predecessors.
	 *  if that count is q, we add him to q.
	 */
	for (succ = q->top; succ != NULL; succ = succ->next) {
	    if (--global_status[succ->rpatid].count == 0) {
		global_status[succ->rpatid].qlink = q->qlink;
		q->qlink = &global_status[succ->rpatid];
	    }
	}	
    }
}
