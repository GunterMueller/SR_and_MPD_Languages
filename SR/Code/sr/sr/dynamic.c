/*  dynamic.c -- generate code using dynamic memory allocation
 *
 *  "temps" are like registers; they're used only in the current expression
 *  and can then be reused.  They are explicitly allocated and deallocated.
 *  When deallocated, they actually remain reserved until the end of the
 *  current generated statement, in order to prevent them from being used
 *  twice in the same expression or argument list.
 *
 *  "transient" variables are Ptr variables holding addresses of temporarily
 *  allocated memory.  They are typically used to hold a temporary expression
 *  result, such as from string concatenation.  Code to call free(ptr) is
 *  automatically generated as soon as possible on every path from the
 *  current point.
 *
 *  "persistent" variables point to allocated memory that remains live until
 *  the end of the current block -- for example, a dynamically allocated array.
 *  Presistent variables are not reused.  They are any type supported by
 *  cprintf("%g").
 */


#include "compiler.h"

static int used[TCOUNT];		/* max temps used so far, by type */
static List avail[TCOUNT];		/* temps available for use */
static List busy[TCOUNT];		/* temps in use (stack discipline) */
static List freed[TCOUNT];		/* temps freed but not reassignable */
static int nfreed;			/* number freed but not reassignable */

static int tbusy;			/* valid pointers to transient memory */
static int tused;			/* number currently in use */

static int pbusy;			/* valid pointers to persistent mem */
static List pnames;			/* their names */



/*  initdynamic() -- initialize dynamic memory code  */

void
initdynamic ()
{
    int i;

    for (i = 0; i < (int) TCOUNT; i++) {
	used[i] = 0;
	avail[i] = list (sizeof (char *));
	busy[i] = list (sizeof (char *));
	freed[i] = list (sizeof (char *));
    }
    nfreed = 0;

    tbusy = 0;
    tused = 0;

    pbusy = 0;
    pnames = list (sizeof (Symptr));
}



/*******************************  temps  *******************************/



/*  alctemp(t) -- allocate temp of given type, returning name.  */

char *
alctemp (t)
Type t;
{
    int n;
    char c, **p, *s, buf[10];

    n = (int) t;

    /*
     * If there is a temp of the right type available for use,
     * just put it on the busy list and return its name.
     */
    p = (char **) lpop (avail[n]);
    if (p != NULL) {
	* (char **) lpush (busy[n]) = *p;
	return *p;
    }

    /*
     * Nope.  We must allocate a new one and put it on the busy list.
     * Generate a new name that reflects the desired type.
     */
    switch (n) {	/* keep in sync with dcltemps() */
	case T_BOOL:	c = 'b';	break;
	case T_CHAR:	c = 'h';	break;
	case T_INT:	c = 'i';	break;
	case T_ENUM:	c = 'e';	break;
	case T_REAL:	c = 'r';	break;
	case T_PTR:	c = 'p';	break;
	case T_ARRAY:	c = 'a';	break;
	case T_STRING:	c = 'z';	break;
	case T_OCAP:	c = 'o';	break;
	default:	BOOM ("bad alctemp type", typetos ((Type) n));
    }
    sprintf (buf, "%c%d", c, ++used[n]);
    s = rsalloc (buf);
    * (char **) lpush (busy[n]) = s;
    return s;			/* return pointer to name */
}



/*  freetemp(t,name) -- free temp of type t.
 *
 *  name must be the last temp of type t that is allocated but not yet freed.
 */

void
freetemp (t, name)
Type t;
char *name;
{
    int n;
    char **p;

    n = (int) t;
    p = (char **) lpop (busy[n]);
    ASSERT (p != NULL && *p == name);
    * (char **) lput (freed[n]) = *p;
    nfreed++;
}


/*  retemp() -- recycle temps to allow reuse  */

void
retemp ()
{
    int t;
    char **p;

    if (nfreed == 0)
	return;
    for (t = 0; t < (int) TCOUNT; t++) {
	while ((p = (char **) lpop (freed[t])) != NULL)
	    * (char **) lpush (avail[t]) = *p;
    }
    nfreed = 0;
}



/*  dcltemps() -- generate declarations for temporaries.  */

void
dcltemps ()
{
    char *s1, *s2;
    int t, i;

    for (t = 0; t < (int) TCOUNT; t++) {
	if (used[t] > 0) {
	    switch (t) {	/* keep in sync with alctemp() */
		case T_BOOL:	s1 = "Bool";	s2 = "b";	break;
		case T_CHAR:	s1 = "Char";	s2 = "h";	break;
		case T_INT:	s1 = "Int";	s2 = "i";	break;
		case T_ENUM:	s1 = "Int";	s2 = "e";	break;
		case T_REAL:	s1 = "Real";	s2 = "r";	break;
		case T_PTR:	s1 = "Ptr";	s2 = "p";	break;
		case T_ARRAY:	s1 = "Array";	s2 = "*a";	break;
		case T_STRING:	s1 = "String";	s2 = "*z";	break;
		case T_OCAP:	s1 = "Ocap";	s2 = "o";	break;
		default:	BOOM ("bad temp type", typetos ((Type) t));
	    }
	    cprintf ("%8%s %s1", s1, s2);
	    for (i = 2; i <= used[t]; i++)
		cprintf ("%8%,%s%d", s2, i);
	    cprintf ("%8;\n");
	}
	ASSERT (lpop (busy[t]) == NULL);
    }
}



/*******************************  transients  *******************************/



/*  alctrans() -- allocate a transient symbol, returning its name.  */

char *
alctrans ()
{
    char name[10];

    tbusy++;
    sprintf (name, "t%d", tbusy);
    if (tbusy > tused) {
	cprintf ("%8Ptr %s;\n", name);
	tused++;
    }
    return unique (name);
}



/*  ntrans() -- return current number of transistent allocations.  */

int ntrans ()
{
    return tbusy;
}



/*  freetrans(n,c) -- gen code to free transient memory to count n, sep by c.
 *
 *  If c==';', generate C statements suffixed by ";\n"
 *  If c==',', generate C expressions prefixed by "%,"
 *
 *  The transient memory, once freed, is forgotten.
 */

void
freetrans (n, c)
int n, c;
{
    ASSERT (tbusy >= n);
    while (tbusy > n) {
	if (c == ',')
	    cprintf ("%,");
	cprintf ("sr_free(t%d)", tbusy);
	if (c == ';')
	    cprintf (";\n");
	tbusy--;
    }
}



/*******************************  persistents  *******************************/



/*  notepst(s) -- push symbol on list as persistent allocated memory pointer.
 *
 *  This just registers s as containing an address of allocated memory.
 *  We assume that the caller declares and assigns it.
 *
 *  NOTE:  global and resource variables are just ignored because they can
 *  be used even after the current block exits.  They are deallocated when
 *  the resource is destroyed.
 */

void
notepst (s)
Symptr s;
{
    if (s->s_depth <= 2)		/* if top-level variable */
	return;
    * (Symptr *) lpush (pnames) = s;
    pbusy++;
}



/*  npst() -- return current number of persistent allocations.  */

int npst ()
{
    return pbusy;
}



/*  freepst(n,forget) -- gen code to free persistent memory back to point n.
 *
 *  If forget is true, the pointers are forgotten;  if not, they stay on
 *  the list (meaning they're still active in other code paths).
 *
 *  sr_kill_inops is called if the object being freed is an op or array of ops.
 */

void
freepst (n, forget)
int n, forget;
{
    int i;
    char *p;
    Symptr s;

    ASSERT (pbusy >= n);

    i = pbusy;
    FOREACH (p, pnames) {
	if (i-- <= n)
	    break;
	s = * (Symptr *) p;
	if (usig (s->s_sig) -> g_type == T_OP) {
	    cprintf ("sr_kill_inops((Ptr)");
	    if (s->s_sig->g_type != T_ARRAY)
		cprintf ("&");
	    cprintf ("%n,%d);\n", s, ndim (s->s_sig));
	} else
	    cprintf ("sr_free(%n);\n", s);
    }

    if (forget)
	for (; pbusy > n; pbusy--)
	    lpop (pnames);
}
