/*  globals.h -- global definitions  */

/*  Globals are declared here for all to use and actually defined in main.c.  */

#ifndef global
#define global extern
#endif

#ifndef initval
#define initval(v)
#endif



global int mpd_argc;				/* program argument count */
global char **mpd_argv;				/* program argument vector */

global int mpd_my_machine;			/* current machine number */
global Vcap mpd_my_vm;				/* current vm number */
global char mpd_my_label[100];			/* diagnostic label fr this vm*/

global Rcap mpd_nu_rcap;			/* null resource capability */
global Rcap mpd_no_rcap;			/* noop resource capability */
global Rcap mpd_main_res;			/* main resource capability */
global Mutex mpd_main_res_mutex;		/* protect mpd_main_res */

global Ocap mpd_nu_ocap;			/* null operation capability */
global Ocap mpd_no_ocap;			/* noop operation capability */

global Vcap mpd_nu_vmcap initval (NULL_VM);	/* null vm capability */
global Vcap mpd_no_vmcap initval (NOOP_VM);	/* noop vm capability */

global Bool mpd_exec_up;			/* is mpdx up yet? */
global int mpd_exec_pid;			/* its pid if so */
global Mutex mpd_exec_up_mutex;			/* protect these two */

global struct idle_st mpd_msg_counts;		/* for detecting termination */

global Bool mpd_mpdx_death_ok initval (FALSE);	/* ignore MPDX death? */

global Sem mpd_kill_wait;			/* to block until killed */

global int mpd_num_job_servers initval (1);	/* number of job servers */

global struct private_st mpd_private[MAX_JOBSERVERS];
					/* private data for each job server */

global int mpd_trc_flag;			/* nz if tracing operations */
global int mpd_trc_fd;				/* fd for tracing output */



/*  mpd_queue_mutex protects (for MultiMPD) the five lists that participate
 *  in the termination decision (in idle_proc in process.c), plus any
 *  queue of type queue_st or inv_queue_st. Those five lists are the globals
 *  mpd_io_list, mpd_ready_list, and mpd_idle_list, plus the lists nap_list and
 *  io_list, which are static to event.c.
 *
 *  mpd_queue_mutex must be grabbed (via LOCK_QUEUE) before any
 *  other lock, namely a semaphore lock.
 */
global Mutex mpd_queue_mutex;

/*  Process control variables.  */
global Procq mpd_ready_list;		/* list of process ready to run */
global Procq mpd_idle_list;		/* list of idle procs */
global Procq mpd_io_list;		/* procs waiting for I/O server (MMPD)*/



/*  File descriptor locks.  NEVER Grab another mutex while holding [1] or [2],
 *  or else all job servers will hang on their next output, and it will not
 *  be clear who holds what.
 */
global Mutex mpd_fd_lock[LAST_SHARED_FD+1];



/*  These variables are defined and initialized by the linker. */

extern Rpat mpd_rpatt[];		/* resource pattern table */
extern int mpd_num_rpats;		/* number of resource patterns */

extern int mpd_max_co_stmts;		/* limit on active "co" statements */
extern int mpd_max_classes;		/* limit on "in" operation classes */
extern int mpd_max_loops;		/* max loop traversals before sched. */
extern int mpd_max_operations;		/* limit on active operations */
extern int mpd_max_processes;		/* limit on number of processes */
extern int mpd_max_rmt_reqs;		/* limit on pending remote requests */
extern int mpd_max_resources;		/* limit on active resources */
extern int mpd_max_semaphores;		/* limit on number of semaphores */

extern int mpd_stack_size;		/* size of a process stack */

extern int mpd_async_flag;		/* asynchronous output? */

extern char mpd_exec_path[];		/* path to find mpdx executable */
