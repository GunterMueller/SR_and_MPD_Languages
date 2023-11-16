/*  list.c -- list creation and manipulation
 *
 *  These Lists are double-ended, doubly-linked lists (actually deques).
 *  A list contains a header, manipulated here, and an additional amount
 *  of data manipulated by the caller.  For type independence, data is
 *  referenced by a char pointer, cast as needed.
 *
 *  Operations are patterned after Icon lists:
 *
 *	list	create a new list
 *	lfirst	look at front
 *	lpush	add to front
 *	lpop	remove from front
 *	lput	add to rear
 *	ldel	delete from anywhere
 *
 *  For proper alignment, the element header size must be a multiple of ALCGRAN.
 */

#include "compiler.h"



/*  list(n) -- create a new list to hold data values of size n  */

List
list (n)
int n;
{
    List l;

    /*SUPPRESS 558*/ /*SUPPRESS 529*/   /* for Saber C: constant conditional */
    /*SUPPRESS 622*/ /*SUPPRESS 569*/	/* for CodeCenter: numbers changed */
    ASSERT (sizeof (Lelem) % ALCGRAN == 0);
    l = NEW (struct list);
    l->l_esize = sizeof (Lelem) + n;
    return l;
}



/*  lfirst(l) -- return a pointer to the first element of list l  */

char *
lfirst (l)
List l;
{
    Lelptr e;

    ASSERT (l != NULL);
    e = l->l_first;
    if (!e)
	return NULL;
    return (char *) e + sizeof (Lelem);
}



/*  lpush(l) -- push a null element on list l and return a pointer to it  */

char *
lpush (l)
List l;
{
    Lelptr e;

    ASSERT (l != NULL);
    e = (Lelptr) ralloc (l->l_esize);
    if (l->l_first)
	l->l_first->l_prev = e;
    else
	l->l_last = e;
    e->l_next = l->l_first;
    l->l_first = e;
    return (char *) e + sizeof (Lelem);
}



/*  lpop(l) -- pop the top element of list l and return a pointer to it  */

char *
lpop (l)
List l;
{
    Lelptr e;

    ASSERT (l != NULL);
    e = l->l_first;
    if (!e)
	return NULL;
    l->l_first = e->l_next;
    if (l->l_first)
	l->l_first->l_prev = NULL;
    else
	l->l_last = NULL;
    return (char *) e + sizeof (Lelem);
}



/*  lput(l) -- append a null element to list l and return a pointer to it  */

char *
lput (l)
List l;
{
    Lelptr e;

    ASSERT (l != NULL);
    e = (Lelptr) ralloc (l->l_esize);
    if (l->l_last)
	l->l_last->l_next = e;
    else
	l->l_first = e;
    e->l_prev = l->l_last;
    l->l_last = e;
    return (char *) e + sizeof (Lelem);
}



/*  ldel(l,e) -- delete element e from list l  */

char *
ldel (l, el)
List l;
char *el;
{
    Lelptr e;

    ASSERT (l != NULL && el != NULL);
    e = (Lelptr) (el - sizeof (Lelem));
    if (e->l_prev) {
	ASSERT (e->l_prev->l_next == e);
	e->l_prev->l_next = e->l_next;
    } else {
	ASSERT (l->l_first == e);
	l->l_first = e->l_next;
    }
    if (e->l_next) {
	ASSERT (e->l_next->l_prev == e);
	e->l_next->l_prev = e->l_prev;
    } else {
	ASSERT (l->l_last == e);
	l->l_last = e->l_prev;
    }
    return el;
}
