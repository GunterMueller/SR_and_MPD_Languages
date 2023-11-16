/*  multimac.h -- runtime macros related to multiprocessing  */



/* if MPDX is defined, disable MULTI_MPD (etc.) for building mpdx. */

#ifdef MPDX
#undef MULTI_MPD
#undef SHARED_FILE_OBJS
#undef UNSHARED_FILE_OBJS
#endif



/* access to per-jobserver private variables */

#define PRIV(var)	mpd_private[MY_JS_ID].var
#define CUR_PROC	PRIV(cur_proc)
#define CUR_RES		PRIV(cur_res)
#define CUR_STACK	(PRIV(cur_proc)->stack)



/*
 *  Mutual exclusion primitives (MultiMPD only).
 *
 *  LOCK_QUEUE and UNLOCK_QUEUE access the queue_mutex.
 *  LOCK and UNLOCK are used for most other mutexes.
 *  There are also special macros for file locking inside io.c.
 */

#ifdef MULTI_MPD		/* locks are needed under MultiMPD */

/* INIT_LOCK allocates the lock and clears it */
#define INIT_LOCK(var, label) { \
    multi_alloc_lock ((var).lock); \
    (var).name = label; \
    multi_reset_lock ((var).lock); \
}


/* RESET_LOCK clears a lock */
#define RESET_LOCK(var) { \
    DEBUG (D_MUTEX, "%-14s reset %s (&%06lX)", "", (var).name, &(var.lock)); \
    multi_reset_lock ((var).lock); \
}

#define FREE_LOCK(var,label) { \
    DEBUG (D_MUTEX, "free %s (%06lX)", label, &(var.lock), 0); \
    multi_free_lock ((var).lock); \
}

#define LOCK(var,who) { \
    DEBUG (D_MUTEX, "%-14s need %s (&%06lX)", who, (var).name, &(var.lock)); \
    multi_lock ((var).lock); \
    DEBUG (D_MUTEX, "%-14s  got %s (&%06lX)", who, (var).name, &(var.lock)); \
}

#define UNLOCK(var,who) { \
    DEBUG (D_MUTEX, "%-14s give %s (&%06lX)", who, (var).name, &(var)); \
    multi_unlock ((var).lock); \
}

/* LOCK_QUEUE only grabs the queue lock if we don't already have it */
#define LOCK_QUEUE(who) { \
    if (PRIV (js_queue_depth) == 0) { \
	DEBUG (D_QMUTEX, "%-14s need queue_mutex", who, 0, 0);  \
	multi_lock (mpd_queue_mutex.lock); \
	DEBUG (D_QMUTEX, "%-14s  got queue_mutex", who, 0, 0);  \
    } \
    PRIV (js_queue_depth)++; \
}

/* UNLOCK_QUEUE only really unlocks it when the depth hits zero */
#define UNLOCK_QUEUE(who) { \
    if (PRIV (js_queue_depth) == 1) { \
	DEBUG (D_QMUTEX, "%-14s give queue_mutex", who, 0, 0);  \
	multi_unlock (mpd_queue_mutex.lock); \
    } \
    PRIV (js_queue_depth)--; \
}


#else			/* locks are unneeded if not MULTI_MPD */

#define LOCK(var,who)
#define UNLOCK(var,who)
#define INIT_LOCK(var,label)	
#define RESET_LOCK(var)	
#define FREE_LOCK(var,label)
#define LOCK_QUEUE(who)
#define UNLOCK_QUEUE(who)

#endif  /* MULTI_MPD */



/*
 *  SHARED_FILE_OBJS and UNSHARED_FILE_OBJS tell the
 *  type of configuration that is built.  The RTS uses these in a few
 *  places.  I_AM_IOSERVER is used in a boolean expression to tell a
 *  job server whether or not it is the job server.  It should go to
 *  0 if heavyweight threads are not used.  I_CHECK_TERMINATION tells
 *  the idle procs which jobserver in MultiMPD checks for termination.
 *  MY_QUEUE_ADDR and lets a job server know what ready queue to
 *  pull jobs off of.
 *
 *  BEGIN_IO(fp)  reserves the lock for file fp, setting PRIV(io_handoff).
 *  END_IO(fp)    releases it, if PRIV(io_handoff) is set.
 *  (The file locking becomes a no-op in uni-MPD.)
 *
 *  ABORT_IO(fp,locn,msg)  is  END_IO  followed by  mpd_loc_abort(locn,msg).
 *  IO_RETURN(fp,v)        is  END_IO  followed by  return v   [where v is int].
 */


#ifdef MULTI_MPD

/* always need file locks whether shared or unshared */
#define SHARED_FD(fd) ((fd>=FIRST_SHARED_FD) && (fd<=LAST_SHARED_FD))

#ifdef SHARED_FILE_OBJS

#define NUM_IO_SERVERS		0
#define I_AM_IOSERVER		0	/* nobody is */
#define I_AM_NON_IOSERVER	0	/* nobody is */
#define BEGIN_IO(fp)		mpd_begin_io ((FILE *) fp)
#define END_IO(fp)		mpd_end_io ((FILE *) fp)

#define I_CHECK_TERMINATION	(PRIV(my_jobserver_id) == 0)
#define MY_QUEUE_ADDR		(& mpd_ready_list)

#else	/* not SHARED_FILE_OBJS */

#define NUM_IO_SERVERS		1
#define IO_SERVER_JOB		0	/* which JS is the IO server */
#define I_AM_IOSERVER		(PRIV(my_jobserver_id) == IO_SERVER_JOB)
#define I_AM_NON_IOSERVER	(PRIV(my_jobserver_id) != IO_SERVER_JOB)
#define BEGIN_IO(fp) \
	((PRIV(io_handoff)=I_AM_NON_IOSERVER)?mpd_begin_io((FILE*)fp):0)
#define END_IO(fp) (PRIV(io_handoff)?mpd_end_io((FILE*)fp):0)

#define I_CHECK_TERMINATION	I_AM_IOSERVER
#define MY_QUEUE_ADDR	(mpd_scheduler_queue_addr[PRIV(my_jobserver_id)])

#endif	/* not SHARED_FILE_OBJS */


#else	/* not MULTI_MPD */

#define SHARED_FD(fd)		0	/* always false */
#define NUM_IO_SERVERS		0
#define I_AM_IOSERVER		0	/* nobody is */
#define I_AM_NON_IOSERVER	0	/* nobody is */
#define I_CHECK_TERMINATION	1	/* everyone does (the only idle proc) */
#define MPD_NUM_IDLE_PROCS	0
#define MY_QUEUE_ADDR		(&mpd_ready_list)

#define BEGIN_IO(fp)		0
#define END_IO(fp)		0

#endif  /* MULTI_MPD */



#define ABORT_IO(fp,locn,msg) (END_IO(fp),mpd_loc_abort(locn,msg))
#define IO_RETURN(fp,v) return(END_IO(fp),(v))
