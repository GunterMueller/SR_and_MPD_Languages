/*  main.c -- overall control and error handling for SR programs  */



/*  This module is the one that actually defines the globals.  */

#define global
#define initval(v) = v
#include "rts.h"


#ifdef __PARAGON__
#include <nx.h>
parameter vm_parameter;		/* parameter struct to start VM */
extern local_message_type;	/* message type == ptype of current VM */
#endif


#include <stdarg.h>

#define RUNERR(s,n,m) {n, m},
static struct err {
    int errnum;
    char *format;
} errlist [] = {
#include "../runerr.h"
    { 0, 0 }
};



static Bool init_done = FALSE;		/* is the RTS initialized yet? */
static Mutex final_mutex;		/* protect sr_stop stuff */
static struct proc_st dummy_proc;

static void startup (), finalize (), shutdn ();



/*
 *  Start up an SR program or one of its virtual machines.
 *
 *  For the original startup, argument interpretation is up to the programmer.
 *  For a new virtual machine, args are:
 *	argv[1]	    a magic string indicating that this is a VM startup
 *	argv[2]	    physical machine number for the new VM
 *	argv[3]	    virtual machine number for the new VM
 *	argv[4]	    network address of srx's connection port
 *	argv[5]	    debugging flags
 *	argv[6]	    sr_num_job_severs
 *	argv[7]	    trace fd, or -1 if no tracing (not applicable for Paragon)
 */
main (argc, argv)
int argc;
char **argv;
{
    char *s;
    int i;
#ifdef __PARAGON__
    long n;
    int stat;
    pid_t *pids;
#endif

    sr_argv = argv;
    sr_num_job_servers = 1;

#ifdef __PARAGON__

    setpgid (0, 0);		/* create new process group */

    /* this code runs on every compute node */
    if (nx_initve (NULL, 0, NULL, &argc, argv) < 0)	/* initialize */
	{ perror ("nx_initve"); exit (1); }
 
    pids = malloc (sizeof (pid_t) * numnodes ());	/* alloc PID array */

    n = nx_nfork (NULL, -1, 0, pids);		/* fork all distributors */
    if (n < 0)
	{ perror ("nx_nxfork"); exit (1); }

    if (n > 0) {		/* if parent */
	while (n-- > 0)
	    wait (&stat);	/* wait until every distributor terminates */
	exit (stat >> 8);	/* exit with status of the last distributor */
    }
#endif

    /* ensure that stderr is unbuffered */
    setbuf (stderr, (char *) NULL);

    sr_init_multiSR ();		/* init stuff in file ../srmulti.c */

    /*  dummy_proc is used for sr_abort (), etc, before we're running
     *  an SR or idle or network thread.
     *
     *  we have just one copy of the dummy proc, and don't free it (this
     *  might goof up the resource pool for proctab entries).  We have its
     *  status ACTIVE, but when sr_vm_job_server (called from sr_init_proc)
     *  calls sr_scheduler, dummy_proc.should_die will be FALSE (if it
     *  weren't this could goof up sr_scheduler).
     */
    dummy_proc.pname = "[dummy_proc]";
    dummy_proc.res = (Rinst) NULL;
    dummy_proc.status = ACTIVE;
    dummy_proc.should_die = FALSE;
    INIT_LOCK ((&dummy_proc)->stack_mutex, "dummy->stack_mutex");
    CUR_PROC = &dummy_proc;	
    CUR_RES = (Rinst) NULL;

    for (i = 0; i <= LAST_SHARED_FD; i++)
	INIT_LOCK (sr_fd_lock[i], "sr_fd_lock[i]");

    INIT_LOCK (final_mutex, "final_mutex");	/* for sr_stop progeny */


#ifndef __PARAGON__
    if (argc != 8 || strcmp (argv[1], VM_MAGIC) != 0) {
	sr_init_trace ((char *) NULL);
#else
    if ((mynode () == 0) && (fork () == 0)) {	/* on node 0 fork VM 0 */
	setptype (local_message_type);	
	sr_init_trace ((char *) NULL);
	if (sr_trc_flag) {
	    lseek (sr_trc_fd, (off_t) 0, SEEK_SET);
	    ftruncate (sr_trc_fd, (off_t) 0);
	    }
#endif

	/* this is the initial startup */
	sr_argc = argc;
	sr_my_vm = MAIN_VM;
	sr_init_debug ((char *) NULL);
	s = getenv (ENV_PARALLEL);
	if (s != NULL && (i = atoi (s)) > 1) {	/* multiprocessing requested */

#ifdef MULTI_SR
	    sr_num_job_servers = i + NUM_IO_SERVERS;	/* add I/O servers */
	    if (sr_num_job_servers > MAX_JOBSERVERS)  {
		char msg[100];

		sr_num_job_servers = 1;
		sprintf (msg, "%s=%d exceeds allowable maximum of %d",
		    ENV_PARALLEL, i, MAX_JOBSERVERS - NUM_IO_SERVERS);
		sr_abort (msg);
		/*NOTREACHED*/
		}

#else /* not MULTI_SR */

	    if (i > 1)	 {
		char msg[100];
		sprintf (msg, "No parallelism configured (%s=%u ignored)",
		    ENV_PARALLEL, i);
		RTS_WARN (msg);
	    }

#endif /* MULTI_SR */
	}

    } else {

	sr_argc = 0;		/* hide args from SR program */

#ifdef __PARAGON__
	/*
	 * The distributor code runs on every node.
	 * For every parameter struct received one VM is forked.
	 * Termination if phys_machine == -1 with the exit code in the 
	 * virt_machine field.
	 * The local_message_type has to exist on every node only once and
	 * is equal to the ptype.
	 */
	for (;;) {			/* parent repeats until termination */
	    local_message_type ++;
	    crecv (0, &vm_parameter, sizeof (vm_parameter));
	    if (vm_parameter.phys_machine == -1)
		exit (vm_parameter.virt_machine);
	    n = fork ();
	    if (n < 0)
	        { perror ("fork"); exit (1); }
	    if (n == 0)
		break;			/* child */
	}
	/*
	 *  Child process: new virtual machine.
	 */
	setptype (local_message_type);	
	sr_my_machine = vm_parameter.phys_machine;
	sr_my_vm = vm_parameter.virt_machine;
	sprintf (sr_my_label, "[vm %d] ", sr_my_vm);
	sr_init_debug ((char *) NULL);	
	sr_init_trace ((char *) NULL);
#else
	/*
	 *  This is a new virtual machine.
	 */
	sr_my_machine = atoi (argv[2]);
	sr_my_vm = atoi (argv[3]);
	sprintf (sr_my_label, "[vm %d] ", sr_my_vm);
	sr_init_debug (argv[5]);
	sscanf (argv[6], "%u", &sr_num_job_servers);
	sr_init_trace (argv[7]);
#endif
    }

#ifdef MULTI_SR
    DEBUG (D_GENERAL, "%ld job server(s)", sr_num_job_servers, 0, 0);
#endif

    sr_stack_size = SRALIGN (sr_stack_size);	/* ensure legal stack size */
    sr_init_proc (startup);	/* init processes (no return; calls startup) */
    /*NOTREACHED*/
}

/*
 *  initialization continues here in an SR process context.
 */
static void
startup ()
{
    CRB *crbp;

    CUR_PROC -> pname = "[startup]";

    /* initialize runtime system */
    sr_init_event ();
    sr_init_mem ();
    sr_init_res ();
    sr_init_oper ();
    sr_init_class ();
    sr_init_co ();
    sr_init_rem ();
    sr_init_vm ();
    sr_init_io ();
    sr_init_misc ();
    sr_init_random ();

    /* no better place to put this since sr_init_net () not called like above */
    INIT_LOCK (sr_exec_up_mutex, "exec_up_mutex");

    DEBUG0 (D_GENERAL, "startup code done initializing RTS");
    if (sr_my_vm == MAIN_VM) {
	init_done = TRUE;	/* RTS initialized */
	/* create an instance of main resource on main machine */
	crbp = (CRB *) sr_alc (-sizeof (struct crb_st), 1);
	crbp->crb_size = sizeof (struct crb_st);
	sr_create_resource ((char *) NULL, 0, sr_my_vm, crbp, sizeof (Rcap));
    } else {
	/* otherwise, just start up network and wait for instructions */
	sr_argc = 0;		/* hide args from users */
#ifndef __PARAGON__
	sr_init_net (sr_argv[4]);
#else
	sr_init_net (vm_parameter.srx_addr);
#endif
	init_done = TRUE;	/* RTS initialized */
    }

    /* kill this initialization process */
    sr_kill (CUR_PROC, (Rinst) NULL);
}



/*
 * Abort giving source line information.
 */
void
sr_loc_abort (locn, msg)
char *locn, *msg;
{
    char buf[200];

    if (!locn)
	sr_abort (msg);			/* no trace info available */
    sr_fmt_locn (buf, locn);
    strcat (buf, ":\n   ");
    strcat (buf, msg);
    sr_abort (buf);
}



/*
 * Translate a "locn" pointer into a text string.
 */
char *
sr_fmt_locn (buf, locn)
char *buf, *locn;
{
    int lno;

    if (!locn) {
	strcpy (buf, "unknown location");
	return buf;
    }
    lno = 1;
    while (*--locn != '\002')
	lno++;
    sprintf (buf, "file %s, line %d", locn + 1, lno);
    return buf;
}



/*
 *  sr_runerr (locn, errnum, args...)
 *  Runerr never really returns, but it is declared as returning int
 *  because of the way it is used in the generated code.
 */
int
sr_runerr (char *locn, int errnum, ...)
{
    va_list ap;
    char c, *f, *o;
    struct err *ep;
    char buf[200];
    Dim *d;
    String *s;

    va_start (ap, errnum);

    for (ep = errlist; ep->errnum != errnum; ep++)
	if (ep->errnum == 0) {
	    sprintf (buf, "unknown runtime error %d", errnum);
	    sr_loc_abort (locn, buf);
	}

    o = buf;
    f = ep->format;
    while ((c = *f++) != '\0') {
	if (c != '%')
	    *o++ = c;
	else {
	    switch (c = *f++) {
		case 'd':			/* integer */
		    sprintf (o, "%d", va_arg (ap, int));
		    break;
		case 'B':			/* both array bounds */
		    d = va_arg (ap, Dim *);
		    sprintf (o, "[%d:%d]",  d->lb, d->ub);
		    break;
		case 'L':			/* string length */
		    s = va_arg (ap, String *);
		    sprintf (o, "%d", s->length);
		    break;
		case 'M':			/* string maxlength */
		    s = va_arg (ap, String *);
		    sprintf (o, "%d", MAXLENGTH (s));
		    break;
		case 'S':
		    s = va_arg (ap, String *);
		    if (s->length <= 25) {
			DATA (s) [s->length] = '\0';
			sprintf (o, "%s", DATA (s));
		    } else {
			sprintf (o, "%.20s...", DATA (s));
		    }
		    break;
		default:			/* % (or bogus) */
		    sprintf (o, "%c", c);
		    break;
	    }
	    o += strlen (o);
	}
    }
    va_end (ap);
    *o++ = '\0';
    sr_loc_abort (locn, buf);
    /*NOTREACHED*/
}



/*
 *  Print run-time support error or warning message.
 *
 *  We do this with a single, self-buffered atomic write to fd 2 to prevent
 *  interference between messages from different virtual machines.
 */
void
sr_message (s, t)
char *s, *t;
{
    char buf[200];
    sprintf (buf, "%sRTS %s: %s\n", sr_my_label, s, t);

    BEGIN_IO (stdout);
    fflush (stdout);		/* flush any user output first */
    END_IO (stdout);

    BEGIN_IO (stderr);
    write (2, buf, strlen (buf));
    END_IO (stderr);
}



/*
 *  Fatal run time support error.
 *  Print error message and tell srx to abort other nodes.
 */
void
sr_abort (s)
char *s;
{
    DEBUG (D_ALL_FLAGS, "ABORT: %s", s, 0, 0);
    sr_message ("abort", s);
    sr_stop (1, 0);
}



/*  Fatal error detected by network routines.  */

void
sr_net_abort (s)
char *s;
{
    sr_abort (s);
}



/*  Runtime malfunction -- for "can't happen" situations.  */

void
sr_malf (s)
char *s;
{
    sr_message ("malfunction", s);
    sr_stop (1, 0);
}



/*
 *  The six steps of distributed termination in SR are (note that
 *  if we only have one VM sr_exec_up is false):
 *
 *	1)	Some VM calls sr_stop.  MSG_STOP is sent to SRX.
 *
 *		(If a node becomes inactive then MSG_IDLE is passed to SRX.
 *		 This is for distributed deadlock detection.)
 *
 *	2)	SRX passes the MSG_STOP to the main VM.
 *
 *	3)	The main VM calls sr_stop.  Finalization is done here.
 *
 *	4)	As part of the main VM's shutdown, MSG_EXIT is sent to SRX.
 *
 *	5)	SRX broadcasts MSG_EXIT to all non-main VMs.
 *
 *	6)	At each non-main VM, sr_net_interface catches the
 *		MSG_EXIT and just exits.
 */



/*  sr_stop(code,report) -- initiate termination.
 *
 *  sr_stop does not return, and thus should be called by a
 *  spawned proc if the caller needs to survive.
 *
 *  If called a second time with a nonzero exitcode,
 *  sr_stop exits immediately.
 */

void
sr_stop (exitcode, report_blocked)
int exitcode, report_blocked;
{
    static Bool exiting = FALSE;
    static Bool finals_started = FALSE;
    static Bool shutdn_started = FALSE;
    struct num_st pkt;
    Proc pr;

    DEBUG (D_GENERAL, "sr_stop(%ld,%ld)", exitcode, report_blocked, 0);
    if (exitcode != 0) {
	LOCK (final_mutex, "sr_stop");
	if (exiting) {
	    UNLOCK (final_mutex, "sr_stop");
	    EXIT (1);
	} else {
	    exiting = TRUE;
	    UNLOCK (final_mutex, "sr_stop");
	}
    }

    if (sr_my_vm != MAIN_VM) {
	/* We're not the main VM; just tell SRX to tell IT to stop. */
	pkt.num = exitcode;
	sr_net_send (SRX_VM, MSG_STOP, &pkt.ph, sizeof (pkt));
	sr_kill (CUR_PROC, (Rinst) NULL);
    }

    /*  If we might be starting the finalization code, we need to run
     *  disassociated from any resource so that we can't be killed 
     *  during finalization.  */

    if (sr_my_vm == MAIN_VM && exitcode == 0 && CUR_RES != NULL) {
	pr = sr_spawn (sr_stop, CUR_PROC->priority, (Rinst) NULL, FALSE,
	    (long) exitcode, (long) report_blocked, 0L, 0L);
	sr_activate (pr);
	sr_kill (CUR_PROC, (Rinst) NULL);
    }

    /*  We see if the final code needs to be fired up if it hasn't
     *  already been fired up.  Do_finals fires up any final code if
     *  the main resource hasn't been destroyed yet.  It then fires up
     *  the final code for the globals, from the bottom up.  */

    LOCK (final_mutex, "sr_stop");
    if (finals_started) {
	UNLOCK (final_mutex, "sr_stop");
    } else {
	finals_started = TRUE;
	UNLOCK (final_mutex, "sr_stop");
	if ((sr_my_vm == MAIN_VM) && (exitcode == 0))
	    finalize ();			/* checks if needed */
    }

    /*  We can now complete the shutdown if the final code has finished
     *  or if sr_stop was called for the second time (either via a SR
     *  stop statement or through deadlock/termination detection).
     *  We only do this if it hasn't been done already. */

    LOCK (final_mutex, "sr_stop");
    if (shutdn_started) {
	UNLOCK (final_mutex, "sr_stop");
    } else {	
	shutdn_started = TRUE;
	UNLOCK (final_mutex, "sr_stop");
	shutdn (exitcode, report_blocked);
    }

    /* kill this process */
    sr_kill (CUR_PROC, (Rinst) NULL);
    /*NOTREACHED*/
}



/*  finalize() runs the finalization -- the main resource's final code
 *  (if that is still around) and then the globals' final code.
 */

static void
finalize ()
{
    Proc pr;
    Sem waitsem;

    /*  if we are on the main resource and the main resource has final
     *  code, then we execute it if the resource hasn't already been
     *  destroyed and if we weren't invoked because of an sr_abort call.
     *  We need to grab the queue mutex to protect the interval between
     *  sr_spawn and sr_activate -- spawn puts the proc on a res list,
     *  where it could be destroyed...
     */

    LOCK (sr_main_res_mutex, "finalize");

    if (sr_main_res.res != NULL
	&& sr_main_res.seqn == sr_main_res.res->seqn++) {

	LOCK (sr_main_res.res->rmutex, "finalize");

	/* protect the spawn-activate region against sr_destroy */
	LOCK_QUEUE ("finalize");

	DEBUG0 (D_GENERAL | D_RESOURCE, "finalizing main resource");

	pr = sr_spawn (sr_rpatt[0].final, CUR_PROC->priority, sr_main_res.res,
			TRUE, (long) sr_main_res.res->crb_addr,
			(long) sr_main_res.res->rv_base, 0L, 0L);

	UNLOCK (sr_main_res.res->rmutex, "finalize");
	UNLOCK (sr_main_res_mutex, "finalize");

	pr->wait = waitsem = sr_make_sem (0);
	pr->pname = "[main final]";			/* GC resets */
	pr->ptype = FINAL_PR;
	sr_activate (pr);
	UNLOCK_QUEUE ("finalize");
	P ((char *) NULL, waitsem);
    } else {
	UNLOCK (sr_main_res_mutex, "finalize");
	DEBUG0 (D_GENERAL | D_RESOURCE, "no main resource to finalize");
    }

    DEBUG0 (D_GENERAL, "destroying main VM globals");
    sr_destroy_globals ();
}


/*  shutdn() initiates the final shutdown for the VM.  */

static void
shutdn (exitcode, report_blocked)
int exitcode, report_blocked;
{
    Bool exec_was_up;
    struct exit_st pkt;

    DEBUG (D_GENERAL, "shutdn(%ld,%ld)", exitcode, report_blocked, 0);

    if (!init_done) {
	/* RTS is not yet initialized (maybe even sr_exec_up_mutex),
	 * so its time to punt rather than risking a core dump */
	DEBUG0 (D_GENERAL, "immediate exit, RTS not initialized");
	EXIT (1);
    }

    LOCK (sr_exec_up_mutex, "shutdn");
    exec_was_up = sr_exec_up;
    if (sr_exec_up) {
	sr_exec_up = FALSE;	/* prevent looping if send aborts */
	UNLOCK (sr_exec_up_mutex, "shutdn");

	if (sr_my_vm == MAIN_VM) {
	    /* tell srx that main final is done; it then tells the others */
	    sr_srx_death_ok = TRUE;
	    pkt.code = exitcode;
	    pkt.report = report_blocked;
	    sr_net_send (SRX_VM, MSG_EXIT, &pkt.ph, sizeof (pkt));
	}
    } else
	UNLOCK (sr_exec_up_mutex, "shutdn");

    if (report_blocked)
	sr_print_blocked ();

    if (sr_my_vm == MAIN_VM && exec_was_up) {
#ifdef MULTI_SR
	/* srx may not be *our* child, so wait() is unsafe */
	if (kill (sr_exec_pid, 0) == 0) {	/* if it's still there */
	    DEBUG0 (D_GENERAL, "sleeping in hope that SRX will die");
	    sleep (1);
	} else
	    DEBUG0 (D_GENERAL, "SRX is gone");
#else
	{
	int wstatus;
	DEBUG0 (D_GENERAL, "waiting for SRX to die");
	wait (&wstatus);
	}
#endif
    }
#ifdef __PARAGON__
    if (sr_my_vm == MAIN_VM) {
	int i;

	DEBUG (D_GENERAL,"sending exit code %ld to distributors",exitcode,0,0);
	/* terminate waiting distributors */
	vm_parameter.phys_machine = -1;		
	vm_parameter.virt_machine = exitcode;	
	for (i = 0; i < numnodes (); i++) {
	    csend (0, &vm_parameter, sizeof (vm_parameter), i, 0);
	}
    }
#endif
    DEBUG (D_GENERAL, "exit(%ld)", exitcode, 0, 0);
    EXIT (exitcode);
}



/*  Terminate program due to stack overflow.  */

void
sr_stk_overflow ()
{
    sr_abort ("stack overflow");
}


/*  Terminate program due to stack underflow.  */

void
sr_stk_underflow ()
{
    sr_abort ("stack underflow");
}



/*  Terminate program due to stack corruption.  */

void
sr_stk_corrupted ()
{
    sr_abort ("stack corrupted");
}
