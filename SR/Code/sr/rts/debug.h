/*  debug.h -- debugging symbols and macros  */

extern int sr_dbg_flags;		/* currently enabled debugging values */

/*
 *  These flag values are used in the DEBUG macro.
 *  This file may be changed to reflect debugging priorities.
 *
 *  The general philosophy is that each bit should control a particular class
 *  of events, and in each class only the major events are produced (not a
 *  detailed trace).
 *
 *  Debugging is enabled by "setenv SR_DEBUG hexvalue".
 */

#define D_ALL_FLAGS	0xFFFFFFFF

/* general */
#define D_GENERAL	0x1		/* startup, shutdown, etc. */
#define D_TEMP		0x2		/* reserved for temporary uses */
#define D_PROG		0x4		/* program calls to SR_DEBUG */

/* resources */
#define D_RESOURCE	0x8		/* resource creation and destruction */

/* virtual machine control */
#define D_SRX_ACT	0x10		/* srx actions */
#define D_SRX_IN	0x20		/* srx inputs */
#define D_SOCKET	0x40		/* misc socket I/O */

/* --- beyond this point the output becomes more voluminous --- */

/* virtual machine messages */
#define D_SENT		0x100		/* packets sent */
#define D_RCVD		0x200		/* packets received */
#define D_TERM		0x400		/* distributed termination */

/* invoke */
#define D_INVOKE	0x800		/* call, send, forward, reply */

/* semaphores */
#define D_V		0x1000		/* V */
#define D_P		0x2000		/* P */
#define D_MKSEM		0x4000		/* create */
#define D_KLSEM		0x8000		/* destroy */

/* processes */
#define D_ACTIVATE	0x10000		/* activate */
#define D_BLOCK		0x20000		/* block */
#define D_SPAWN		0x40000		/* spawn, kill */
#define D_KILL		0x40000		/* spawn, kill */
#define D_RESTART	0x80000		/* restart (context switch) */

/* I/O and clock */
#define D_CLOCK		0x100000	/* clock events */
#define D_SELECT	0x200000	/* select() calls */

/* MultiSR mutexes */
#define D_QMUTEX	0x400000	/* queue mutex */
#define D_MUTEX		0x800000	/* other mutexes */

/* loops */
#define D_LOOP		0x1000000	/* loop counter expired */

/* memory allocation */
#define D_NEW		0x10000000	/* new/free calls */
#define D_ALLOC		0x20000000	/* low-level allocations */
#define D_ARRAY		0x40000000	/* array & string alloc & init */

/* control flag (this alters behavior rather than selecting tracing) */
#define D_NOFREE	0x80000000	/* inhibit freeing of memory */



/*  Print values v with format f if any debugging flags masked by n are set.  */

#ifndef NDEBUG

#define DBFLAGS(n) (sr_dbg_flags & (n))
#define DEBUG0(n,f) (DBFLAGS(n) ? sr_bugout (f, 0L, 0L, 0L, 0L, 0L) : 0)
#define DEBUG(n,f,v1,v2,v3) \
    (DBFLAGS(n) ? sr_bugout (f, (long)(v1), (long)(v2), (long)(v3), 0L, 0L) : 0)
#define DEBUG5(n,f,a,b,c,d,e) \
   (DBFLAGS(n)?sr_bugout(f,(long)(a),(long)(b),(long)(c),(long)(d),(long)(e)):0)

#else /*NDEBUG*/

#define DBFLAGS(n) 0
#define DEBUG0(n,f) 0
#define DEBUG(n,f,v1,v2,v3) 0
#define DEBUG5(n,f,v1,v2,v3,v4,v5) 0

#endif /*NDEBUG*/
