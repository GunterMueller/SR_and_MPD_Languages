/*  linux.h -- MultiSR configuration for Linux-x86 using the
 *             __clone() system call.
 */

#define MULTI_SR		/* enable MultiSR */

#include <stdio.h>
#include <sched.h>
#include <signal.h>


/* use the task segment descriptor for job server identification */

#define get_tr() \
    ({ \
        unsigned short _seg__; \
        asm volatile("str %0" : "=rm" (_seg__) ); \
        _seg__; \
    })

typedef volatile int spin_lock_t;

#define SPIN_LOCK_INITIALIZER	0

#define spin_lock_init(s)	(*(s) = SPIN_LOCK_INITIALIZER)
#define spin_lock_locked(s)	(*(s) != SPIN_LOCK_INITIALIZER)

#ifdef	__GNUC__

#define	spin_unlock(p) \
	({  register int _u__ ; \
	    __asm__ volatile("xorl %0, %0; \n\
			  xchgl %0, %1" \
			: "=&r" (_u__), "=m" (*(p)) ); \
	    0; })

#define	spin_try_lock(p)\
	(!({  register int _r__; \
	    __asm__ volatile("movl $1, %0; \n\
			  xchgl %0, %1" \
			: "=&r" (_r__), "=m" (*(p)) ); \
	    _r__; }))

#define spin_lock(p) \
	({ while (!spin_try_lock(p)) while (*(p)); })

#endif	/* __GNUC__ */


void *sr_malloc_multiSR(size_t size);
void sr_free_multiSR(void *ptr);

/*  malloc and free are not multiprocessor safe in Linux  */
#define MALLOC(a)		(void *) sr_malloc_multiSR(a)
#define UNMALLOC(a)		sr_free_multiSR(a)


/*  lock datatype and actions  */
#define multi_mutex_t spin_lock_t

#define multi_lock(a)       spin_lock(&(a))
#define multi_unlock(a)     spin_unlock(&(a))
#define multi_reset_lock(a) spin_lock_init(&(a))
#define multi_alloc_lock(a) 0
#define multi_free_lock(a)  0


/*  Shared file objects: Linux threads share all file descriptor tables.  */
#undef UNSHARED_FILE_OBJS
#define SHARED_FILE_OBJS

/*  Least and greatest object (by file descriptor) shared  */
#define FIRST_SHARED_FD  0
#define LAST_SHARED_FD 255


/*  job id stuff  */
#define MAX_JOBSERVERS  256

/*#define MY_JS_ID	(sr_num_job_servers==1 ? 0 : sr_js_id[(int)getpid()])*/
#define MY_JS_ID	(sr_js_id[(int)get_tr()])

extern int sr_num_job_servers, sr_js_id[];


/*  exit(code) -- shut down all threads using "code" as the exit status  */
#define EXIT(code) sr_exit_multiSR(code)

