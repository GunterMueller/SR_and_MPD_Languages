/*  names.c -- maintain tables of names and other strings  */

#include "compiler.h"
#include <ctype.h>

static struct srec {		/* hash table of known strings */
    char *text;	
    int serial;
    struct srec *next;
} *bucket[HTSIZE];

static struct srec * ulookup	PARAMS ((char *string));



/*  initnames () -- initialize string table for a new resource  */

void
initnames ()
{
    memset ((char *) bucket, 0, sizeof (bucket));	/* init hash table */
}



/*  unique(string) -- return pointer to unique copy of string
 *
 *  Every lookup of a particular string always returns the same address,
 *  allowing comparisons of pointers.
 */

char *
unique (string)
char *string;
{
    /* null pointer returns null pointer */
    if (!string)
	return NULLSTR;

    /* otherwise return string pointer from table entry */
    return ulookup (string) -> text;
}



/*  genref (name) -- generate unique handle for an identifier
 *
 *  For any particular id, successive calls to genref(id) return "1id", "2id",
 *  etc. for ensuring unique names in the generated code.
 */

char *
genref (name)
char *name;
{
    register struct srec *p;
    char buf[100];

    if (!name)
	return NULL;
    p = ulookup (name);
    sprintf (buf, "%d%s", ++p->serial, name);
    return rsalloc (buf);
}



/*  ulookup (string) -- find or create entry in table, and return pointer.  */

static struct srec *
ulookup (string)
char *string;
{
    register unsigned int h;
    register char *s;
    register struct srec *p;

    /* compute hash based on character values */
    h = 0;
    for (s = string; *s; )
	/*SUPPRESS 8*/
	h = 39 * h + *s++;
    h %= HTSIZE;

    /* search hash chain for existing string */
    for (p = bucket[h]; p; p = p->next)
	if (strcmp (string, p->text) == 0)
	    return p;

    /* need to build a new one */
    p = NEW (struct srec);
    p->text = rsalloc (string);
    p->serial = 0;
    p->next = bucket[h];
    bucket[h] = p;
    return p;
}
