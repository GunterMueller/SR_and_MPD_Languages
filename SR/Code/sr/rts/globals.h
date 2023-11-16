/*  globals.h -- global definitions  */

/*  Globals are declared here for all to use and actually defined in main.c.  */

#ifndef global
#define global extern
#endif

#ifndef initval
#define initval(v)
#endif



global int sr_argc;				/* program argument count */
global char **sr_argv;				/* program argument vector */

global int sr_my_machine;			/* current machine number */
global Vcap sr_my_vm;				/* current vm number */
global char sr_my_label[100];			/* diagnostic label fr this vm*/

global Rcap sr_nu_rcap;				/* null resource capability */
global Rcap sr_no_rcap;				/* noop resource capability */
global Rcap sr_main_res;			/* main resource capability */
global Mutex sr_main_res_mutex;			/* protect sr_main_res */

global Ocap sr_nu_ocap;				/* null operation capability */
global Ocap sr_no_ocap;				/* noop operation capability */

global Vcap sr_nu_vmcap initval (NULL_VM);	/* null vm capability */
global Vcap sr_no_vmcap initval (NOOP_VM);	/* noop vm capability */

global Bool sr_exec_up;				/* is srx up yet? */
global int sr_exec_pid;				/* its pid if so */
global Mutex sr_exec_up_mutex;			/* protect these two */

global struct idle_st sr_msg_counts;		/* for detecting termination */

global Bool sr_srx_death_ok initval (FALSE);	/* ignore SRX death? */

global Sem sr_kill_wait;			/* to block until killed */

global int sr_num_job_servers initval (1);	/* number of job servers */

global struct private_st sr_private[MAX_JOBSERVERS];
					/* private data for each job server */

global int sr_trc_flag;				/* nz if tracing operations */
global int sr_trc_fd;				/* fd for tracing output */



/*  sr_queue_mutex protects (for MultiSR) the five lists that participate
 *  in the termination decision (in idle_proc in process.c), plus any
 *  queue of type queue_st or inv_queue_st. Those five lists are the globals
 *  sr_io_list, sr_ready_list, and sr_idle_list, plus the lists nap_list and
 *  io_list, which are static to event.c.
 *
 *  sr_queue_mutex must be grabbed (via LOCK_QUEUE) before any
 *  other lock, namely a semaphore lock.
 */
global Mutex sr_queue_mutex;

/*  Process control variables.  */
global Procq sr_ready_list;		/* list of process ready to run */
global Procq sr_idle_list;		/* list of idle procs */
global Procq sr_io_list;		/* procs waiting for I/O server (MSR) */



/*  File descriptor locks.  NEVER Grab another mutex while holding [1] or [2],
 *  or else all job servers will hang on their next output, and it will not
 *  be clear who holds what.
 */
global Mutex sr_fd_lock[LAST_SHARED_FD+1];



/*  These variables are defined and initialized by the linker. */

extern Rpat sr_rpatt[];			/* resource pattern table */
extern int sr_num_rpats;		/* number of resource patterns */

extern int sr_max_co_stmts;		/* limit on active "co" statements */
extern int sr_max_classes;		/* limit on "in" operation classes */
extern int sr_max_loops;		/* max loop traversals before sched. */
extern int sr_max_operations;		/* limit on active operations */
extern int sr_max_processes;		/* limit on number of processes */
extern int sr_max_rmt_reqs;		/* limit on pending remote requests */
extern int sr_max_resources;		/* limit on active resources */
extern int sr_max_semaphores;		/* limit on number of semaphores */

extern int sr_stack_size;		/* size of a process stack */

extern int sr_async_flag;		/* asynchronous output? */

extern char sr_exec_path[];		/* path to find srx executable */
