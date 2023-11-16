/*  fold.c -- folding constants  */

#include <math.h>
#include "compiler.h"

static Nodeptr	ireplace PARAMS ((Nodeptr e, int v));
static Nodeptr	rreplace PARAMS ((Nodeptr e, Real v));



/*  fold (e) -- fold constants within tree e.
 *
 *  For each node e, recursively: if e can be replaced by a constant, do so
 *  by modifying it in place (in case of references from elsewhere).
 *
 *  Return e if it is constant, else return null.
 */

Nodeptr
fold (e)
Nodeptr e;
{
    Operator o;
    Nodeptr l, r;
    Symptr s;
    Type t;
    int v;

    o = e->e_opr;

    switch (o) {	/* certain parts of the tree should not be walked */
	case O_ENUM:
	    return NULL;
	case O_SUBS:
	    /* don't replace L side */
	    if (e->e_r != NULL)
		fold (e->e_r);
	    return NULL;
    }

    l = LNODE (e);  if (l != NULL) l = fold (l);
    r = RNODE (e);  if (r != NULL) r = fold (r);

    if (l == NULL) {

	/*
	 *  Not a typical operator.  Check a few other possibilities.
	 */
	switch (o) {
	    case O_BLIT:
	    case O_CLIT:
	    case O_ILIT:
	    case O_RLIT:
	    case O_SLIT:
		return e;		/* already a constant */

	    case O_FIELD:
		if (r != NULL) {	/* if imported constant */
		    *e = *r;
		    return e;
		}
		return NULL;

	    case O_SYM:
		s = e->e_sym;
		if (s->s_kind==K_VAR && s->s_var!=NULL) {  /* named constant? */
		    if ((r = s->s_var->v_value) != NULL && fold (r)) {
			*e = *r;		/* replace node by value */
			return e;
		    };
		} else if (s->s_kind == K_ELIT) {	/* check enum literal */
		    return ireplace (e, s->s_seq);
		}
		return NULL;

	    case O_CAST:
		if (r == NULL)
		    break;
		t = e->e_sig->g_type;		/* destination type */
		switch (r->e_sig->g_type) {	/* source type */
		    case T_REAL:
			if (t == T_REAL) {
			    /* avoid truncation to int by general code */
			    *e = *r;
			    return e;
			}
			v = *r->e_rptr;
			break;
		    case T_BOOL:
		    case T_CHAR:
		    case T_INT:
		    case T_ENUM:
			v = r->e_int;
			break;
		    default:
			return NULL;
		}
		switch (t) {
		    case T_REAL:
			return rreplace (e, (Real) v);
		    case T_BOOL:
			return ireplace (e, v != 0);
		    case T_CHAR:
			return ireplace (e, v & 0xFF);
		    case T_INT:
		    case T_ENUM:
			return ireplace (e, v);
		    default:
			return NULL;
		}

	};

    } else if (r == NULL) {

	/*
	 *  One constant child.  Check for foldable unary operator.
	 *
	 *  could add:  {low,high} ( {char,enum,bool,int} )
	 */
	switch (o) {
	    case O_POS:
		*e = *l;
		return e;
	    case O_NEG:
		if (e->e_sig->g_type == T_REAL)
		    return rreplace (e, -*l->e_rptr);
		else
		    return ireplace (e, -l->e_int);
	    case O_NOT:
		if (e->e_sig->g_type == T_BOOL)
		    return ireplace (e, !l->e_int);
		else
		    return ireplace (e, ~l->e_int);
	};

    } else {

	/*
	 *  Two constant children.  Check for foldable binary operator.
	 */

#define XREPLACE(e,o) \
(t==T_REAL?rreplace(e,*l->e_rptr o *r->e_rptr):ireplace(e,l->e_int o r->e_int))

	t = l->e_sig->g_type;
	switch (o) {
	    /* note: no worry about pointer arithmetic; won't get this far */
	    case O_ADD:    return XREPLACE (e, +);
	    case O_SUB:    return XREPLACE (e, -);
	    case O_MUL:    return XREPLACE (e, *);
	    case O_DIV:    return XREPLACE (e, /);

	    case O_LSHIFT: return ireplace (e, l->e_int << r->e_int);
	    case O_RSHIFT: return ireplace (e, l->e_int >> r->e_int);
	    case O_XOR:    return ireplace (e, l->e_int ^ r->e_int);
	    case O_OR:	   return ireplace (e, l->e_int | r->e_int);
	    case O_AND:    return ireplace (e, l->e_int & r->e_int);

	    case O_REMDR: 
		if (t == T_REAL)
#ifdef NO_FMOD
		    return NULL;	/* can't do */
#else
		    return rreplace (e, fmod (*l->e_rptr, *r->e_rptr));
#endif
		else
		    return ireplace (e, l->e_int % r->e_int); 
	};
    }
    return NULL;
}



/* ireplace (e, v) -- replace bool/char/int/enum node with literal value v  */

static Nodeptr
ireplace (e, v)
Nodeptr e;
int v;
{
    Type t = e->e_sig->g_type;

    if (t == T_BOOL)
	e->e_opr = O_BLIT;
    else if (t == T_CHAR)
	e->e_opr = O_CLIT;
    else
	e->e_opr = O_ILIT;
    e->e_l = e->e_r = NULL;
    e->e_int = v;
    /* leave e->e_sig and e->e_locn unaltered */
    return e;
}



/* rreplace (e, v) -- replace real node with literal value v  */

static Nodeptr
rreplace (e, v)
Nodeptr e;
Real v;
{
    ASSERT (e->e_sig->g_type == T_REAL);
    e->e_opr = O_RLIT;
    e->e_l = e->e_r = NULL;
    *(e->e_rptr = NEW (Real)) = v;
    /* leave e->e_sig and e->e_locn unaltered */
    return e;
}
