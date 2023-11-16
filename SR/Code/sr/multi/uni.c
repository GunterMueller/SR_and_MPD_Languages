/*  multiprocessing routines included by ../rts/process.c  */
/*  uni.c -- uniprocessor stub versions  */



/*
 *  Create n jobservers.  Should not be called here.
 */
void
sr_create_jobservers (code, n)
void (*code) ();
int n;
{
    /*  since we're without multi_SR, lets warn if the user
     *  might expect parallelism. */
    if (n > 1) {
	char buf[100];
	sprintf (buf,
	    "multiSR not configured; SR_PARALLEL (%u) ignored", n);
	RTS_WARN (buf);
    }
    (*code) ();
    fprintf (stderr, "Error: sr_create_jobservers code returned\n");
    exit (1);
}


/*
 *  This will be the first thing that code above calls.
 *  It can be used for sanity checks on the threads package.
 */
/*ARGSUSED*/ /*(under MultiSR, that is)*/
void
sr_jobserver_first (arg)
void *arg;
{
}


/*
 *  initialize anything in this file.  sr_init_multiSR() is
 *  guaranteed to be called before any other function in this file.
 */
void
sr_init_multiSR ()
{
}
