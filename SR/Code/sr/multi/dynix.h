/*  dynix.h -- MultiSR configuration for a Sequent Symmetry running Dynix  */

#define MULTI_SR



/* system definitions  */

/* <stdio.h> must precede <parallel/parallel.h> to avoid conflicts */
#include <stdio.h>
#include <parallel/microtask.h>
#include <parallel/parallel.h>

extern char *shmalloc ();
extern void shfree ();

#define MALLOC(a)   shmalloc(a)
#define UNMALLOC(a) shfree(a)

#define MULTI_CC_OPT "-Y"	/* pass -Y to cc for generated code */



/*  Mutual exclusion  */

#define multi_mutex_t slock_t

#define multi_lock(a)		S_LOCK(&(a))
#define multi_unlock(a)		S_UNLOCK(&(a))
#define multi_reset_lock(a)	S_INIT_LOCK(&(a))

/* locks are not dynamic in DYNIX---they're just chars---so
 * there is nothing to allocate and free. */
#define multi_alloc_lock(a)	0
#define multi_free_lock(a)	0



/*  Shared file objects.
 *
 *  On the Sequent we pretend that we can share 1 and 2 (stdin/stdout)
 *  even though the buffers are private to each process.  We can get away
 *  with this because we flush the buffers after each write.
 */

#undef SHARED_FILE_OBJS		/* not shared in general */

#define UNSHARED_FILE_OBJS
#define FIRST_SHARED_FD  1
#define LAST_SHARED_FD  2



/*  job id stuff */

#define MAX_JOBSERVERS		MAXPROCS
#define MY_JS_ID		m_myid

/* shut down all threads and give caller $status of code */
#define EXIT(code) exit(code)
