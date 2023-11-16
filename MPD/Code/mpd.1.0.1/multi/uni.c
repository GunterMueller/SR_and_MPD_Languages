/*  multiprocessing routines included by ../rts/process.c  */
/*  uni.c -- uniprocessor stub versions  */



/*
 *  Create n jobservers.  Should not be called here.
 */
void
mpd_create_jobservers (code, n)
void (*code) ();
int n;
{
    /*  since we're without multi_MPD, lets warn if the user
     *  might expect parallelism. */
    if (n > 1) {
	char buf[100];
	sprintf (buf,
	    "multiMPD not configured; MPD_PARALLEL (%u) ignored", n);
	RTS_WARN (buf);
    }
    (*code) ();
    fprintf (stderr, "Error: mpd_create_jobservers code returned\n");
    exit (1);
}


/*
 *  This will be the first thing that code above calls.
 *  It can be used for sanity checks on the threads package.
 */
/*ARGSUSED*/ /*(under MultiMPD, that is)*/
void
mpd_jobserver_first (arg)
void *arg;
{
}


/*
 *  initialize anything in this file.  mpd_init_multiMPD() is
 *  guaranteed to be called before any other function in this file.
 */
void
mpd_init_multiMPD ()
{
}
