/*  irix.h -- MultiSR configuration for Silicon Graphics Iris running Irix  */

#define MULTI_SR

#include <stdio.h>	/* prevents warnings about ulocks function prototypes */
#include <ulocks.h>
#include <task.h>

extern usptr_t *sr_irix_locks;
extern int sr_irix_exitcode;



/*  multiprocessor malloc() and free() coordinate properly on an Iris  */
#define MALLOC(a)		(char *) malloc(a)
#define UNMALLOC(a)		free(a)

/*  lock datatype and actions  */
#define multi_mutex_t		ulock_t
#define multi_lock(a)		ussetlock(a)
#define multi_unlock(a)		usunsetlock(a)
#define multi_reset_lock(a)	usinitlock(a)
#define multi_alloc_lock(a) { \
    a = usnewlock (sr_irix_locks); \
    DEBUG (D_ALLOC, "alloc lock @%06x", a, 0, 0); \
    if (a == NULL) \
	sr_abort ("lock alloc failed"); \
}
#define multi_free_lock(a) { \
    DEBUG (D_ALLOC, "free  lock @%06x", a, 0, 0); \
    usfreelock (a, sr_irix_locks); \
}



/* Shared file objects. 
 * 
 * IRIX threads share all file descriptor tables.
 */
#undef UNSHARED_FILE_OBJS
#define SHARED_FILE_OBJS

/* Least and greatest object (by file descriptor) shared */
#define FIRST_SHARED_FD  0
#define LAST_SHARED_FD FOPEN_MAX


/*  lock configuration */
#define MAX_LOCKS 2000		/* number of locks to allow */
#define LOCK_SIZE 150		/* empirically observed under Irix 6.5 */


/*  job id stuff */
#define MAX_JOBSERVERS  10		/* practical max is 8 (1 per CPU) */
#define MY_JS_ID	get_tid()	/* from <task.h>, refs prda entry */



/*  exit(code) -- shut down all threads using "code" as the exit status
 *
 *  We've called prctl(PR_SETEXITSIG), so that every other process sees our
 *  own exit; we save the exit code here so they know what exit code to use
 *  after catching the signal.
 */
#define EXIT(code) exit (sr_irix_exitcode = code)
