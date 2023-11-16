/*  alloc.c -- memory management routines
 *
 *  This file deals with memory allocated by the runtime system or *implicitly*
 *  by SR programs.  Explicit SR new() and free() calls are supported in misc.c.
 */

#include "rts.h"

static Memh all_mem;	/* header blocks for SR allocated memory */
static Mutex mem_mutex;	/* protection for all_mem; acquired after res->rmutex.*/

static void do_free ();



/*
 *  Initialize memory management.
 */

void
sr_init_mem ()
{
    INIT_LOCK (mem_mutex, "mem_mutex");
    all_mem = NULL;
}



/*
 *  Allocate a chunk of contiguous memory and remember who owns it.
 *
 *  If size >= 0, the memory is associated with the current resource.
 *  If size < 0, the memory is not associated with any resource.
 *  Need_mutex should be 0 iff the caller already has res->rmutex.
 *
 *  This grabs first the res rmutex first and then mem_mutex.  It should
 *  be possible to do this serially; only the insert into all_mem needs
 *  mem_mutex, and only the insert into res->meml needs res->rmutex.
 *
 *  The only other place this nesting happens is in sr_free below, so
 *  if they are serialized here then they should be serialized there.
 *  Note that if they are nested, we have to grab rmutex first, since
 *  sometimes routines in this file are called while rmutex is already
 *  locked by the caller.
 *
 *  Note that this sometimes grabs rmutex.  All such callers (and their
 *  RTS callers) of this routine that do so do *not* ever hold any
 *  other lock while calling this, and they must not in future RTS
 *  mods to preserve the linear ordering.
 */
Ptr
sr_alc (size, need_rmutex)
int size, need_rmutex;
{
    Memh mp;
    Rinst res;

    if (size >= 0) {
	res = CUR_RES;
    } else {
	res = NULL;
	size = -size;
    }
    if (res != NULL && need_rmutex)
	LOCK (res->rmutex, "sr_alc");

    LOCK (mem_mutex, "sr_alc");
    mp = (Memh) sr_alloc (size + MEMH_SZ);
    mp->res = res;
    insert (mp, all_mem, mnext, mlast);
    UNLOCK (mem_mutex, "sr_alc");

    if (res != NULL) {
	insert (mp, res->meml, rnext, rlast);
	if (need_rmutex)
	    UNLOCK (res->rmutex, "sr_alc");
    }

    DEBUG (D_ALLOC, "sr_alc @%06lX (%ld)", mp, size, 0);

    return (Ptr) mp + MEMH_SZ;
}



/*
 *  Called by the generated code to allocate and initialize a string
 */
String *
sr_alc_string (maxlength)
int maxlength;
{
    String *s;
    int size = maxlength + STRING_OVH;

    sr_check_stk (CUR_STACK);
    s = (String *) sr_alc (size, 1);
    s->size = size;
    s->length = -1;
    return s;
}



/*
 *  malloc a chunk of memory and check for success.
 */
Ptr
sr_alloc (size)
int size;
{
    Ptr mp;

    mp = MALLOC ((unsigned int) size);
    DEBUG (D_ALLOC, "sr_alloc @%06lX (%ld)", mp, size, 0);
    if (mp == NULL)
	sr_abort ("out of memory");
#ifndef NDEBUG
    memset (mp, 0xCF, size);	/* init to unexpected value to catch errors */
#endif
    return mp;
}



/*
 *  Free memory allocated by sr_alc or sr_alc_string.
 */
void
sr_free (addr)
Ptr addr;
{
    sr_check_stk (CUR_STACK);
    do_free (addr, 1);
}



/*
 *  Free sr_alc'd memory while holding res->rmutex.
 */
void
sr_locked_free (addr)
Ptr addr;
{
    sr_check_stk (CUR_STACK);
    do_free (addr, 0);
}



/*
 *  Free a previously allocated chunk of memory.
 *  Remove record from global and resource memory lists.
 */
static void
do_free (addr, need_res_mutex)
Ptr addr;
int need_res_mutex;
{
    Memh mp;

    mp = (Memh) (addr - MEMH_SZ);

    if ((mp->res != NULL) && need_res_mutex)
	LOCK (mp->res->rmutex, "sr_free");
    LOCK (mem_mutex, "sr_free");

    delete (mp, all_mem, mnext, mlast);

    if (mp->res != NULL)
	delete (mp, mp->res->meml, rnext, rlast);

    if ((mp->res != NULL) && need_res_mutex)
	UNLOCK (mp->res->rmutex, "sr_free");
    UNLOCK (mem_mutex, "sr_free");

    DEBUG (D_ALLOC, "sr_free  @%06lX", mp, 0, 0);
    if (!DBFLAGS (D_NOFREE))		/* can be inhibited using debug flag */
	UNMALLOC ((Ptr) mp);
}



/*
 *  Free all memory belonging to the specified resource.
 *  The caller must possess res->rmutex.
 */
void
sr_res_free (res)
Rinst res;
{
    Memh mp, nxt;

    LOCK (mem_mutex, "sr_res_free");
    for (mp = res->meml; mp; mp = nxt) {
	nxt = mp->rnext;
	delete (mp, all_mem, mnext, mlast);
	DEBUG (D_ALLOC, "sr_res_free (%06lX)", mp, 0, 0);
	UNMALLOC ((Ptr) mp);
    }

    UNLOCK (mem_mutex, "sr_res_free");
}
