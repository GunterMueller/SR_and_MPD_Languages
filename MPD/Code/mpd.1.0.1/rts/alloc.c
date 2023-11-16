/*  alloc.c -- memory management routines
 *
 *  This file deals with memory allocated by the runtime system or *implicitly*
 *  by MPD programs.  Explicit MPD new() and free() calls are supported in
 *  misc.c.
 */

#include "rts.h"

static Memh all_mem;	/* header blocks for MPD allocated memory */
static Mutex mem_mutex;	/* protection for all_mem; acquired after res->rmutex.*/

static void do_free ();



/*
 *  Initialize memory management.
 */

void
mpd_init_mem ()
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
 *  The only other place this nesting happens is in mpd_free below, so
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
mpd_alc (size, need_rmutex)
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
	LOCK (res->rmutex, "mpd_alc");

    LOCK (mem_mutex, "mpd_alc");
    mp = (Memh) mpd_alloc (size + MEMH_SZ);
    mp->res = res;
    insert (mp, all_mem, mnext, mlast);
    UNLOCK (mem_mutex, "mpd_alc");

    if (res != NULL) {
	insert (mp, res->meml, rnext, rlast);
	if (need_rmutex)
	    UNLOCK (res->rmutex, "mpd_alc");
    }

    DEBUG (D_ALLOC, "mpd_alc @%06lX (%ld)", mp, size, 0);

    return (Ptr) mp + MEMH_SZ;
}



/*
 *  Called by the generated code to allocate and initialize a string
 */
String *
mpd_alc_string (maxlength)
int maxlength;
{
    String *s;
    int size = maxlength + STRING_OVH;

    mpd_check_stk (CUR_STACK);
    s = (String *) mpd_alc (size, 1);
    s->size = size;
    s->length = -1;
    return s;
}



/*
 *  malloc a chunk of memory and check for success.
 */
Ptr
mpd_alloc (size)
int size;
{
    Ptr mp;

    mp = MALLOC ((unsigned int) size);
    DEBUG (D_ALLOC, "mpd_alloc @%06lX (%ld)", mp, size, 0);
    if (mp == NULL)
	mpd_abort ("out of memory");
#ifndef NDEBUG
    memset (mp, 0xCF, size);	/* init to unexpected value to catch errors */
#endif
    return mp;
}



/*
 *  Free memory allocated by mpd_alc or mpd_alc_string.
 */
void
mpd_free (addr)
Ptr addr;
{
    mpd_check_stk (CUR_STACK);
    do_free (addr, 1);
}



/*
 *  Free mpd_alc'd memory while holding res->rmutex.
 */
void
mpd_locked_free (addr)
Ptr addr;
{
    mpd_check_stk (CUR_STACK);
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
	LOCK (mp->res->rmutex, "mpd_free");
    LOCK (mem_mutex, "mpd_free");

    delete (mp, all_mem, mnext, mlast);

    if (mp->res != NULL)
	delete (mp, mp->res->meml, rnext, rlast);

    if ((mp->res != NULL) && need_res_mutex)
	UNLOCK (mp->res->rmutex, "mpd_free");
    UNLOCK (mem_mutex, "mpd_free");

    DEBUG (D_ALLOC, "mpd_free  @%06lX", mp, 0, 0);
    if (!DBFLAGS (D_NOFREE))		/* can be inhibited using debug flag */
	UNMALLOC ((Ptr) mp);
}



/*
 *  Free all memory belonging to the specified resource.
 *  The caller must possess res->rmutex.
 */
void
mpd_res_free (res)
Rinst res;
{
    Memh mp, nxt;

    LOCK (mem_mutex, "mpd_res_free");
    for (mp = res->meml; mp; mp = nxt) {
	nxt = mp->rnext;
	delete (mp, all_mem, mnext, mlast);
	DEBUG (D_ALLOC, "mpd_res_free (%06lX)", mp, 0, 0);
	UNMALLOC ((Ptr) mp);
    }

    UNLOCK (mem_mutex, "mpd_res_free");
}
