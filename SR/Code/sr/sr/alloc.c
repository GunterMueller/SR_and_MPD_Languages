/*  alloc.c -- memory allocation code.
 *
 *  Memory is initialized at the beginning of execution and at the end of
 *  each resource (spec, body, or combined).  No attempt is made to reclaim
 *  space at any other time.
 */

#include "compiler.h"



typedef struct hunk {		/* layout of a hunk of allocated memory */
    int used;			/* number of bytes in use */
    struct hunk *prev;		/* pointer to previous hunk */
    char data [ALCSIZE];	/* data bytes */
} Hunk;

static Hunk *cur;		/* current hunk */



/*  initmem () -- [re]initialize memory  */

void
initmem ()
{
    Hunk *p;

    while (cur) {		/* return any memory in current use */
	p = cur->prev;
	free ((char *) cur);
	cur = p;
    }
    nwarnings = nfatals = 0;	/* initialize error counts */
    initnames ();		/* initialize names */
    initsig ();			/* initialize signatures */
    initsym ();			/* initialize symbol table */
    initprotos ();		/* initialize prototype list */
    initops ();			/* initialize op and class lists */
}



/*  ralloc(n) -- allocate n zero bytes with resource lifetime  */

char *
ralloc (n)
int n;
{
    char *p;
    Hunk *h;

    n = (n + ALCGRAN - 1) & ~(ALCGRAN - 1);	/* round up size */
    ASSERT (n <= ALCSIZE);

    if (!cur || (cur->used + n > ALCSIZE)) {	/* if need new hunk of mem */
	h = (Hunk *) alloc (sizeof (Hunk));
	h->used = 0;
	h->prev = cur;
	memset (h->data, 0, ALCSIZE);		/* zero the memory */
	cur = h;
    }

    p = cur->data + cur->used;
    cur->used += n;
    return p;
}



/*  rsalloc(s) -- allocate string with resource lifetime.
 *  If a null pointer is passed, a null pointer is returned.
 */

char *
rsalloc (s)
char *s;
{
    if (s)
	return strcpy (ralloc (strlen (s) + 1), s);
    else
	return NULL;
}
