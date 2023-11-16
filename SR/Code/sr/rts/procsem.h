/*  procsem.h -- data structures for processes and semaphores  */



/*
 *  The order of these status codes is important.  In sr_cswitch,  action
 *  is taken in two cases, one of the status is <BLOCKED and the other if
 *  the status is <DOING_IO.  Also, see debug.c if any status is added.
 */
#define ACTIVE		2
#define READY		3
#define DOING_IO	4	/* btwn BEGIN_IO & END_IO, run by IO Server */
#define BLOCKED		5
#define INFANT		6
#define FREE		7
#define BLOCKED_DOING_IO	8	/* blocked from state DOING_IO */
#define	TO_BE_FREED	9	/* marked to be freed */

#define MAX_STATUS	9	/* maximum status value */



/*    process queues    */

struct procq_st {
    Proc head;		/* pointer to head of process list */
    Proc tail;		/* pointer to tail of process list */
};



/*  semaphore descriptor  */
/*  IMPORTANT:  generated code assumes that "value" is first  */

struct sem_st {
    Int value;		/* sem value; MUST be first struct member */
    Procq blocked;
    Mutex smutex;	/*  protects value, next, blocked (the queue is also
			 *  protected by sr_queue_mutex, but we need to do
			 *  all changes to sem_st fields in one atomic
			 *  action). */
    };



#ifdef UNSHARED_FILE_OBJS
#define BLOCK_STATUS(s) (s == DOING_IO ? BLOCKED_DOING_IO : BLOCKED);
#else
#define BLOCK_STATUS(s) BLOCKED
#endif


/*  Block the current process and place it on the  specified waiting list.  */

#define block(plist) { \
    int new_status; \
    LOCK_QUEUE ("block"); \
    new_status = BLOCK_STATUS (CUR_PROC->status);\
\
    CUR_PROC->blocked_on = (plist); /* don't lock; plist won't change */\
    CUR_PROC->status = new_status;  /* after changing blocked_on */ \
    CUR_PROC->next = NULL; \
    sr_enqueue ((plist), CUR_PROC); \
    UNLOCK_QUEUE ("block"); \
\
    DEBUG (D_BLOCK, "r%06lX *%06lX block   p%06lX", \
	CUR_RES, plist, CUR_PROC); \
}


/*
 *  Awaken the next process on the specified list.  If the process is
 *  in state BLOCKED_DOING_IO (only valid in UNSHARED_FILE_OBJS), then we stick
 *  him on the sr_io_list, otherwise he goes on sr_ready_list.
 */

#ifdef UNSHARED_FILE_OBJS
#define AWAKEN_MY_QUEUE_ADDR(s) \
    (s == BLOCKED_DOING_IO ? &sr_io_list : &sr_ready_list)
#define UNBLOCK_STATUS(s)  (s == BLOCKED_DOING_IO ? DOING_IO : READY);
#define CHECK_STATUS if(old_status==DOING_IO)sr_malf ("awaken:bad old_status")
#else
#define UNBLOCK_STATUS(s) READY
#define AWAKEN_MY_QUEUE_ADDR(s) &sr_ready_list
#define CHECK_STATUS 0
#endif

#define awaken(plist) { \
    Proc pr; \
    int old_status, new_status; \
\
    LOCK_QUEUE ("awaken"); \
    pr = (plist).head; \
    (plist).head = pr->next; \
    if (pr->next == NULL) \
	(plist).tail = NULL;\
    old_status = pr->status; \
    new_status = UNBLOCK_STATUS (old_status); \
    pr->status = new_status; \
    pr->next = NULL; \
\
    sr_enqueue (AWAKEN_MY_QUEUE_ADDR (old_status), pr); \
    UNLOCK_QUEUE ("awaken"); \
\
    DEBUG5 (D_ACTIVATE, "r%06lX awaken   p%06lX prio %d from %06lX", \
	CUR_RES, pr, pr->priority, & (plist), 0); \
    CHECK_STATUS; \
}
