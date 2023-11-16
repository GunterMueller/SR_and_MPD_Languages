/*  res.h -- data structures for resource support  */



#define INIT_REPLY  0x01  /* initial process has replied */
#define FINAL_REPLY 0x02  /* final process has replied */
#define FREE_SLOT   0x04  /* this slot is currently free */



/* resource instance descriptor */

struct rin_st {
    short rpatid;	/* resource pattern */
    unsigned short seqn; /* sequence number */
    Bool is_global;	/* is this a global or normal resource? */
    Ptr rv_base;	/* base addr of res variable area */
    CRB *crb_addr;	/* address of create request block */
    Mutex rmutex;	/* mutual exclusion for descriptor */
    Proc procs;		/* processes belonging to resource */
    Memh meml;		/* list of memory allocated to res */
    Rcap *rcp;		/* addr of res cap during create */
    int rc_size;	/* size of res cap in bytes */
    Oper oper_list;	/* list of proc operations; protected by res->mutex */
    Class class_list;   /* list of class structs; protected by res->mutex */
    int status;		/* initial/final/reply status flag */
    Rinst next;		/* free list link */
};



/* memory allocation list entry */

struct memh_st {
    Rinst res;		/* resource that owns memory block */
    Memh mnext;		/* forward ptr in global mem list */
    Memh mlast;		/* backward ptr in global mem list */
    Memh rnext;		/* forward ptr in resource mem list */
    Memh rlast;		/* backward ptr in resource mem list */
};
