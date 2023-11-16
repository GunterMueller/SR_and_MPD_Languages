/*  process.c -- process management and scheduling  */

#include "rts.h"
#include "../srmulti.c"

static void rem_proc (), vm_jobserver ();
static void idle_proc (), check_idle (), make_idle_proc ();
static void add_readyq (), dequeue ();
static void handle_should_die ();
static void init_proc (), re_init_proc ();

static Bool check_ready ();
static Proc dequeue_first ();
static Ptr newstack ();



static Pool proc_pool;

static Procq *sr_scheduler_queue_addr[MAX_JOBSERVERS];

static int idle_count = 1;	/* negative input count at last idle message */

static int spin_count = DEF_SPIN;	/* idle iterations before napping */
static int timeout = DEF_NAP;		/* nap time in msec after giving up */



/*
 *  Return a process descriptor to the free list.
 *  The "caller" argument must be a persistent string.
 */
#define free_proc(pr, caller)  { \
    LOCK_QUEUE ("free_proc"); \
    sr_delpool (proc_pool, (Ptr) pr); \
    UNLOCK_QUEUE ("free_proc"); \
}


/*
 *  Create a new process:
 *	Allocate a new process descriptor.
 *	Add process to list for specified resource.
 *	Allocate stack space.
 *	Set up stack for process initiation.
 *	Return process descriptor.
 *	(Do not put process on the ready list.)
 *
 *  If a proc is going to belong to a resource
 *  in the RTS we need to protect the proc (its status, and
 *  where it is blocked) with the queue mutex like this:
 *
 *          if (res != NULL) grab res->rmutex
 *          grab queue mutex
 *          pr = sr_spawn (...res, TRUE, ...)
 *          fill in pr->fields
 *          sr_activate (pr)
 *          give queue mutex
 *          if (res != NULL) give res->rmutex
 *
 *  Otherwise, sr_destroy could find it on the res list, and try
 *  to kill it, before it is on the queue.  This causes problems.
 *  Note that the TRUE in the sr_spawn call means that we already
 *  hold res->rmutex.  Also, must grab rmutex before queue mutex, so...
 *
 */

Proc
sr_spawn (pc, pri, res, have_res_mutex, arg1, arg2, arg3, arg4)
Func pc;
int pri;
Rinst res;
Bool have_res_mutex;
long arg1, arg2, arg3, arg4;
{
    Proc pr;

    if ((res != NULL) && (!have_res_mutex))
	LOCK (res->rmutex, "sr_spawn");

    pr = (Proc) sr_addpool (proc_pool);
    pr->next = NULL;
    pr->res = res;

    if (res != NULL) {
	/* we already hold rmutex */
	pr->procs = res->procs;
	res->procs = pr;
    }

    /*
     *  If we don't still hold rmutex before setting the status below,
     *  the sr_destroy could find it on its res->procs list and try
     *  to kill it (based on the *old* status).  sr_kill would not
     *  likely find it where it was supposed to be, and it would abort.
     */
    pr->priority = pri;
    pr->should_die = FALSE;
    pr->waiting_killer = NULL;
    pr->status = INFANT;	/* no need to lock status access */
    if (!pr->stack)
	pr->stack = newstack (sr_stack_size);

    sr_build_context (pc, pr->stack, sr_stack_size, arg1, arg2, arg3, arg4);

    if ((res != NULL)  && (!have_res_mutex))
	UNLOCK (res->rmutex, "sr_spawn");

    DEBUG (D_SPAWN, "r%06lX spawn    p%06lX", CUR_RES, pr, 0);
    return pr;
}


/*
 *  Place a process created by sr_spawn () the ready list.
 */
void
sr_activate (pr)
Proc pr;
{
    LOCK_QUEUE ("sr_activate");	/* protect pr->status */
    pr->status = READY;
    add_readyq (pr);
    DEBUG (D_ACTIVATE, "r%06lX activate p%06lX (%s)", CUR_RES, pr, pr->pname);
    UNLOCK_QUEUE ("sr_activate");
}



/*
 *  Kill an SR process:
 *	Remove process from list for its resource (always).
 *	Remove process from appropriate scheduler list.
 *	Free process descriptor.
 *	Don't free the stack; we can probably reuse it, and if we're
 *	    killing ourself then we're still using it.
 *
 *  Note: nobody calls sr_kill holding a mutex >= sr_queue_mutex.
 *
 *  res_mutex_held points to pr->res if the caller holds pr->res->rmutex
 *  and is NULL otherwise.
 */
void
sr_kill (pr, res_mutex_held)
Proc pr;
Rinst res_mutex_held;
{
    Rinst res = pr->res;
    Bool released_queue_mutex = FALSE;

    /* we need to get res->rmutex if it exists and if we don't have it */
    if ((res != NULL) && (res_mutex_held == NULL))
	LOCK (res->rmutex, "sr_kill #01");

    /* The queue mutex protects pr->should_die and pr->blocked_on */
    LOCK_QUEUE ("sr_kill #02");

    /*  Remove pr from its resource list, if it belongs to a res and
     *  if someone else isn't killing it (let killer remove from list). */
    if ((res != NULL) && (!pr->should_die))
	rem_proc (pr);

    switch (pr->status) {

	case ACTIVE:
	case DOING_IO:
	    if (pr != CUR_PROC) {
		Sem wait;
		wait = sr_make_sem (0);
		pr->waiting_killer = wait;
		pr->should_die = TRUE;
		/* can't P while holding these or RTS could hang! */
		UNLOCK_QUEUE ("sr_kill #03");
		released_queue_mutex = TRUE;
		if (res != NULL)
		    UNLOCK (res->rmutex, "sr_kill #04");
		P ((char *) 0 , wait);
		sr_kill_sem (wait);
		/* if we had rmutex coming in, then we'd better regain it */
		if (res_mutex_held != NULL)
		    LOCK (res->rmutex, "sr_kill #05");
	    } else {
		if (res != NULL)	/* release rmutex if we hold it */
		    UNLOCK (res->rmutex, "sr_kill #06");
		pr->status = TO_BE_FREED; /* will be freed by sr_scheduler () */
		sr_scheduler ();
	    }
	    break;

	case READY:
	    dequeue (&sr_ready_list, pr);
	    free_proc (pr, "sr_kill");
	    break;

	case BLOCKED:
	case BLOCKED_DOING_IO:
	    dequeue (pr->blocked_on, pr);
	    free_proc (pr, "sr_kill B");
	    break;

	case INFANT:
	    free_proc (pr, "sr_kill I");
	    break;

	default: {
	    char msg[100];
	    sprintf (msg, "illegal process status %d for kill", pr->status);
	    sr_malf (msg);
	}
    }
    if (!released_queue_mutex)
	UNLOCK_QUEUE ("sr_kill #13");

    /* if we grabbed rmutex above then we must release it. */
    if ((res != NULL) && (res_mutex_held == NULL))
	UNLOCK (res->rmutex, "sr_kill #15");
}



/*  Reschedule pr on the appropriate ready list.  The caller must
 *  already hold sr_queue_mutex and then continue to hold it while
 *  calling sr_scheduler ().
 */
void
sr_reschedule (pr)
Proc pr;
{
    pr->status = READY;		/* caller must have sr_queue_mutex */
    if (pr->ptype == IDLE_PR)
	sr_enqueue (&sr_idle_list, pr);
    else
	add_readyq (pr);
}


/*  Reschedule the current proc to possibly let someone else run.
 *  Check if any napping or I/O blocked jobs can be awakened.
 *  Called by the GC when enough loops have been run.
 */
void
sr_loop_resched (locn)
char *locn;
{
    char buf[100];

    DEBUG (D_LOOP, "loop_resched at %s", sr_fmt_locn (buf, locn), 0, 0);
    PRIV (rem_loops) = sr_max_loops;	/* reset the loop counter */
    if (I_CHECK_TERMINATION)	/* only check on the appropriate jobserver */
	sr_evcheck (0);		/* check I/O ready; immediate return */
    LOCK_QUEUE ("sr_loop_resched");
    sr_reschedule (CUR_PROC);
    sr_scheduler ();
}



/*
 *  A context switch is requested.  Allow the next process (if there is one)
 *  on the appropriate list (as determined by which job server called this)
 *  to execute.
 *
 *  We do not requeue the current proc on a ready list.
 *  The caller must do that.
 *
 *  The caller of this routine must have acquired sr_queue_mutex before
 *  queueing CUR_PROC.
 */
void
sr_scheduler ()
{
    Proc pr;
    char *old_stack;

#ifdef MULTI_SR
    if (PRIV (js_queue_depth) != 1)
	sr_malf ("js_queue depth not 1 in sr_scheduler");
#endif

    /*
     *  if somebody else is trying to kill CUR_PROC, we let the
     *  killer proceed and extinguish the caller.  We need to do this
     *  before trying to find the next proc to run, or otherwise
     *  we might find CUR_PROC on the ready list, which would
     *  make things complicated.
     */
    if (CUR_PROC != NULL && CUR_PROC->should_die)
	handle_should_die ();

    /* Find something to run */

    pr = dequeue_first (MY_QUEUE_ADDR);

    if (pr == NULL) {
	pr = dequeue_first (&sr_idle_list);
	if (pr == NULL)
	    sr_malf ("no idle procs");
    }

    /* with the IO servers BEGIN_IO changes the status to DOING_IO, and
     * it is not until END_IO that it gets set back to ACTIVE.
     * Note that we must set the status under the queue mutex protection. */

#ifdef UNSHARED_FILE_OBJS
    if (pr->status == BLOCKED_DOING_IO)
	sr_malf ("BLOCKED_DOING_IO pr in sr_scheduler");
    if (pr->status != DOING_IO)
#endif
	pr->status = ACTIVE;	/* unconditional if not UNSHARED_FILE OBJS */

#ifndef MULTI_SR
    /* free the current process if it's no longer needed */
    /* [deferred until now to prevent reuse of its stack] */
    /* this has to be done with switch_proc in MultiSR */

    if (CUR_PROC != NULL && CUR_PROC->status == TO_BE_FREED)
	free_proc (CUR_PROC, "sr_scheduler");
#endif

    /* start the process; if it's the current one that's really easy */
    DEBUG (D_RESTART, "r%06lX switchto p%06lX prio %ld",
	CUR_RES, pr, pr->priority);

    if (pr == CUR_PROC) {
	UNLOCK_QUEUE ("sr_scheduler");		/* cswitch won't !! */
	return;
    }

    /* variables for switch_proc to free and acquire stack locks with */
    PRIV (old_proc) = CUR_PROC;
    CUR_PROC = pr;
    CUR_RES = pr->res;

    /* CUR_PROC could be 0 at top */
    old_stack = (PRIV (old_proc) ?  PRIV (old_proc)->stack : 0);
#ifdef MULTI_SR
    sr_chg_context (PRIV (switch_stack), old_stack);
#else
    sr_chg_context (pr->stack, old_stack);
#endif

}


/*  The calling proc should_die.  We yank it off of any queue it is on,
 *  signal the killer, etc. This is only called from sr_scheduler.  */
static void
handle_should_die ()
{
    if ((CUR_PROC->status == ACTIVE)
    || (CUR_PROC->status == DOING_IO))
	sr_malf ("active thread in handle_should_die");

    /*  Remove the caller off of any queue its on.  We can't release
     *  the queue mutex and then call sr_kill, because someone else
     *  could grab it off of the ready queue.  Thus, we need to do
     *  this while we hold the queue mutex.  This below just mimics
     *  some of the logic in sr_kill.  */

    switch (CUR_PROC->status) {
	case READY:
	    dequeue (&sr_ready_list, CUR_PROC);
	    break;

	case BLOCKED:
	case BLOCKED_DOING_IO:
	    dequeue (CUR_PROC->blocked_on, CUR_PROC);
	    break;

    }  /* end switch */
    CUR_PROC->status = TO_BE_FREED;		/* switch_proc frees */

    /* should_die not protected here, since only killer and CUR_PROC
     * could have a handle on it */
    CUR_PROC->should_die = FALSE;
    V (CUR_PROC->waiting_killer);
    CUR_PROC->waiting_killer = NULL;
}



/*
 *  Add a process to the given queue, which is sorted by priority.
 */
static void
add_readyq (pr)
Proc pr;
{
    Proc *ptr;		/* pointer used to walk down ready list */


    LOCK_QUEUE ("add_readyq");

    if (pr == NULL)
	sr_malf ("null pr in add_readyq");
    if (pr->status != READY)
	sr_malf ("adding unready process to ready list");

    if (sr_ready_list.head == NULL) {
	/* queue is empty */
	sr_ready_list.head = sr_ready_list.tail = pr;
	pr->next = NULL;
    } else if (sr_ready_list.tail->priority >= pr->priority) {
	/* Last proc in queue has higher priority than proc being added.
	 * Add process to the tail of the ready queue.
	 */
	sr_ready_list.tail->next = pr;
	sr_ready_list.tail = pr;
	pr->next = NULL;
    } else {
	/* process goes somewhere in the middle (or head) of ready list */
	ptr = &sr_ready_list.head;
	while ((*ptr)->priority >= pr->priority)
	    ptr = & ((*ptr)->next);
	pr->next = *ptr;
	* (ptr) = pr;
    }

    UNLOCK_QUEUE ("add_readyq");
}



/*
 *  Add a process to the end of the given queue.
 */
void
sr_enqueue (pl, pr)
Procq *pl;
Proc  pr;
{
    if (pl == &sr_ready_list) {
	add_readyq (pr);
	return;
    }

    LOCK_QUEUE ("sr_enqueue");

    if ((pl == NULL) || (pr == NULL))
	sr_malf ("null arg to sr_enqueue");
    if ((pr->status != READY) && (pl == &sr_ready_list))
	sr_malf ("enqueueing unready process on ready list");

    if (pl->head == NULL) {
	pl->head = pr;
	pl->tail = pr;
    } else {
	pl->tail->next = pr;
	pl->tail = pr;
    }
    pr->next = NULL;

    UNLOCK_QUEUE ("sr_enqueue");
}



/*
 *  Remove a process descriptor from the specified queue;  assume it's there.
 */
static void
dequeue (pl, pr)
Procq *pl;
Proc pr;
{
    Proc tpp;

    DEBUG (D_KILL, "        *%06lX dequeue  p%06lX", pl, pr, 0);

    LOCK_QUEUE ("dequeue");
    if ((tpp = pl->head) == pr)
	pl->head = pr->next;
    else {
	if (tpp == NULL)
	    sr_malf ("dequeue failed -- queue empty");
	while (tpp->next != pr){
	    tpp = tpp->next;
	    if (tpp == NULL)
		sr_malf ("dequeue failed -- not on queue");
	}
	if (tpp->next == pl->tail)
	    pl->tail = tpp;
	tpp->next = pr->next;

	if (pr->status == ACTIVE)
	    sr_malf ("dequeue found an active process");
    }
    UNLOCK_QUEUE ("dequeue");
}


/*
 *  Remove the first process descriptor from the specified queue.
 *  If the queue is empty then NULL will be returned.
 */
static Proc
dequeue_first (pl)
Procq *pl;
{
    Proc first;

    LOCK_QUEUE ("dequeue_first");
    if (pl->head == NULL)
	first = (Proc) NULL;
    else {
	first = pl->head;
	pl->head = first->next;
	if (first->next == NULL)
	    pl->tail = NULL;
	first->next = NULL;
	if (first->status == ACTIVE)
	    sr_malf ("dequeue_first found active process");
    }
    UNLOCK_QUEUE ("dequeue_first");
    return first;
}


/* return priority of current process */
int
sr_mypri ()
{
    return CUR_PROC->priority;

}



/* set priority of current process */
void
sr_setpri (newpri)
int  newpri;
{
    int oldpri;

    oldpri = CUR_PROC->priority;
    CUR_PROC->priority = newpri;
    if (newpri < oldpri) {
	/* reschedule self after acquiring sr_queue_mutex. */
	LOCK_QUEUE ("sr_setpri");
	sr_reschedule (CUR_PROC);
	sr_scheduler ();
    }
}



/*
 *  Remove a process from the list of processes associated with the specified
 *  resource.  The caller must possess rmutex.
 */
static void
rem_proc (pr)
Proc pr;
{
    Proc this;
    Proc last = NULL;
    for (this = pr->res->procs; this; this = this->procs) {
	if (this == pr) {
	    if (last == NULL)
		pr->res->procs = this->procs;
	    else
		last->procs = this->procs;
	    return;
	}
	last = this;
    }
    sr_malf ("rem_proc: proc not found on resource list");
}



/*
 *  Initialize the process management system:
 *	Set up the free list of process descriptors.
 *	Create an SR process context for the startup code and enqueue it.
 *	Create the vm job servers (the first to get to the queue will
 *					execute the startup code).
 *	DOES NOT RETURN.
 */

void
sr_init_proc (start_code)
Func start_code;
{
    int i;
    Proc pr;
    char *s;

    if (s = getenv (ENV_SPIN))
	spin_count = atoi (s);
    if (s = getenv (ENV_NAP))
	timeout = atoi (s);

    /* ensure max_loops is strictly positive */
    /* (not locked, because only one process is active at this time) */ 
    if (sr_max_loops <= 0)
	sr_max_loops = MAX_INT;		/* treat 0 as effectively infinite */

    sr_init_sem ();			/* initialize semaphores */

    /* initialize queues; again, no locking, because we're the only process */
    sr_ready_list.head = sr_ready_list.tail = NULL;
    sr_io_list.head = sr_io_list.tail = NULL;
    sr_idle_list.head = sr_idle_list.tail = NULL;

    /* initialize the meta-lock that protects the termination decision */
    INIT_LOCK (sr_queue_mutex, "queue_mutex");

    proc_pool = sr_makepool ("processes", sizeof (struct proc_st),
	sr_max_processes, init_proc, re_init_proc);

    /* make an SR process to execute the startup code */
    pr = (Proc) sr_addpool (proc_pool);
    pr->stack = newstack (sr_stack_size);
    pr->priority = 0;
    pr->status = READY;
    pr->next = NULL;
    pr->res = NULL;
    pr->ptype = INITIAL_PR;
    pr->pname = "[startup]";
    pr->should_die = FALSE;
    sr_build_context (start_code, pr->stack, sr_stack_size, 0L, 0L, 0L, 0L);
    add_readyq (pr);

    /* if PRIV changes, need to change this, too. */
    for (i = 0; i < MAX_JOBSERVERS; i++)
	sr_private[i].cur_proc = sr_private[i].old_proc = 0;

    /*  set up the proc for the job servers to use as their initial CUR_PROC. */
    for (i = 0; i < sr_num_job_servers; i++) {
	pr = (Proc) sr_addpool (proc_pool);
	pr->next = NULL;
	pr->res = NULL;
	pr->stack = NULL;		/* flag for sr_chg_context */
	pr->status = TO_BE_FREED;
	pr->pname = "[JSdummy]";
	sr_private[i].cur_proc = pr;
    }

    sr_kill_wait = sr_make_sem (0);

    if (sr_num_job_servers == 1) {
	sr_scheduler_queue_addr[0] = &sr_ready_list;
	timeout = -1;		/* never timeout when blocked on I/O */
    } else {
	sr_scheduler_queue_addr[0] = &sr_io_list;
	for (i = 1; i < sr_num_job_servers; i++)
	    sr_scheduler_queue_addr[i] = &sr_ready_list;
    }

    /* create job servers; our thread of control dies here */
    sr_create_jobservers (vm_jobserver, sr_num_job_servers);
    sr_malf ("sr_create_jobservers returned");
}



/*
 * init_proc -- first-time proc initialization
 */
static void
init_proc (p)
Proc p;
{
    p->stack = NULL;
    INIT_LOCK (p->stack_mutex, "p->stack_mutex");
}



/*
 *  re_init_proc -- initialize a proc descriptor for use
 */
static void
re_init_proc (p)
Proc p;
{
    p->ptype = FREE_PR;
    p->status = FREE;
    p->priority = 0;
    p->pname = "[freed]";
    p->should_die = FALSE;
    p->blocked_on = NULL;
    RESET_LOCK (p->stack_mutex);
}



/*
 * print out one blocked process
 */
static void
print_blocked (pr)
struct proc_st *pr;
{
    char buf[100];
    char buf1[50];

    if ((pr->pname != NULL)
    && (isalpha (pr->pname[0]))
    && (pr->status == BLOCKED || pr->status == BLOCKED_DOING_IO)) {
	sr_fmt_locn (buf1, pr->locn); 
	sprintf (buf, "blocked process: %s : %s", pr->pname, buf1);
	RTS_WARN (buf);
    }

}


void
sr_print_blocked ()
{
    LOCK_QUEUE ("sr_print_blocked");
    sr_eachpool (proc_pool, print_blocked);
    UNLOCK_QUEUE ("sr_print_blocked");
}



#ifdef MULTI_SR
/*
 *  called from sr_scheduler, via sr_chg_context (PRIV (switch_stack)),
 *  switch_proc releases the lock for the stack sr_scheduler entered with,
 *  releases the queue lock, then grabs the lock for the stack it will
 *  become next, and then changes context to that new stack.
 */
static void
switch_proc ()
{
    for (;;) {
	if (PRIV (old_proc) != NULL)
	    UNLOCK ((PRIV (old_proc))->stack_mutex, "switch_proc");
	/*
	 *  free the current process if it's no longer needed.
	 *  [deferred until now to prevent reuse of its stack].
	 *  Note this has to be done by the same JS.
	 */
	if (PRIV (old_proc) != NULL && PRIV (old_proc)->status == TO_BE_FREED)
	    free_proc (PRIV (old_proc), "switch_proc");
	UNLOCK_QUEUE ("switch_proc");
	LOCK (CUR_PROC->stack_mutex, "switch_proc");
	sr_chg_context (CUR_STACK, PRIV (switch_stack));
    }
}
#endif /* MULTI_SR */



/*
 * VM job server - grab the first process on the ready list and become it.
 * Argument *arg* is unused here and simply passed along to sr_jobserver_first.
 */
static void
vm_jobserver (arg)
void *arg;
{
    int id;

    sr_jobserver_first (arg);
    id = PRIV (my_jobserver_id) = MY_JS_ID;

    DEBUG (D_GENERAL, "job server %ld alive, pid %ld, ppid %ld",
	id, getpid (), getppid ());

    /* 
     * Grab the lock of the thread that is our current (dummy) proc,
     * so it is held when switch_proc (via scheduler) releases it. 
     * Some threads packages care about this, and even report 
     * if a Mutex not previously LOCKed is UNLOCKed, or if one
     * OS process LOCKs a Mutex and another process UNLOCKs it! 
     */
    LOCK (CUR_PROC->stack_mutex, "vm_jobserver");


#ifdef MULTI_SR
    /* make context switch proc */
    PRIV (switch_stack) = newstack (SWITCH_PROC_STACKSIZE);
    sr_build_context (switch_proc, PRIV (switch_stack), SWITCH_PROC_STACKSIZE,
	0, 0, 0, 0);
#endif

    make_idle_proc ();

    /*  sr_init_proc already set CUR_PROC to &pr[1], so sr_scheduler ()
     *  is ready to go. And the function knows which queue this JS
     *  pulls jobs off of. */

    DEBUG (D_SPAWN, "JS %ld calling sr_scheduler ()", id, 0, 0);
    LOCK_QUEUE ("vm_jobserver");
    sr_scheduler ();
    sr_malf ("sr_scheduler returned to vm_jobserver");
}



/*
 *  The idle process: scheduled when there's nothing else to run.
 *
 *  We loop forever, marking our job server as idle, sleeping briefly,
 *  possibly checking events and (local) termination, and then calling
 *  sr_scheduler to let anyone else run.  We employ an asymmetric solution
 *  to event checking and local termination: only the IO server does this.
 */

static void
idle_proc (my_id)
int my_id;
{
    char my_label[20];
    struct timeval tv;

    sprintf (my_label, "idle_proc (%d)", my_id);
    for (;;) {
	if (!check_ready ())  {
	    if (I_CHECK_TERMINATION) {
		/* check events and consider termination, all under lock */
		LOCK_QUEUE (my_label);
		check_idle ();			/* check if locally idle */
		UNLOCK_QUEUE (my_label);
		/* if no idle process created, wait (a while or forever) */
		if (sr_ready_list.head == NULL)	/* if still nothing to run */
		    sr_evcheck (timeout);	/* check for I/O; may block */
	    } else {
		tv.tv_sec = timeout / 1000;	/* delay SR_NAP_INTERVAL */
		tv.tv_usec = 1000 * (timeout % 1000);
		select (1, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &tv);
	    }
	}
	LOCK_QUEUE (my_label);		/* need queue_mutex to call scheduler */
	sr_reschedule (CUR_PROC);
	sr_scheduler ();
    }
    /*NOTREACHED*/
}



/*
 *  check_idle(): check if idle and tell somebody if so.
 *
 *  The queue_mutex is currently held.
 */
static void
check_idle ()
{
    Proc pr;

    if (sr_idle_list.head == NULL	/* idle jobs should all be running */
    &&  sr_io_list.head == NULL		/* nobody waiting for I/O server */
    &&  sr_ready_list.head == NULL	/* nobody waiting to run */
    &&  sr_nap_list_empty ()		/* nobody napping */
    &&  sr_evio_list_empty ()) {	/* nobody blocked on I/O */

	if (sr_exec_up) {

	    /* we have found local deadlock; srx will evaluate globally */
	    if (sr_msg_counts.nmsgs[sr_my_vm] == idle_count)
		return;			/* no new input since last time idle */
	    idle_count = sr_msg_counts.nmsgs[sr_my_vm];
	    DEBUG (D_TERM, "locally idle on vm %ld (input total %ld)",
		sr_my_vm, -idle_count, 0);
	    sr_net_send (SRX_VM, MSG_IDLE,
		&sr_msg_counts.ph, sizeof (sr_msg_counts));

	} else {			/* we are the only VM */

	    /* Spawn a process to run sr_stop (exitcode=0, report_blocked=1). */
	    pr = sr_spawn (sr_stop,
		CUR_PROC->priority, (Rinst) NULL, FALSE, 0L, 1L, 0L, 0L);
	    pr->pname = "[shutdown]";
	    DEBUG (D_GENERAL, "WE ARE IDLE: activating sr_stop p%06lX", pr,0,0);
	    sr_activate (pr);
	}
    }
}



/*
 *  Check ready queue and return TRUE if non-empty.
 *
 *  If there are other processes running, spin a little bit to give them
 *  a chance to make something ready.
 */

static Bool
check_ready ()
{
    int i = 0;
    int j;
    int k = 0;
    int l = 2;

    if (MY_QUEUE_ADDR->head != NULL)
	return TRUE;
    if (sr_num_job_servers == 1)
	return FALSE;
    /* 
     * The loop body here is at least slightly nontrivial to try and inhibit
     * its optimization.  Also, if it is modified, we'd need to recalibrate
     * its timings.
     */
    for (j = 0; j < spin_count; j++)
	if (MY_QUEUE_ADDR->head != NULL)
	    return TRUE;
	else
	    k += l + i;
    l = k + 1;				/* keep lint happy */
    return FALSE;
}



/*
 *  make_idle_proc creates an idle proc and schedules it.
 */

static void
make_idle_proc ()
{
    Proc idle_pr;
    static int num_created = 0;
    int my_created;
    static char namebuf[MAX_JOBSERVERS+1][20], *mybuf;

    LOCK_QUEUE ("make_idle_proc");
    idle_pr = (Proc) sr_addpool (proc_pool);
    my_created = ++num_created;
    mybuf = namebuf[my_created];

    idle_pr->stack = newstack (IDLE_PROC_STACKSIZE);
    idle_pr->priority = -9;  /* doesn't matter - not used */
    idle_pr->status = READY;
    idle_pr->ptype = IDLE_PR;
    idle_pr->next = NULL;
    idle_pr->res = NULL;
    sprintf (mybuf, "idle (%d)", my_created);
    idle_pr->pname = mybuf;
    sr_build_context (idle_proc, idle_pr->stack, IDLE_PROC_STACKSIZE,
	(long) my_created, 0L, 0L, 0L);
    /* we can't use activate since they don't go on sr_ready_list */
    sr_enqueue (&sr_idle_list, idle_pr);
    UNLOCK_QUEUE ("make_idle_proc");
}



/*
 *  The threads package MultiSR was ported to has not given enough threads.
 *  Complain and quit.
 */
void
sr_missing_children (requested, granted)
int requested, granted;
{
    char buf[100];
    sprintf (buf, "could not honor %s=%d; got only %d processes",
	ENV_PARALLEL, requested, granted);
    sr_abort (buf);
}



/*  newstack (size) -- allocate a new stack, ensuring proper alignment
 *
 *  (The Iris malloc, at least, can return memory that is misaligned
 *   for use as a stack.)
 */

static Ptr
newstack (size)
int size;
{
    return (Ptr) SRALIGN ((long) sr_alloc (size + GRAN - 1));
}
