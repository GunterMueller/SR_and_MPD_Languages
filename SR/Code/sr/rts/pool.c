/*  pool.c -- runtime memory pools
 *
 *  These functions implement an allocation pool.
 *
 *  - Pool sr_makepool (name, el_size, el_max, init, re_init)
 *      initializes a pool of descriptors.  The name is used only to print
 *      error messages.  Each descriptor is of size el_size;  a maximum of
 *      el_max descriptors are allowed.
 *
 *      The functions "init" and "re_init" are supplied to initialize
 *      descriptors.  The "init" function is called only the first time
 *      a descriptor is returned; it should not expect any fields to have
 *      predictable values.  It must use INIT_LOCK to allocate every 
 *      field of type Mutex in the descriptor type.  The function "re_init"
 *      is called every time a descriptor is returned, thus it can assume 
 *      that values are the same as when the descriptor was released.  It 
 *      must use RESET_LOCK to initialize every field of type Mutex
 *	in the descriptor type.
 *
 *	Either "init" or "re_init" may be NULL if no such function is needed.
 *
 *  - Ptr sr_addpool (Pool p) allocates a new descriptor from pool p and
 *      returns a pointer to it.
 *
 *  - void sr_delpool (Pool p, Ptr el) returns el to the pool.
 *
 *  - void sr_eachpool (Pool p, Func f) invokes the function *f on
 *      each currently allocated descriptor in pool p.  The argument
 *      to function f is a pointer to a descriptor.  *f must NOT
 *      allocate or deallocate items from p.
 *
 *  - void sr_killpool (Pool p) deallocates all storage associated with pool p.
 *
 *  As far as users of a pool are concerned, type Pool is a magic cookie;
 *  there are no useful external operations on it.
 *
 *  Internally, pool allocates two extra pointers for every element.  The
 *  pointers are hidden; they are actually stored just before the address
 *  returned by sr_addpool (). They are used to keep a doubly linked list of
 *  allocated elements, and one of them is used for linking the free list.
 *
 *  These pool functions access no mutexes except the pool mutexes.
 *  However, the function passed to sr_eachpool can potentially access
 *  a mutex.
 */

#include "rts.h"

/*
 *  Links for pool elements
 */
typedef struct linkage {
    struct linkage *next;	/* next element in list */
    struct linkage *prev;	/* previous element in list */
} PoolLinks;

/*
 * Representation of a Pool.
 */
struct pool_str {
    char *name;			/* name of this pool */
    char *lockname;		/* name of this pool's lock (for debugging) */
    PoolLinks *free;		/* free list */
    PoolLinks *inuse;		/* in-use list */
    PoolLinks *blk;		/* next block allocated */
    int el_size;		/* element size */
    int el_max;			/* maximum number of elements allowed */
    int el_cur;			/* current number of elements allocated */
    Func init;			/* function to call on new elements */
    Func re_init;		/* function to call on used elements */
    Mutex pmutex;		/* protect table access.  Access to fields
				 * of individual element are protected by
				 * its own lock if necessary. */
};

/*
 * translate between pointers the user sees and pointers to PoolLinks
 */
#define user_addr(el) ((Ptr)(el) + sizeof (PoolLinks))
#define pool_addr(el) ((PoolLinks *)((Ptr)(el) - sizeof (PoolLinks)))



/*
 * initialize a pool
 */
Pool
sr_makepool (name, el_size, el_max, init, re_init)
char *name;			/* name of pool */
int el_size;			/* size of elements */
int el_max;			/* max number of elements */
Func init;			/* function to initialize new elements */
Func re_init;			/* function to re-initialize elements */
{
    Pool pl;

    pl = (Pool) sr_alloc (sizeof (struct pool_str));
    pl->name = name;			/* name */
    pl->lockname = (char *) sr_alloc (strlen (name) + 12);
    sprintf (pl->lockname, "%s->pmutex", pl->name);
    pl->free = 0;			/* free list */
    pl->inuse = 0;			/* in-use list */
    pl->blk = 0;			/* allocated block */
    pl->el_size = el_size + sizeof (PoolLinks);  /* element size */
    pl->el_max = el_max;		/* max number of elements */
    pl->el_cur = 0;			/* current # of elements allocated */
    pl->init = init;
    pl->re_init = re_init;
    INIT_LOCK (pl->pmutex, pl->lockname);
    return pl;
}


/*
 * allocate an element from pool p
 */
Ptr
sr_addpool (pl)
Pool pl;
{
    PoolLinks *res, *el;
    int i;

    LOCK (pl->pmutex, "sr_addpool");

    if (pl->free != 0) {
	res = pl->free;
	pl->free = res->next;
    } else if (pl->el_cur < pl->el_max) {
	PoolLinks *blk;
	int how_many;
	
	/* allocate a block of elements */
	if (pl->el_cur + ALCPOOL > pl->el_max)
	    how_many = pl->el_max - pl->el_cur;
	else
	    how_many = ALCPOOL;
	blk = (PoolLinks *) sr_alloc ((how_many + 1) * pl->el_size);

	/* link blk into block list */
	/* first element in blk is reserved for use as a block link */
	blk->next = pl->blk;
	pl->blk = blk;
	
	/* call init on each element and link into the free list */
	/* (Caution: slightly funky pointer arithmetic) */
	el = pl->free = (PoolLinks *) ((char *) blk + pl->el_size);
	for (i = 0; i < how_many - 1; i += 1) {
	    PoolLinks *next_el;

	    if (pl->init)		/* initialize this element */
		(*pl->init) (user_addr (el));

	    /* link it into the free list */
	    next_el = (PoolLinks *) ((char *) el + pl->el_size);
	    el->next = next_el;
	    el = next_el;
	}
	if (pl->init)			/* initialize the last element */
	    (*pl->init) (user_addr (el));
	el->next = 0;
	pl->el_cur += how_many;
	
	/* finally, get res from free list */
	res = pl->free;
	pl->free = res->next;
    } else {
	char buf[100];
	sprintf (buf, "too many %s", pl->name);
	sr_abort (buf);
    }

    /* link res into inuse chain */
    res->prev = 0;
    res->next = pl->inuse;
    if (pl->inuse != 0)
	pl->inuse->prev = res;
    pl->inuse = res;
    if (pl->re_init)
	(*pl->re_init) (user_addr (res));

    DEBUG (D_ALLOC, "addpool  @%06lX %s", user_addr (res), pl->name, 0);
    UNLOCK (pl->pmutex, "sr_addpool");
    return user_addr (res);
}


/*
 *  put el back in pool pl.
 */
void
sr_delpool (pl, el_ptr)
Pool pl;
Ptr el_ptr;
{
    PoolLinks * el;

    LOCK (pl->pmutex, "sr_delpool");

    el = pool_addr (el_ptr);
    DEBUG (D_ALLOC, "delpool  @%06lX %s", el_ptr, pl->name, 0);

    /* extract el from the inuse list */
    if (el->prev == 0) {	/* first element */
	pl->inuse = el->next;
	if (pl->inuse)
	    pl->inuse->prev = 0;
    } else if (el->next == 0) {	/* last element, but not only one */
	el->prev->next = 0;
    } else {			/* in the middle */
	el->prev->next = el->next;
	el->next->prev = el->prev;
    }

    /* put element on the free list, which is singly linked */
    if (!DBFLAGS (D_NOFREE)) {		/* can be inhibited by debug flag */
	el->next = pl->free;
	pl->free = el;
    }

    UNLOCK (pl->pmutex, "sr_delpool");
}


/*
 * invoke f once for each element in pl->inuse.
 * f must NOT alter the inuse list.  And nothing
 * that f calls may cause pmutex to be grabbed
 * again (this problem goes away with a generalized
 * locking facility that allows nesting, as in
 * the special case of sr_queue_mutex).
 */
void
sr_eachpool (pl, f)
Pool pl;
Func f;
{
    PoolLinks *el;

    LOCK (pl->pmutex, "sr_eachpool");

    for (el = pl->inuse; el != 0; el = el->next)
	(*f) (user_addr (el));

    UNLOCK (pl->pmutex, "sr_eachpool");
}


#ifdef NEVER	/* can re-enable if this is ever needed */
/*
 * deallocate all storage associated with a pool
 */
void
sr_killpool (p)
Pool p;
{
    PoolLinks *el;

    for (el = p->blk; el != 0;) {
	PoolLinks *next_blk = el->next;
	UNMALLOC ((Ptr) el);
	/*  we really could use a un_init function here to deallocate
	 *  anything that the init function allocated. */
	el = next_blk;
    }
    FREE_LOCK (pl->pmutex, pl->lockname, 0);
    UNMALLOC ((Ptr) p->lockname);
    UNMALLOC ((Ptr) p);
}
#endif
