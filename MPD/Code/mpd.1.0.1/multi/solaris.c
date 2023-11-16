/*  multiprocessing routines included by ../rts/process.c  */
/*  solaris.c -- for the Sun Solaris 2.x OS  */

/*  The Solaris thread package is described in the "SunOS 5.2 Guide to
 *  Multi-Thread Programming" and in chapter 4 of the "SunOS 5.2 System
 *  Services" manual.
 */


int mpd_js_id[1000];	/* maps Solaris thread ID to MPD process index */



/*
 *  create n copies of the thread, each starting at code(arg).
 */
void
mpd_create_jobservers (code, n)
void (*code)(void *);
int n;
{
    int i;

    /*  One job server (the caller) is already active; create n-1 more.  */
    for (i = 1; i < n; i++)  {
	if (thr_create (0, STACK_SIZE,
		(void * (*) (void *)) code, (void *) i, THR_NEW_LWP, 0) != 0)
	    mpd_abort ("Could not create thread");
    }

    /* now caller becomes job server 0. */
    (*code) ((void *) 0);

    mpd_malf ("mpd_create_jobservers code_caller() returned");
    /*NOTREACHED*/
}


/*
 *  This will be the first thing that code above calls.
 */
void
mpd_jobserver_first (arg)
void *arg;
{
    int n;
    n = (int) thr_self();
    if (n < 0 || n >= (sizeof(mpd_js_id) / sizeof(mpd_js_id[0])))
	mpd_malf ("thread id out of range");
    mpd_js_id[n] = (int) arg;
}



/*
 *  initialize anything in this file.  mpd_init_multiMPD() is
 *  guaranteed to be called before any other function in this file.
 */
void
mpd_init_multiMPD ()
{
    /* nothing to do.  */
}
