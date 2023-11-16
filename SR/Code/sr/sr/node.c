/*  node.c -- node creation & destruction  */

#include "compiler.h"



/*  leftchild[] -- the left child variant associated with each Node operator  */

Child leftchild [] = {
#define OPR(name,left) left,
#include "operators.h"
};



/*  newnode (opr) -- return a new node with the given code  */

Nodeptr
newnode (opr)
Operator opr;
{
    Nodeptr e = NEW (Node);
    e->e_opr = opr;
    e->e_locn = srclocn;
    return e;
}



/*  unode (opr, l) -- return new unary node with given opr and child  */

Nodeptr
unode (opr, l)
Operator opr;
Nodeptr l;
{
    Nodeptr e = newnode (opr);
    if (l) {
	e->e_l = l;
	e->e_locn = l->e_locn;
    }
    return e;
}



/*  bnode (opr, l, r) -- return new binary node with given opr and children  */

Nodeptr
bnode (opr, l, r)
Operator opr;
Nodeptr l, r;
{
    Nodeptr e = newnode (opr);

    /* do r first, then l, to end up with l's locn if there is one */
    if (r) {
	e->e_r = r;
	e->e_locn = r->e_locn;
    }
    if (l) {
	e->e_l = l;
	e->e_locn = l->e_locn;
    }
    return e;
}



/*  blist (opr, l, r) -- make binary list.
 *
 *  Change each element of list l into a binary node with the given operator,
 *  retaining the old value as the left child and making r the right child.
 */
Nodeptr
blist (o, l, r)
Operator o;
Nodeptr l, r;
{
    Nodeptr e;

    for (e = l; e != NULL; e = e->e_r) {
	ASSERT (e->e_opr == O_LIST);
	e->e_l = bnode (o, e->e_l, r);
	e->e_l->e_locn = e->e_l->e_l->e_locn;
    }
    return l;
}



/*  lcat (list1, list2) -- append list2 (or node2) to list1, returning list1  */

Nodeptr
lcat (list1, list2)
Nodeptr list1, list2;
{
    Nodeptr e;

    if (list2 == NULL)
	return list1;
    if (list2->e_opr != O_LIST)
	list2 = unode (O_LIST, list2);
    if (list1 == NULL)
	return list2;

    for (e = list1; e->e_r != NULL; e = e->e_r)
	;

    ASSERT (e->e_opr == O_LIST);
    e->e_r = list2;
    return list1;
}



/*  indx (l, r) -- build INDEX tree of expr l subscripted by list r  */

Nodeptr
indx (l, r)
Nodeptr l, r;
{
    Nodeptr a;

    for (a = r; a != NULL; a = a->e_r)
	if (a->e_l->e_l != NULL || a->e_l->e_r->e_opr == O_ASTER)
	    break;

    if (a == NULL) { 
	/* no dimension is sliced */
	while (r != NULL) {
	    l = bnode (O_INDEX, l, r->e_l->e_r);
	    r = r->e_r;
	}
    } else {
	/* there's a slice involved */
	l = bnode (O_SLICE, l, r);
	while (r != NULL) {
	    r->e_l->e_opr = O_RANGE;
	    if (r->e_l->e_l == NULL && r->e_l->e_r->e_opr == O_ASTER)
		r->e_l->e_l = newnode (O_ASTER);
	    r = r->e_r;
	}
    }
    return l;
}



/*  mkarray (l, r) -- mutate list l (or NULL) and node r into O_ARRAY tree */

Nodeptr
mkarray (l, r)
Nodeptr l, r;
{
    Nodeptr e;

    if (l == NULL)
	return r;
    e = l;
    while (l->e_r != NULL) {
	l->e_opr = O_ARRAY;
	l = l->e_r;
	}
    l->e_opr = O_ARRAY;
    l->e_r = r;
    return e;
}



/*  intnode (n) -- return new O_ILIT node with given int value  */

Nodeptr
intnode (n)
int n;
{
    Nodeptr e = newnode (O_ILIT);
    e->e_sig = int_sig;
    e->e_int = n;
    return e;
}



/*  realnode (v) -- return new O_RLIT node  */

Nodeptr
realnode (v)
Real v;
{
    Nodeptr e = newnode (O_RLIT);
    e->e_sig = real_sig;
    *(e->e_rptr = NEW (Real)) = v;
    return e;
}



/*  idnode (name) -- return new O_ID node with given identifier value  */

Nodeptr
idnode (name)
char *name;
{
    Nodeptr e = newnode (O_ID);
    e->e_name = unique (name);
    return e;
}



/*  refnode (tmpname, g) -- return node referencing memory via indirect ptr */

Nodeptr
refnode (tmpname, g)
char *tmpname;
Sigptr g;
{
    char ref[30];

    if (DESCRIBED (g))
	sprintf (ref, "((%s)%s)", csig (g), tmpname);
    else
	sprintf (ref, "(*(%s*)%s)", csig (g), tmpname);
    return vbmnode (ref, g);
}



/*  vbmnode (s, g) -- return O_VERBATIM node for text s with signature g  */

Nodeptr
vbmnode (s, g)
char *s;
Sigptr g;
{
    Nodeptr e = newnode (O_VERBATIM);
    e->e_name = rsalloc (s);
    e->e_sig = g;
    return e;
}



/*  parmnode (pbcast, pbaddr, n, g) -- return VERBATIM node ref to a param
 *
 *  pbaddr is the name of a temp holding the parameter block address.
 *  pbcast is the corresponding typedef iff pbaddr is declared as Ptr.
 *  n selects the nth parameter.
 *  g is the parameter's signature.
 */

Nodeptr
parmnode (pbcast, pbaddr, n, g)
char *pbcast, *pbaddr;
int n;
Sigptr g;
{
    char buf[100];

    if (DESCRIBED (g)) {
	if (pbcast)
	    sprintf (buf, "((%s)(%s+((%s*)%s)->o_%d))",
		csig (g), pbaddr, pbcast, pbaddr, n);
	else
	    sprintf (buf, "((%s)((Ptr)%s+%s->o_%d))", 
		csig (g), pbaddr, pbaddr, n);
    } else {
	if (pbcast)
	    sprintf (buf, "((%s*)%s)->_%d", pbcast, pbaddr, n);
	else
	    sprintf (buf, "%s->_%d", pbaddr, n);
    }
    return vbmnode (buf, g);
}



/*  nreplace (e, cast, tname) -- replace node e with reference to temp tname
 *
 *  e->e_r will be a copy of the old node *e.
 */

Nodeptr
nreplace (e, cast, tname)
Nodeptr e;
char *cast, *tname;
{
    char buf[100];
    Nodeptr a;

    if (cast) {
	sprintf (buf, "((%s)%s)", cast, tname);
	tname = buf;
    }
    a = vbmnode (tname, e->e_sig);
    a->e_locn = e->e_locn;
    a->e_r = NEW (Node);
    *a->e_r = *e;
    *e = *a;
    return e;
}
