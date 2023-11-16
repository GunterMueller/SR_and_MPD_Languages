/*  semaphore.c -- semaphore support  */

#include "rts.h"

static void init_sem (), re_init_sem ();

static Pool sem_pool;			/* pool of semaphores */



/*
 *  Create a semaphore.
 */
Sem
mpd_make_sem (init_val)
int init_val;
{
    Sem sp;

    /* allocate a descriptor and initialize */
    sp = (Sem) mpd_addpool (sem_pool);
    sp->value = init_val;

    /*  nobody can have a handle on a freed lock.  If they did,
     *  then locking sp->smutex wouldn't do much good, since they could
     *  use sp after we've reset its fields (and let go of smutex). */

    if (sp->blocked.head != NULL)
	mpd_malf ("found blocked process on new semaphore");

    sp->blocked.tail = NULL;

    DEBUG (D_MKSEM, "r%06lX new_sem  s%06lX (%s)", CUR_RES,sp,CUR_PROC->pname);
    return sp;
}



/*
 *  Kill a semaphore.
 */
void
mpd_kill_sem (sp)
Sem sp;
{
    Proc pr;
    char buf[100];

    DEBUG (D_KLSEM, "r%06lX kill_sem s%06lX", CUR_RES, sp, 0);
    if (sp == NULL)
	mpd_malf ("mpd_kill_sem (NULL)");

    LOCK_QUEUE ("mpd_kill_sem");
    LOCK (sp->smutex, "mpd_kill_sem");
    for (pr = sp->blocked.head; pr != NULL; pr = pr->next)
	if (pr->pname[0] != '[') {
	    sprintf (buf, "process blocked on killed sem: %s", pr->pname);
	    RTS_WARN (buf);
	    DEBUG (D_KLSEM, buf, 0, 0, 0);
	}
    UNLOCK_QUEUE ("mpd_kill_sem");
    UNLOCK (sp->smutex, "mpd_kill_sem");

    mpd_delpool (sem_pool, (Ptr) sp);
}



/*
 *  mpd_P and mpd_V -- semaphore operations called from generated code.
 */
void
mpd_V (locn, sp)
char *locn;
Sem sp;
{
    TRACE ("V", locn, sp);
    V (sp);
}

void
mpd_P (locn, sp)
char *locn;
Sem sp;
{
    TRACE ("P", locn, sp);
    P (locn, sp);
    TRACE ("CONTP", locn, sp);
}



/*
 *  P and V -- interal calls from within the runtime system.
 *  The stack is not checked for these calls, and no trace is generated.
 */
void
V (sp)
Sem sp;
{
    DEBUG (D_V, "r%06lX V        s%06lX", CUR_RES, sp, 0);
    LOCK_QUEUE ("V");			/* must grab before sem smutex */
    LOCK (sp->smutex, "V");

    if (! sp->blocked.head)
	sp->value++;
    else
	awaken (sp->blocked);
    UNLOCK_QUEUE ("V");
    UNLOCK (sp->smutex, "V");
}

void
P (locn, sp)
char *locn;
Sem sp;
{
    if (locn)
	CUR_PROC->locn = locn;		/* remember source location */
    DEBUG (D_P, "r%06lX P        s%06lX", CUR_RES, sp, 0);
    LOCK_QUEUE ("P");			/* must grab before smutex */
    LOCK (sp->smutex, "P");

    if (sp->value) {
	sp->value--;
	UNLOCK_QUEUE ("P");
	UNLOCK (sp->smutex, "P");
    } else {
	block (& sp->blocked);
	UNLOCK (sp->smutex, "P");
	mpd_scheduler ();		/* scheduler releases queue_mutex */
    }
}



void
mpd_init_sem ()
{
    sem_pool = mpd_makepool ("semaphores", sizeof (struct sem_st),
	mpd_max_semaphores, init_sem, re_init_sem);
}



static void
init_sem (s)
Sem s;
{
    s->blocked.head = NULL;
    s->blocked.tail = NULL;
    INIT_LOCK (s->smutex, "s->smutex");
}



/*ARGSUSED*/ /*(under MultiMPD they are)*/
static void
re_init_sem (s)
Sem s;
{
    RESET_LOCK (s->smutex);
}


/*
 *  mpd_init_arraysem (locn, dest, src, ndim) - initialize array of semaphores.
 *
 *  dest is the address of an array of sems.
 *  src  is the address of an array of integers.
 *  ndim is the number of dimensions.
 *
 *  Code is similar to (a merge of) mpd_init_semop and semarray.
 */
void
mpd_init_arraysem (locn, dest, src, ndim)
char *locn;
Ptr dest, src;
int ndim;
{
    int n;
    Array *dstp, *srcp; /* parameters just cast. */
    Sem *d;
    Int *s;
    int i;

    if (ndim <= 0)
	mpd_loc_abort (locn, "cannot mpd_init_semop an array with <=0 dims");

    dstp = (Array *) dest;
    srcp = (Array *) src;

    d = (Sem *) ADATA (dstp);
    s = (Int *) ADATA (srcp);

    for (i = 0; i < dstp->ndim; i++)
	if (UB (dstp, i) - LB (dstp, i) != UB (srcp, i) - LB (srcp, i))
	    mpd_runerr (locn, E_ASIZ, ((Dim*)(d+1))[i], ((Dim*)(s+1))[i]);

    for (i = mpd_acount (dstp); i--; d++, s++)
	(*d)->value = *s;	/* initialize a single element (semaphore) */
}
