/*  solaris.h -- MultiSR configuration for Solaris threads.
 *
 *  Does not work with vm creation with SR_PARALLEL>1, presumably
 *  because the socket library is not multiprocessor-safe.
 *  This is expected to be fixed in Solaris 2.3.
 */

#define MULTI_SR		/* enable MultiSR */

#define _REENTRANT		/* set flag affecting stdio etc. */

#include <stdio.h>
#include <thread.h>


/*  malloc and free are multiprocessor safe on Solaris 2.2  */
#define MALLOC(a)		(char *) malloc(a)
#define UNMALLOC(a)		free(a)


/*  lock datatype and actions  */
#define multi_mutex_t		mutex_t
#define multi_lock(a)		mutex_lock(&(a))
#define multi_unlock(a)		mutex_unlock(&(a))
#define multi_reset_lock(a)	mutex_init(&(a), USYNC_THREAD, (void *) 0)

/*  We use static locks, so there's no need to allocate or free them.  */
#define multi_alloc_lock(a)	0
#define multi_free_lock(a)      0


/*  Shared file objects: Solaris  threads share all file descriptor tables.  */
#undef UNSHARED_FILE_OBJS
#define SHARED_FILE_OBJS

/*  Least and greatest object (by file descriptor) shared  */
#define FIRST_SHARED_FD  0
#define LAST_SHARED_FD 255


/*  job id stuff  */
#define MAX_JOBSERVERS  256
#define MY_JS_ID	(sr_num_job_servers==1 ? 0 : sr_js_id[(int)thr_self()])
extern int sr_num_job_servers, sr_js_id[];

/*  exit(code) -- shut down all threads using "code" as the exit status  */
#define EXIT(code) exit (code)
