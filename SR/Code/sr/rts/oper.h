/*  oper.h -- data structures for operation routines  */



/*  operation table entry  */

struct oper_st {
    Rinst res;		/* resource operation belongs to */
    unsigned short seqn; /* operation sequence number */
    short pending;	/* num pending input invocations for ? op */
    enum op_type type;	/* operation type */
    union {
	Func code;	/* addr of code for operation */
	Class clap;	/* input operation class */
	Sem sema;	/* optimization for semaphore operations */
    } u;
    Oper next;		/* list of operations for resource */
    Mutex omutex;	/* lock for MultiSR */
};



/*  co statement descriptor block  */

struct cob_st {
    unsigned short seqn; /* co sequence number */
    short pending;	/* num of pending invocations */
    Mutex cobmutex;	/* co block mutual exclusion */
    Sem done;		/* for sr_co_wait to block on */
    Invb done_list;	/* list of completed invocations */
    Cob next;		/* list of co for process */
};



/* invocation queue structure.  protected by sr_queue_mutex. */

struct inv_queue_st {
    Invb head;		/* head of invocation list */
    Invb tail;		/* tail of invocation list */
};

typedef struct inv_queue_st Invq;



/* operation class structure */

struct class_st {
    Mutex clmutex;	/* exclusion lock. ?inuse should go, but later... */
    Bool inuse;		/* exclusion indicator */
    short numops;	/* num of operations in this class */
    Invq old_in;	/* list of available invocations */
    Invq new_in;	/* arrivals while inuse == TRUE */
    Procq old_pr;	/* processes waiting for invocations */
    Procq new_pr;	/* processes eligible to gain access */
    Proc else_pr;	/* first process with `else' eligible to gain access */
    Proc else_tailpr;	/* last process with `else' eligible to gain access */
    Class next;		/* free list link */
};
