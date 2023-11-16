/*  uni.h -- MultiSR configuration for a uniprocessor  */

/*  This configuration does NOT define MULTI_SR; all others do.  */



#define MALLOC(a)	malloc(a)
#define UNMALLOC(a)	free(a)

#define MAX_JOBSERVERS	1
#define MY_JS_ID	0



/*  Mutual exclusion: nothing needed in Uni-SR  */

#define multi_mutex_t char

#define multi_lock(a)
#define multi_unlock(a)
#define multi_reset_lock(a)
#define multi_alloc_lock(a)
#define multi_free_lock(a)



/*  File descriptors are neither shared nor unshared  */

#undef SHARED_FILE_OBJS
#undef UNSHARED_FILE_OBJS


/*  Least and greatest object (by file descriptor) shared.
 *  max < min to make the range empty.
 */
#define FIRST_SHARED_FD  1
#define LAST_SHARED_FD  0

#define EXIT(code) exit(code)
