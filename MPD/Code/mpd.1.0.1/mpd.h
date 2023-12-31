/*  mpd.h -- runtime constants and data structures  */
/*  include multi.h before including this file.  */

#include <stdio.h>	/* needed for defn of FILE */

#ifndef TRUE		/* some header files define these */
#define TRUE  ((Bool)1)
#define FALSE ((Bool)0)
#endif

#define ASTERISK 0x80000000	/* index value of '*' */
#define SINGLEINDEX 0x7FFFFFFF	/* no second index value (in slice expr) */

#define NULL_SEQN 0xFDC0	/* seqn of null resource or operation cap */
#define NOOP_SEQN 0xFE4F	/* seqn of noop resource or operation cap */

#define	STDIN		0	/* standard input file */
#define	STDOUT		1	/* standard output file */
#define	STDERR		2	/* standard error output file */
#define	NULL_FILE	(-1)	/* file on which all operations are error */
#define	NOOP_FILE	(-2)	/* file on which all operations are no-op */
#define NOOP_VM		254	/* special code for NOOP vmcap */
#define NULL_VM		255	/* special code for NULL vmcap */

#define GRAN 8			/* alignment granularity */

#define MPDALIGN(x) (((x)+GRAN-1) & ~(GRAN-1))



/*
 *   For things that are struct {a,b,...,x[*]}, x is not declared.
 *
 *   Thus sizeof(structname) gives the size excluding the array x
 *   and DATA(&s) gives the address of the beginning of x.
 *   For arrays, where one or more Dim (dimension) structs follow
 *   the header, ADATA() must be used instead of DATA().
 *
 *   We assume that the size field is always first in all descriptors.
 */

#define DATA(p) ((char *) (p) + sizeof (*(p)))



typedef unsigned char	Bool;
typedef unsigned char	Char;
typedef int		Int;
typedef int		Enum;
typedef double		Real;
typedef char *		Ptr;
typedef FILE *		File;

typedef void (*Func) ();



typedef struct {    /* Array header */
    int size;		/* total allocated size */
    int ndim;		/* number of dimensions */
    int offset;		/* offset to data (total header length) */
    int pad;		/* pad for double-word alignment */
    /* Dim d[*]; */	/* dimension structs follow */
} Array;

typedef struct {    /* Array dimension */
    int lb;		/* lower bound */
    int ub;		/* upper bound */
    int stride;		/* size of element of this dimension */
    int pad;		/* pad for double-word alignment */
} Dim;

#define ADIM(a,d)	(((Dim*)((a)+1))[d])
#define LB(a,d)		(ADIM(a,d).lb)
#define UB(a,d)		(ADIM(a,d).ub)
#define STRIDE(a,d)	(ADIM(a,d).stride)
#define ADATA(a)	((Ptr)(a)+(a)->offset)

typedef struct {    /* String header */
    int size;		/* implies maxlength = size - STRING_OVH */
    int length;		/* current assigned length */
    /* char c[*]; */
} String;

#define STRING(x,n,s) static struct{int z,l;char c[n+1];}x={n+STRING_OVH,n,s}
#define STRING_OVH (sizeof(String) + 1)		/* +1 is for room for \0 */
#define MAXLENGTH(s) ((s)->size-STRING_OVH)

typedef struct {	/* common front part of all descriptor-based values */
    int size;		/* total allocated size */
} Descr;

#define DSIZE(p) (((Descr*)(p))->size)



typedef unsigned short Vcap;	/* virtual machine capability */

typedef struct {		/* operation capability */
    Vcap vm;			/* virtual machine number */
    unsigned short seqn;	/* sequence number */
    Ptr oper_entry;		/* pointer to operation entry */
} Ocap;

typedef struct {		/* resource capability */
    Vcap vm;			/* virtual machine number */
    unsigned short seqn;	/* sequence number */
    struct rin_st *res;		/* resource instance */
    /* Ocap o[*] */		/* opcaps & arrays of opcaps follow */
} Rcap;

typedef struct {	/* resource pattern; keep in sync with mpdl/gen.c */
    char *name;
    Func initial;
    Func final;
} Rpat;



typedef struct {		/* mutual exclusion lock for MultiMPD */
    multi_mutex_t lock;		/* actual software or hardware lock */
    char *name;			/* name for debugging */
} Mutex;



#define	RTS_CALL 0		/* this invocation came via the RTS */

enum in_type { CALL_IN, SEND_IN, COCALL_IN, COSEND_IN, REM_COCALL_IN };
enum op_type { PROC_OP, PROC_SEP_OP, INPUT_OP, DYNAMIC_OP, SEMA_OP, END_OP };
enum pr_type { FREE_PR, INITIAL_PR, FINAL_PR, PROC_PR, IDLE_PR };

typedef struct cob_st  *Cob;	/* co block */
typedef struct invb_st *Invb;	/* invocation block */
typedef struct proc_st *Proc;	/* process descriptor */
typedef struct rin_st *Rinst;	/* resource instance */
typedef struct sem_st *Sem;	/* semaphore descriptor */
typedef struct procq_st Procq;	/* process queue */
typedef struct pach_st *Pach;	/* packet header */
typedef struct remd_st *Remd;	/* remote request message descr */



/* RTS packet types -- also change rts/netw.h (PROTO_VER), rts/debug.c */

enum ms_type {
    MSG_BAD,			/* error */
    MSG_NONE,			/* no packet available */
    MSG_SEOF,			/* EOF on socket */
    MSG_HELLO,			/* VM signon */
    MSG_IDLE,			/* VM is idle */
    MSG_STOP,			/* user has called stop */
    MSG_QUIT,			/* MPDX finds all VMs idle, tells main VM */
    MSG_EXIT,			/* request for a VM to shut down */
    MSG_RCVCALL,		/* acks RECEIVE, also acts as REQ_INVOKE */
    REQ_LOCVM,  ACK_LOCVM,	/* specify location for VM */
    REQ_CREVM,  ACK_CREVM,	/* create VM */
    REQ_FINDVM, ACK_FINDVM,	/* find socket of VM */
    REQ_CREATE, ACK_CREATE,	/* create on remote VM */
    REQ_DESTROY,ACK_DESTROY,	/* destroy on remote VM */ 
    REQ_COUNT,  ACK_COUNT,	/* query invocation count on remote VM */
    REQ_INVOKE, ACK_INVOKE,	/* invoke on remote VM */
    REQ_RECEIVE,ACK_RECEIVE,	/* receive from remote VM */
    REQ_DESTOP, ACK_DESTOP,	/* destroy a remote op */
    REQ_DESTVM, ACK_DESTVM,	/* destroy a VM itself */
    REQ_CALLME, ACK_CALLME	/* ask another VM to initiate contact */
};

struct	pach_st	{	/* packet header */
    Vcap	origin;		/* originating machine */
    Vcap	dest;		/* destination machine */
    int		size;		/* size (in bytes) of the entire packet */
    int		priority;	/* priority of sending process */
    enum	ms_type	type;	/* message type */
    Remd	rem;		/* pending request message descriptor */
};



typedef struct crb_st {	/* resource creation request block */
    struct	pach_st	ph;	/* packet header */
    short	rpatid;		/* resource pattern table index */
    Vcap	vm;		/* virtual machine on which to create */
    Rcap	*rcp;		/* ptr to memory area for capability */
    int		rc_size;	/* size of capability area */
    int		crb_size;	/* size of entire CRB (header + arg area) */
} CRB;

struct	cii_st	{	/* co invocation information */
    Cob		cobp;		/* ptr to co block */
    Invb	next;		/* list of completed invocations */
    short	arm_num;	/* arm number in co stmt */
    unsigned short  seqn;	/* co block sequence number */
};

struct	invb_st	{	/* invocation block */
    struct	pach_st	ph;	/* packet header */
    Ocap	opc;		/* operation capability */
    enum	in_type	type;	/* invocation type */
    Bool	replied;	/* true iff early reply done */
    Bool	forwarded;	/* this is a forwarded block */
    Bool	discard;	/* unneeded by caller; free when done */
    struct	cii_st	co;	/* information for co stmts */
    Sem		wait;		/* sem for waiting on call/input to return */
    Invb	*ibpret;	/* location for returning final ibp address */
    Invb	next;		/* list of pending invocations */
    Invb	last;		/* back pointer */
    struct	proc_st *invoker; /* invoker id */
    struct	proc_st *forwarder; /* forwarder id */
};



struct proc_st {	/* process descriptor */

    enum pr_type ptype;		/* type of process */
    enum in_type itype;		/* type of invocation if a PROC */
    char *pname;		/* proc name, or "body" or "final" */
    char *locn;			/* current source location (when blocking) */
    int priority;		/* process priority */
    Sem wait;			/* sem for initial completion */

    Ptr   stack;		/* process context and stack */
    Mutex stack_mutex;		/* is the stack free yet? */

    /* status and blocked_on are protected by mpd_queue_mutex */
    int status;			/* process status */
    Procq *blocked_on;		/* pointer to list process is blocked on.
				 * only used by awaken and mpd_kill, when the
				 * JS has a handle on the proc (and thus no
				 * other JS can), so it does not need to be
				 * protected.
				 */

    Bool should_die;		/* is someone trying to kill this? */
    Sem waiting_killer;		/* if so, is blocked on this */

    Rinst res;			/* resource process belongs to.  This res has
				 * a mutex, which is grabbed before we use this
				 * pointer to change any of the resource's
				 * fields.
				 */
    Proc procs;			/* list of processes for resource.  res->mutex
				 * is locked when this is manipulated. */
    Cob cob_list;		/* list of co stmt blocks.  Only used with
				 * cur_proc, so no mutex is needed. */
    Invb next_inv;		/* next pending invocation to examine.  This is
				 * only used with cur_proc or through a proc
				 * field in a locked class, so it doesn't need
				 * to be protected here. */
    Bool else_leg;		/* in in statement with else leg */

    /* The next and next_else lists are not protected, since a proc can only
     * be on one (protected) list at a time and only one job server could
     * have grabbed it from a queue at any time.
     */
    Proc next;			/*  ready, blocked, or free list */
    Proc next_else;		/*  next process with else option */
};



struct private_st {	/* private variables for each job server */
    int rem_loops;		/* remaining loop traversals before resched */
    Rinst cur_res;		/* current resource */
    Proc cur_proc;		/* current process */
    Proc old_proc;		/* previous current process */
    char *switch_stack;		/* for swapping stack locks */

    int my_jobserver_id;	/* jobserver ("OS" thread) ID */
    int io_handoff;		/* flag for switching to I/O server */

    /*
     * js_queue_depth tracks the number of grab_queue_mutex calls
     * that have not been matched by the returning give_queue_mutex
     * call.  Thus, if the depth is 0 on entry grab_queue_mutex actually
     * grabs the mutex, and then it increments js_queue_depth.  If
     * this is 1 upon entry give_queue_mutex releases the mutex.  This
     * is necessary because the access to these queues happen at all
     * levels, and to add parameters telling if we possess a certain
     * queue would be both ugly and error prone.  (Note that being
     * private means that each job server has a copy.)
     */
    int js_queue_depth;
};



/* runtime globals and routines referenced by compiled code */

#ifdef RUNTIME

extern int mpd_my_machine;
extern int mpd_trc_flag;
extern Vcap mpd_no_vmcap, mpd_nu_vmcap, mpd_my_vm;
extern Ocap mpd_no_ocap, mpd_nu_ocap;
extern Rcap mpd_no_rcap, mpd_nu_rcap;
extern struct private_st mpd_private[];

extern double fabs(), ceil(), floor(), fmod();
extern double sqrt(), exp(), log(), log10(), pow();
extern double sin(), cos(), tan(), asin(), acos(), atan(), atan2();

#ifdef __alpha
/* defn required here because of 64-bit pointers */
/* could use on other systems too if we knew whether char* or void* */
extern void *memcpy();
#endif

/* always use new-style prototypes for varargs functions */
extern Ptr	mpd_cat (String *slist, ...);
extern int	mpd_imax (int n, ...);
extern int	mpd_imin (int n, ...);
extern Array *	mpd_init_array (char *locn, Array *addr,
			int elemsize, Ptr initvalue, int ndim, ...);
extern void	mpd_printf (char *locn, File fp, String *sp, String *str,
			Ptr argt, ...);
extern int	mpd_read (char *locn, File fp, char *argt, ...);
extern Real	mpd_rmax (int n, ...);
extern Real	mpd_rmin (int n, ...);
extern int	mpd_runerr (char *locn, int errnum, ...);
extern int	mpd_scanf (char *locn, FILE *fp, String *sarg, String *sfmt,
			Ptr argt, ...);
extern Ptr	mpd_slice (char *locn, Array *a1, Array *a2,
			int elemsize, int nbounds, ...);

extern void	mpd_P();
extern void	mpd_V();
extern Array *	mpd_acopy();
extern int	mpd_age();
extern Ptr	mpd_alc();
extern String *	mpd_alc_string();
extern Ptr	mpd_alloc_rv();
extern int	mpd_arg_bool();
extern int	mpd_arg_carray();
extern int	mpd_arg_char();
extern int	mpd_arg_int();
extern int	mpd_arg_ptr();
extern int	mpd_arg_real();
extern int	mpd_arg_string();
extern Ptr	mpd_astring();
extern Array *	mpd_aswap();
extern int	mpd_boolval();
extern Bool	mpd_cap_ck();
extern Array *	mpd_chars();
extern int	mpd_charval();
extern Array *	mpd_chgarr();
extern String *	mpd_chgstr();
extern Ptr	mpd_chk_myinv();
extern Ptr	mpd_clone();
extern void	mpd_close();
extern void	mpd_co_end();
extern void	mpd_co_start();
extern Ptr	mpd_co_wait();
extern void	mpd_create_global();
extern Ptr	mpd_create_resource();
extern Vcap	mpd_crevm();
extern Vcap	mpd_crevm_name();
extern void	mpd_destroy();
extern void	mpd_destvm();
extern void	mpd_dest_array();
extern void	mpd_dest_op();
extern void	mpd_dispose();
extern void	mpd_finished_final();
extern void	mpd_finished_init();
extern void	mpd_finished_input();
extern void	mpd_finished_proc();
extern void	mpd_flush();
extern Ptr	mpd_fmt_arr();
extern Ptr	mpd_fmt_bool();
extern Ptr	mpd_fmt_char();
extern Ptr	mpd_fmt_int();
extern Ptr	mpd_fmt_ptr();
extern Ptr	mpd_fmt_real();
extern Ptr	mpd_forward();
extern void	mpd_free();
extern Ptr	mpd_get_anyinv();
extern int	mpd_get_carray();
extern int	mpd_get_string();
extern Ptr	mpd_gswap();
extern void	mpd_iaccess();
extern int	mpd_imod();
extern void	mpd_init_arraysem ();
extern void	mpd_init_semop();
extern int	mpd_intval();
extern Ptr	mpd_invoke();
extern int	mpd_itoi();
extern void	mpd_kill_inops();
extern Ptr	mpd_literal_rcap();
extern void	mpd_locate();
extern void	mpd_loop_resched();
extern Ptr	mpd_make_class();
extern Ptr	mpd_make_inops();
extern Ptr	mpd_make_arraysem ();
extern void	mpd_make_proc();
extern Ptr	mpd_make_semop();
extern int	mpd_mypri();
extern void	mpd_nap();
extern Ptr	mpd_new();
extern Ocap	mpd_new_op();
extern int	mpd_numargs();
extern File	mpd_open();
extern Ptr	mpd_ptrval();
extern int	mpd_query_iop();
extern Real	mpd_random();
extern Real	mpd_realval();
extern Ptr	mpd_receive();
extern Bool	mpd_remove();
extern Invb	mpd_reply();
extern void	mpd_rm_iop();
extern Real	mpd_rmod();
extern Real	mpd_round();
extern Real	mpd_rtoi();
extern Real	mpd_rtor();
extern void	mpd_seed();
extern int	mpd_seek();
extern void	mpd_setpri();
extern Ptr	mpd_sslice();
extern String *	mpd_sswap();
extern void	mpd_stop();
extern Array *	mpd_strarr();
extern int	mpd_strcmp();
extern int	mpd_trace();
extern int	mpd_where();

#endif /* RUNTIME */
