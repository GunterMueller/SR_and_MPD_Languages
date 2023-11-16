/*  multiprocessing routines included by ../rts/process.c  */
/*  dynix.c -- Sequent Dynix OS version  */



static multi_mutex_t mpd_multi_kids_mutex;	/* ensure only one aborts */


/*
 *  create n copies of the thread, each starting at the function *code.
 *  Code calls mpd_jobserver_first before doing anything else.
 */
void
mpd_create_jobservers (code, n)
void (*code) ();
int n;
{
    m_set_procs (n);
    m_fork (code, (void *) 0);
    fprintf (stderr, "Error: mpd_create_jobservers code returned\n");
    exit (1);
}



/*
 *  This will be the first thing that code above calls.
 *  It can be used for sanity checks on the threads package.
 */
void
mpd_jobserver_first (arg)
void *arg;
{
    /* Test to see that enough threads were created */
    if (mpd_num_job_servers != m_numprocs) {

	/* generate only one complaint and abort */
	multi_lock (mpd_multi_kids_mutex);

	mpd_missing_children (mpd_num_job_servers, m_numprocs);
	/*NOTREACHED*/
    }
}



/*
 *  Initialize anything in this file.  mpd_init_multiMPD() is
 *  guaranteed to be called before any other function in this file.
 */
void
mpd_init_multiMPD ()
{
    multi_reset_lock (mpd_multi_kids_mutex);
}


/*
 *  Dynix-specific workaround:  we overload the cpus_online library routine
 *  in order to lie to m_fork about the number of CPUs available.  This is
 *  so that it doesn't refuse to create processes for us; its usual limit
 *  is one-half the actual number of CPUs.
 */
int
cpus_online ()
{
    return MAXPROCS;
}
