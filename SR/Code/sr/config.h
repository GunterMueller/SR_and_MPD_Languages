/*  config.h -- implementation constants, limits, and default values  */


/*  File name configuration  */

#define SOURCE_SUF ".sr"	/* suffix for source file */
#define SPEC_SUF ".spec"	/* suffix for specification file name */
#define IMPL_SUF ".impl"	/* suffix for global impl file name */
#define INTER_DIR "Interfaces"	/* subdirectory for interface information */
#define RUNTIME_OBJ "srlib.a"	/* file name for runtime library */
#define RUNTIME_EXEC "srx"	/* name of runtime executive */


/*  Runtime environment variables  */
#define ENV_PARALLEL	"SR_PARALLEL"	/* degree of runtime parallelism */
#define ENV_SPIN	"SR_SPIN_COUNT"	/* idle iterations before napping */
#define ENV_NAP		"SR_NAP_INTERVAL"  /* nap time after giving up */
#define ENV_DEBUG	"SR_DEBUG"	/* flags for runtime debugging */
#define ENV_SRX		"SRXPATH"	/* where to exec srx from */
#define ENV_NETMAP	"SRMAP"		/* where to read network configuration*/
#define ENV_TRACE	"SR_TRACE"	/* flags for runtime invocation trace */
#define ENV_PATH	"SR_PATH"	/* init search path for obj & rsrcs */


/*  Absolute limits  */

#define MAX_INT   0x7FFFFFFF	/* largest positive integer */

#define MAX_FATALS	  20	/* stop at this many fatals compiling resource*/
#define MAX_ERRORS	 200	/* same for fatal+warning count */

#define MAX_RES_DEF	 200	/* maximum defined (static) resources */
#define MAX_SRC		 200	/* maximum source files incl unique .specs */

#define HOST_NAME_LEN	  64	/* max length of a host name */
#define MAX_PATH	1000	/* maximum path length */
#define MAX_LINE	1000	/* maximum line in directive file */

#define MAX_COLUMNS	  78	/* max width for srm output */
#define TAB_WIDTH	   8	/* assumed width of a \t character */

#define MAX_NESTING	  50	/* maximum block nesting */
#define MAX_DIMENS	  32	/* maximum number of array dimensions */


/*  Miscellaneous configuration issues  */

#define ALCSIZE		16360	/* allocation chunk size (compiler) */
#define ALCGRAN		    8	/* allocation granularity, must be power of 2 */
#define HTSIZE		  647	/* hash table size */
#define OBUFSIZE	 3584	/* output buffer size */

#define DEF_SPIN	   35	/* default spin count */
#define DEF_NAP		   10	/* default nap (msec) */

/*  Number of elements allocated for a runtime pool (not counting the extra
 *  overhead element).  Profiling shows that more than this doesn't help.
 */
#define ALCPOOL		   62	/* elements in a runtime allocation pool */

/* runtime stack sizes for overhead processes */
/* n.b. HP (at least) needs >2000 for sprintf use */
#define IDLE_PROC_STACKSIZE	0x4000
#define SWITCH_PROC_STACKSIZE	0x4000
#define DUMMY_PROC_STACKSIZE	0x4000



/*  Runtime limits changeable by srl option.  Additions require changes in:
 *	man/srl.1	(man page)
 *	srl/globals.h	(define variable to hold default)
 *	srl/main.c	(declare variable, accept option, usage message)
 *	srl/gen.c	(generate value for later runtime use)
 *	rts/globals.h	(definition of default value)
 */

#define MAX_ALL		1000000

#define MAX_CO_STMTS	MAX_ALL	/* C: default limit on active "co" statements */
#define MAX_CLASSES	MAX_ALL	/* N: default limit on "in" operation classes */
#define MAX_OPERATIONS	MAX_ALL	/* O: default limit on active operations */
#define MAX_PROCESSES	MAX_ALL	/* P: default limit on number of processes */
#define MAX_RMT_REQS	MAX_ALL	/* Q: default limit on pending remote requests*/
#define MAX_RESOURCES	MAX_ALL	/* R: default limit on active resources */
#define MAX_SEMAPHORES	MAX_ALL	/* V: default limit on number of semaphores */

/*  Picking STACK_SIZE is hard.  Some simple Sun programs need >32K.  */
#define STACK_SIZE	40000	/* S: default size of a process stack */
#define MAX_LOOPS	10000	/* L: default limit on loops between resched */
