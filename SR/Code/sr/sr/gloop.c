/*  gloop.c -- loop generation code  */

#include "compiler.h"


/*  qbegin (l) -- generate the prolog for code controlled by quantifier list  */

void
qbegin (l)
Nodeptr l;
{
    Nodeptr e;
    Symptr v;

    if (l == NULL)
	return;
    while (l != NULL) {

	e = l->e_l;				/* O_QUANT node */
	ASSERT (e->e_opr == O_QUANT);
	v = e->e_r->e_l->e_l->e_sym;		/* iteration var's symtab ent */

	cprintf ("{\n%g %n=%e;\n", v->s_var->v_sig, v, e->e_l->e_l);
	freetrans (0, ';');
	cprintf ("for (; %f; %n+=%e", e->e_r->e_l, v, e->e_l->e_r);
	freetrans (0, ',');
	cprintf (") ");
	if (e->e_r->e_r != NULL)
	    cprintf ("\nif (%f)", e->e_r->e_r);	/* "st" clause */

	l = l->e_r;
    }

    cprintf ("{\n");
    if (!option_T) {
	cprintf ("if (--sr_private[MY_JS_ID].rem_loops<=0)\n");
	cprintf ("   sr_loop_resched(%t);\n", e);
    }
}



/*  qend (l) -- generate the epilog for code controlled by quantifier list  */

void
qend (l)
Nodeptr l;
{
    if (l == NULL)
	return;
    while (l != NULL) {			/* one } per quantifier */
	cprintf ("}\n");
	l = l->e_r;
    }
    cprintf ("}\n");			/* one more for overall brace */
}



/*
 *  bgnloop() -- reserve labels for "next" and "exit" statements
 *  goloop(e) -- generate goto depending on e->e_opr (O_NEXT or O_EXIT)
 *  nextloop() -- generate the innermost "next" label
 *  endloop() -- generate the innermost "exit" label and forget the loop
 *
 *  These routines also keep track of persistent temporaries.
 *  e is a Node used for diagnostics if the exit or next is inappropriate.
 */

static int nextlab[MAX_NESTING];
static int exitlab[MAX_NESTING];
static int pststack[MAX_NESTING];
static int instack[MAX_NESTING];
static int exitdepth = 0;

void
bgnloop ()
{
    exitdepth++;
    nextlab[exitdepth] = nextlabel++;
    exitlab[exitdepth] = nextlabel++;
    pststack[exitdepth] = npst ();
    instack[exitdepth] = indepth;
}

void
goloop (e)
Nodeptr e;
{
    int i;

    for (i = indepth; i > instack[exitdepth]; i--)
	if (invptr[i] != NULL)
	    cprintf ("sr_finished_input(%t,(Ptr)%s);\n", e, invptr[i]);
    freepst (pststack[exitdepth], FALSE);
    if (exitdepth == 0)
	EFATAL (e, "no enclosing loop");
    else if (e->e_opr == O_NEXT)
	cprintf ("goto %L;\n", nextlab[exitdepth]);
    else
	cprintf ("goto %L;\n", exitlab[exitdepth]);
}

void
nextloop ()
{
    cprintf ("%s%L:;\n", "\n", nextlab[exitdepth]);
}

void
endloop ()
{
    cprintf ("%s%L:;\n", "\n", exitlab[exitdepth--]);
}
