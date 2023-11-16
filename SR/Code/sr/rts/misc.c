/*  misc.c -- miscellaneous routines supporting the generated code  */

#include <stdarg.h>
#include "rts.h"



/*
 *  sr_cat (string1, string2, ..., NULL)
 *  Allocate a buffer and concatenate one or more strings.
 *  Return the address of the new string.
 *
 *  Args that are properly aligned are string addresses.
 *  Improper addresses contain a char shifted left two bits.
 */
Ptr
sr_cat (String *slist, ...)
{
    va_list ap;
    String *s, *t;
    int n;
    char *p;

    n = 0;					/* total the string lengths */
    va_start (ap, slist);
    for (s = slist; s != NULL; s = va_arg (ap, String *)) 
	if ((int) s & 1)
	    n++;				/* char argument */
	else
	    n += s->length;			/* String argument */
    va_end (ap);

    t = (String *) sr_alc (n + STRING_OVH, 1);
    t->size = n + STRING_OVH;
    t->length = n;

    p = DATA (t);
    va_start (ap, slist);
    for (s = slist; s != NULL; s = va_arg (ap, String *))
	if ((int) s & 1) {			/* for each input string: */
	    *p++ = (int) s >> 2;		    /* copy in char argument */
	} else {
	    memcpy (p, DATA (s), s->length);	    /* copy into new string */
	    p += s->length;			    /* advance pointer*/
	}
    va_end (ap);
    return (Ptr) t;				/* return result address */
}



/*
 *  Compare two strings and return <0, 0, or >0.
 */
int
sr_strcmp (l, r)
String *l, *r;
{
    unsigned char *laddr, *raddr;
    int d, llen, rlen;

    laddr = (unsigned char *) DATA (l);
    raddr = (unsigned char *) DATA (r);
    llen = l->length;
    rlen = r->length;
    while (rlen--)  {
	if (!llen--)
	    return -1;
	if ((d = *laddr++ - *raddr++) != 0)
	    return d;
    }
    return llen;
}



/*
 *  Swap two strings and return the left side (containing former R side value).
 */
String *
sr_sswap (locn, ls, rs)
char *locn;
String *ls, *rs;
{
    register char t, *l, *r;
    register int n;

    if (rs->length > MAXLENGTH (ls))		/* check lengths */
	sr_runerr (locn, E_SSIZ, rs, ls);
    if (ls->length > MAXLENGTH (rs))
	sr_runerr (locn, E_SSIZ, ls, rs);
    n = ls->length;				/* swap lengths */
    ls->length = rs->length;
    rs->length = n;
    if (ls->length > n)
	n = ls->length;				/* get swap count */
    l = DATA (ls);
    r = DATA (rs);
    while (n-- > 0)  {				/* swap them */
	t = *l;
	*l++ = *r;
	*r++ = t;
    }
    return ls;
}



/*
 *  Swap two objects and return the left side (containing former R side value).
 */
Ptr
sr_gswap (l, r, nbytes)
Ptr l, r;
int nbytes;
{
    register char c;
    Ptr orgl;

    orgl = l;
    while (--nbytes >= 0) {
	c = *l;
	*l++ = *r;
	*r++ = c;
    }
    return orgl;
}



/*
 *  Allocate memory for an SR new (type) call.
 */

#define ALLOC_MAGIC 314159265
static unsigned long low_alloc = (unsigned long) ~0L;
static unsigned long high_alloc = 0L;
static Mutex alloc_mutex;			/* protects above two */

void
sr_init_misc ()
{
    INIT_LOCK (alloc_mutex, "alloc_mutex");
}


Ptr
sr_new (locn, len)
char *locn;
int len;
{
    Ptr addr;
    unsigned long uaddr;

    if (len < 0)
	sr_loc_abort (locn, "negative argument to sr_new ()");
    addr = (Ptr) MALLOC ((unsigned int) (len + SRALIGN (sizeof (long))));
    DEBUG (D_NEW, "sr_new (%ld) -> %06lX", len, addr, 0);
    if (!addr)
	sr_loc_abort (locn, "out of memory");
    * (unsigned long *) addr = ALLOC_MAGIC;
    addr += SRALIGN (sizeof (long));
    LOCK (alloc_mutex, "sr_new");
    uaddr = (unsigned long) addr;
    if (uaddr < low_alloc)
	low_alloc = uaddr;
    if (uaddr > high_alloc)
	high_alloc = uaddr;
    UNLOCK (alloc_mutex, "sr_new");
    return addr;
}



/*
 *  Deallocate a block allocated by sr_new; noop if null address.
 */

void
sr_dispose (locn, addr)
char *locn;
Ptr addr;
{
    unsigned long uaddr;

    if (addr)  {
	uaddr = (unsigned long) addr;
	addr -= SRALIGN (sizeof (long));
	DEBUG (D_NEW, "sr_dispose (%06lX)", addr, 0, 0);
	LOCK (alloc_mutex, "sr_dispose");
	if (uaddr < low_alloc || uaddr > high_alloc
	|| (* (unsigned long *) addr != ALLOC_MAGIC))
	    sr_loc_abort(locn,"bad address for free(), or block overwritten");
	* (unsigned long *) addr = 0;	/* prevent double free */
	UNMALLOC (addr);
	UNLOCK (alloc_mutex, "sr_dispose");
    }
}



/*
 *  Return the number of command line arguments.
 */
int
sr_numargs ()
{
    sr_check_stk (CUR_STACK);
    return (sr_argc > 0) ? sr_argc - 1 : 0;
}



/*
 *  Interpret command line argument "n" as a Boolean literal.
 */
int
sr_arg_bool (n, p)
int n;
Bool *p;
{
    sr_check_stk (CUR_STACK);
    if (n < 0 || n >= sr_argc)
	return EOF;
    return sr_cvbool (sr_argv[n], p);
}



/*
 *  Interpret command line argument "n" as an integer.
 */
int
sr_arg_int (n, p)
int n;
Int *p;
{
    sr_check_stk (CUR_STACK);
    if (n < 0 || n >= sr_argc)
	return EOF;
    return sr_cvint (sr_argv[n], p);
}



/*
 *  Interpret command line argument "n" as a real.
 */
int
sr_arg_real (n, p)
int n;
Real *p;
{
    char c[2];
    int r;
    double t;

    sr_check_stk (CUR_STACK);
    if (n < 0 || n >= sr_argc)
	return EOF;

    c[0] = '\0';
    r = sscanf (sr_argv[n], "%lf%1s", &t, c);
    if (r != 1 || c[0] != '\0')
	return 0;
    *p = t;
    return 1;
}



/*
 *  Copy the "n"th command line argument to SR char array.
 */
int
sr_arg_carray (n, a)
int n;
Array *a;
{
    int len, count;
    Ptr p, ap;

    sr_check_stk (CUR_STACK);
    if (n < 0 || n >= sr_argc)
	return EOF;

    p = ADATA (a);
    len = UB (a, 0) - LB (a, 0) + 1;
    count = 0;
    for (ap = sr_argv [n]; *ap != '\0' && len > 0; len--, count++)
	*p++ = *ap++;

    return count;
}

/*
 * Interpret command line argument n as a pointer.
 */
int
sr_arg_ptr (n, p)
int n;
Ptr *p;
{
    int r;
    char c[2];
    long t;

    sr_check_stk (CUR_STACK);
    if (n < 0 || n >= sr_argc)
	return EOF;

    if (strcmp (sr_argv[n], "==null==") == 0) {
	*p = NULL;
	return 1;
    }

    c[0] = '\0';
    r = sscanf (sr_argv[n], "%8lx%1s", &t, c);
    if (r != 1 || c[0] != '\0')
	return 0;
    *p = (Ptr) t;
    return 1;
}



/*
 * Interpret command line argument n as a char.
 */
int
sr_arg_char (n, p)
int n;
Char *p;
{
    char c[2];

    sr_check_stk (CUR_STACK);
    if (n < 0 || n >= sr_argc)
	return EOF;

    c[0] = '\0';
    sscanf (sr_argv[n], "%1s", c);
    *p = c[0];
    return 1;
}


/*
 *  Copy the "n"th command line argument to SR string.
 */
int
sr_arg_string (n, s)
int n;
String *s;
{
    int len, count;
    Ptr p, ap;

    sr_check_stk (CUR_STACK);
    if (n < 0 || n >= sr_argc)
	return EOF;

    p = DATA (s);
    len = MAXLENGTH (s);
    count = 0;
    for (ap = sr_argv [n]; *ap != '\0' && len > 0; len--, count++)
	*p++ = *ap++;
    s->length = count;

    return count;
}
