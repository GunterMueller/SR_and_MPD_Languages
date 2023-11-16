/*  multiprocessing routines included by ../rts/process.c  */
/*  irix.c -- for the Silicon Graphics "Irix" OS  */



#include <signal.h>

static void sigterm_handler ();

usptr_t *sr_irix_locks;			/* memory used for locks */
int sr_irix_exitcode = 0;		/* for exiting correctly */



/*
 *  create n copies of the thread, each starting at the function *code.
 *  Code calls sr_jobserver_first before doing anything else.
 */
void
sr_create_jobservers (code, n)
void (*code)();	
int n;
{
    int i;

    /* ensure we're ready to catch signals, even early ones */
    signal (SIGTERM, sigterm_handler);

    /* one job server here already, create n-1 more */
    for (i = 1; i < n; i++)
	if ((sproc (code, PR_SALL, (void *) i)) == -1) {
	    perror ("sr_create_jobservers sproc");
	    EXIT (1);
	}
    
    /* now become job server 0 */
    (*code) ((void *) 0);

    sr_malf ("sr_create_jobservers code_caller() returned");
    /*NOTREACHED*/
}



/*
 *  This will be the first thing that code above calls.
 *  It can be used for sanity checks on the threads package.
 *
 *  For Irix, the jobserver is passed (as a void *) its ID in [0..n]
 *  and saves it in process-local storage.
 */ 
void
sr_jobserver_first (arg)
void *arg;
{
    /* save process index in prda structure */
    MY_JS_ID = (int) arg;

    /* catch SIGTERM from EXIT */
    signal (SIGTERM, sigterm_handler);
}



/* 
 *  initialize anything in this file.  sr_init_multiSR() is 
 *  guaranteed to be called before any other function in this file.
 */
void
sr_init_multiSR ()
{
    static char filename[] = "/tmp/multiSR_XXXXXX";

    /* make exit() propogate signal to all threads */
    if (prctl (PR_SETEXITSIG, SIGTERM) == -1) {
	perror ("setexitsig");
	EXIT (1);
    }

    /* configure to use simple, fast locks */
    if (usconfig (CONF_LOCKTYPE, US_NODEBUG) == -1) {
	perror ("locktype");
	EXIT (1);
    }

    /* allow MAX_JOBSERVERS jobservers in a process group */
    if (usconfig (CONF_INITUSERS, MAX_JOBSERVERS) == -1) {
	perror ("initusers");
	EXIT (1);
    }

    if (usconfig(CONF_INITSIZE, MAX_LOCKS * LOCK_SIZE) == -1) {
	perror ("arenasize");
	EXIT (1);
    }

    /* initialize the lock system */
    mktemp (filename);
    sr_irix_locks = usinit (filename);
    if (sr_irix_locks == NULL)  {
	perror ("usinit");
	EXIT (1);
    }
    unlink (filename);
}



/*  Handle a SIGTERM raised by the exit of another process.
 *  Exit using the globally saved exit code.
 */
static void
sigterm_handler (sig, code, scp)
int sig, code;
struct sigcontext *scp;
{
    /* ignore further signals */
    signal (SIGTERM, SIG_IGN);

    /* send no signal on own exit */
    prctl (PR_SETEXITSIG, 0);

    DEBUG (D_GENERAL, "caught SIGTERM; _exit(%d)", sr_irix_exitcode, 0, 0);

    /* exit without flushing files -- locks may be held inside stdio */
    _exit (sr_irix_exitcode);
}
