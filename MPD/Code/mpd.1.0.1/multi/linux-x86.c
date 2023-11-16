/*  multiprocessing routines included by ../rts/process.c  */
/*  linux.c -- for Linux 2.X  */


int mpd_js_id[32678];	/* maps x86 TSS register to MPD process index */

int mpd_js_clonepid[MAX_JOBSERVERS];  /* used for killing job servers  */

int mpd_js_jscount = 1;
int mpd_js_exitcode = 0;

spin_lock_t multi_memory_lock = SPIN_LOCK_INITIALIZER;


void mpd_exit_multiMPD(int status);
void *mpd_malloc_multiMPD(size_t size);
void mpd_free_multiMPD(void *ptr);




/*
 *  create n copies of the thread, each starting at code(arg).
 */
void
mpd_create_jobservers (code, n)
void (*code)(void *);
int n;
{
    int i;
    void *sbase;
    void *sptr;

    mpd_js_clonepid[0] = getpid();

    /*  One job server (the caller) is already active; create n-1 more.  */
    for (i = 1; i < n; i++)  {
        /* allocate a stack for the job server thread */
        sbase = (void *) mpd_malloc_multiMPD(STACK_SIZE);
        sptr = sbase + (STACK_SIZE - 8);
                    
        if ((mpd_js_clonepid[i] =
            __clone( (int (*) (void *)) code, sptr,
                     (CLONE_VM | CLONE_FS | CLONE_FILES
                      | CLONE_SIGHAND ), (void *) i)) == -1) {
            mpd_abort ("Could not create thread");
        }
        mpd_js_jscount++;
    }


    
    /* now caller becomes job server 0. */
    (*code) ((void *) 0);

    mpd_malf ("mpd_create_jobservers code_caller() returned");
    /*NOTREACHED*/
}



/*
 * SIGTERM hander for kernel thread termination
 */
void
mpd_handler_SIGTERM(signum)
int signum;
{
    if (MY_JS_ID == 0) {
        exit(mpd_js_exitcode);
    } else {
        _exit(mpd_js_exitcode);
    }
}



/*
 * SIGINT handler for mpdx termination
 */
void
mpd_handler_SIGINT(signum)
int signum;
{
    mpd_exit_multiMPD(0);
}



/*
 * SIGQUIT handler for mpdx termination
 */
void
mpd_handler_SIGQUIT(signum)
int signum;
{
    mpd_exit_multiMPD(0);
}   



/*
 *  this will be the first thing that code above calls.
 */
void
mpd_jobserver_first (arg)
void *arg;
{
    int n;
    n = (int) get_tr();
    if (n < 0 || n >= (sizeof(mpd_js_id) / sizeof(mpd_js_id[0])))
	mpd_malf ("thread id out of range");
    mpd_js_id[n] = (int) arg;

    signal(SIGTERM, mpd_handler_SIGTERM);
    signal(SIGINT, mpd_handler_SIGINT);
    signal(SIGQUIT, mpd_handler_SIGQUIT);
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


/*
 * exit MultiMPD.  On Linux, all job servers must be killed explicitly.
 *
 */
void
mpd_exit_multiMPD(status)
int status;
{
    int i;
    int this_pid;
    
    this_pid = getpid();

    mpd_js_exitcode = status;

    /* kill all job servers */

    for (i = 0; i < mpd_js_jscount; i++) {
        if (mpd_js_clonepid[i] != this_pid) {
            kill(mpd_js_clonepid[i], SIGTERM);
        }
    }

    if (MY_JS_ID == 0) {
        exit(status);
    } else {
        _exit(status);
    }
}



void *
mpd_malloc_multiMPD(size)
size_t size;
{
    void *rv;
        
    spin_lock(&multi_memory_lock);
    rv = (void *) malloc(size);
    spin_unlock(&multi_memory_lock);

    return rv;
}



void
mpd_free_multiMPD(ptr)
void *ptr;
{
    spin_lock(&multi_memory_lock);
    free(ptr);
    spin_unlock(&multi_memory_lock);
}
