/*  multiprocessing routines included by ../rts/process.c  */
/*  linux.c -- for Linux 2.X  */


int sr_js_id[32678];	/* maps x86 TSS register to SR process index */

int sr_js_clonepid[MAX_JOBSERVERS];  /* used for killing job servers  */

int sr_js_jscount = 1;
int sr_js_exitcode = 0;

spin_lock_t multi_memory_lock = SPIN_LOCK_INITIALIZER;


void sr_exit_multiSR(int status);
void *sr_malloc_multiSR(size_t size);
void sr_free_multiSR(void *ptr);




/*
 *  create n copies of the thread, each starting at code(arg).
 */
void
sr_create_jobservers (code, n)
void (*code)(void *);
int n;
{
    int i;
    void *sbase;
    void *sptr;

    sr_js_clonepid[0] = getpid();

    /*  One job server (the caller) is already active; create n-1 more.  */
    for (i = 1; i < n; i++)  {
        /* allocate a stack for the job server thread */
        sbase = (void *) sr_malloc_multiSR(STACK_SIZE);
        sptr = sbase + (STACK_SIZE - 8);
                    
        if ((sr_js_clonepid[i] =
            __clone( (int (*) (void *)) code, sptr,
                     (CLONE_VM | CLONE_FS | CLONE_FILES
                      | CLONE_SIGHAND ), (void *) i)) == -1) {
            sr_abort ("Could not create thread");
        }
        sr_js_jscount++;
    }


    
    /* now caller becomes job server 0. */
    (*code) ((void *) 0);

    sr_malf ("sr_create_jobservers code_caller() returned");
    /*NOTREACHED*/
}



/*
 * SIGTERM hander for kernel thread termination
 */
void
sr_handler_SIGTERM(signum)
int signum;
{
    if (MY_JS_ID == 0) {
        exit(sr_js_exitcode);
    } else {
        _exit(sr_js_exitcode);
    }
}



/*
 * SIGINT handler for srx termination
 */
void
sr_handler_SIGINT(signum)
int signum;
{
    sr_exit_multiSR(0);
}



/*
 * SIGQUIT handler for srx termination
 */
void
sr_handler_SIGQUIT(signum)
int signum;
{
    sr_exit_multiSR(0);
}   



/*
 *  this will be the first thing that code above calls.
 */
void
sr_jobserver_first (arg)
void *arg;
{
    int n;
    n = (int) get_tr();
    if (n < 0 || n >= (sizeof(sr_js_id) / sizeof(sr_js_id[0])))
	sr_malf ("thread id out of range");
    sr_js_id[n] = (int) arg;

    signal(SIGTERM, sr_handler_SIGTERM);
    signal(SIGINT, sr_handler_SIGINT);
    signal(SIGQUIT, sr_handler_SIGQUIT);
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


/*
 * exit MultiSR.  On Linux, all job servers must be killed explicitly.
 *
 */
void
sr_exit_multiSR(status)
int status;
{
    int i;
    int this_pid;
    
    this_pid = getpid();

    sr_js_exitcode = status;

    /* kill all job servers */

    for (i = 0; i < sr_js_jscount; i++) {
        if (sr_js_clonepid[i] != this_pid) {
            kill(sr_js_clonepid[i], SIGTERM);
        }
    }

    if (MY_JS_ID == 0) {
        exit(status);
    } else {
        _exit(status);
    }
}



void *
sr_malloc_multiSR(size)
size_t size;
{
    void *rv;
        
    spin_lock(&multi_memory_lock);
    rv = (void *) malloc(size);
    spin_unlock(&multi_memory_lock);

    return rv;
}



void
sr_free_multiSR(ptr)
void *ptr;
{
    spin_lock(&multi_memory_lock);
    free(ptr);
    spin_unlock(&multi_memory_lock);
}
