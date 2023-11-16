/*  multiprocessing routines included by ../rts/process.c  */
/*  solaris.c -- for the Sun Solaris 2.x OS  */

/*  The Solaris thread package is described in the "SunOS 5.2 Guide to
 *  Multi-Thread Programming" and in chapter 4 of the "SunOS 5.2 System
 *  Services" manual.
 */


int sr_js_id[1000];	/* maps Solaris thread ID to SR process index */



/*
 *  create n copies of the thread, each starting at code(arg).
 */
void
sr_create_jobservers (code, n)
void (*code)(void *);
int n;
{
    int i;

    /*  One job server (the caller) is already active; create n-1 more.  */
    for (i = 1; i < n; i++)  {
	if (thr_create (0, STACK_SIZE,
		(void * (*) (void *)) code, (void *) i, THR_NEW_LWP, 0) != 0)
	    sr_abort ("Could not create thread");
    }

    /* now caller becomes job server 0. */
    (*code) ((void *) 0);

    sr_malf ("sr_create_jobservers code_caller() returned");
    /*NOTREACHED*/
}


/*
 *  This will be the first thing that code above calls.
 */
void
sr_jobserver_first (arg)
void *arg;
{
    int n;
    n = (int) thr_self();
    if (n < 0 || n >= (sizeof(sr_js_id) / sizeof(sr_js_id[0])))
	sr_malf ("thread id out of range");
    sr_js_id[n] = (int) arg;
}



/*
 *  initialize anything in this file.  sr_init_multiSR() is
 *  guaranteed to be called before any other function in this file.
 */
void
sr_init_multiSR ()
{
    /* nothing to do.  */
}
