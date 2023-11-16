/*  multiprocessing routines included by ../rts/process.c  */
/*  dynix.c -- Sequent Dynix OS version  */



static multi_mutex_t sr_multi_kids_mutex;	/* ensure only one aborts */


/*
 *  create n copies of the thread, each starting at the function *code.
 *  Code calls sr_jobserver_first before doing anything else.
 */
void
sr_create_jobservers (code, n)
void (*code) ();
int n;
{
    m_set_procs (n);
    m_fork (code, (void *) 0);
    fprintf (stderr, "Error: sr_create_jobservers code returned\n");
    exit (1);
}



/*
 *  This will be the first thing that code above calls.
 *  It can be used for sanity checks on the threads package.
 */
void
sr_jobserver_first (arg)
void *arg;
{
    /* Test to see that enough threads were created */
    if (sr_num_job_servers != m_numprocs) {

	/* generate only one complaint and abort */
	multi_lock (sr_multi_kids_mutex);

	sr_missing_children (sr_num_job_servers, m_numprocs);
	/*NOTREACHED*/
    }
}



/*
 *  Initialize anything in this file.  sr_init_multiSR() is
 *  guaranteed to be called before any other function in this file.
 */
void
sr_init_multiSR ()
{
    multi_reset_lock (sr_multi_kids_mutex);
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
