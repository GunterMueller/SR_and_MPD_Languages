/*  gparam.c -- generate code for handling parameters  */

#include "compiler.h"

static void addparsize	PARAMS ((Sigptr g, Nodeptr a));
static void alcparm	PARAMS ((Proptr p, char*b, char*o, Parptr m,Nodeptr a));
static void sze		PARAMS ((Sigptr g, Nodeptr a));
static Bool isfixedsize	PARAMS ((Sigptr g));



/*  pbdef (s) -- return the typedef of the param block of resource or op s  */

char *
pbdef (s)
Symptr s;
{
    Proptr p;

    if (s->s_kind == K_OP)
	p = s->s_op->o_usig->g_proto;
    else {
	ASSERT (s->s_kind == K_RES);
	p = s->s_res->r_sig->g_proto;
    }
    return p->p_def;
}



/*  fixedpb (proto) -- return name of predeclared parameter block, if any  */

/*  output is in a static buffer; use it or lose it.  */

char *
fixedpb (proto)
Proptr proto;
{
    Parptr m;
    static int sn = 0;
    static char buf[20];

    for (m = proto->p_param; m != NULL; m = m->m_next)
	if (!isfixedsize (m->m_sig))
	    return NULL;			/* can't be fixed size */

    sn++;
    setstream (8);				/* output decl to %8 */
    cprintf ("struct {%s p; char s[32", proto->p_def);

    m = proto->p_param;
    if (m->m_sig->g_type != T_VOID)
	addparsize (m->m_sig, NULLNODE);	/* add room for return val */
    m = m->m_next;
    while (m != NULL) {
	if (m->m_pass != O_REF)
	    addparsize (m->m_sig, NULLNODE);	/* add room for other params */
	m = m->m_next;
    }

    cprintf ("];} q%d;\n", sn);			/* terminate decl */
    setstream (9);				/* reset %8 to %9 */

    sprintf (buf, "(Ptr)&q%d", sn);		/* format and return pointer */
    return buf;
}



/*  gparblk () -- gen code to alloc & fill a param block, returning its addr */

void
gparblk (blk, proto, actuals, qlist, opr)
char *blk;		/* parameter block name, if already allocated */
Proptr proto;		/* parameter list prototype */
Nodeptr actuals;	/* tree list of actual params */
Nodeptr qlist;		/* tree list of quantifiers for co */
Operator opr;		/* operator: O_CREATE for resource, else invocation */
{
    Nodeptr l, a, q, e;
    Parptr m;
    char *n, *p;

    p = alctemp (T_PTR);
    n = alctemp (T_INT);
    cprintf ("(");

    /*
     * For var/res slices, ensure that base array and indices are just
     * calculated once.  gexpr() will hide the slice expression under
     * a VERBATIM node, and then gparback() will dig it back out.
     */
    m = proto->p_param->m_next;
    l = actuals;
    while (m != NULL && l != NULL) {
	a = l->e_l;
	if (a->e_opr == O_SLICE && m->m_pass != O_VAL) {
	    if (m->m_pass == O_REF)
		EFATAL (a, "cannot pass slice expression to ref parameter");
	    once (a->e_l, ',');
	    while ((a = a->e_r) != NULL) {
		once (a->e_l->e_l, ',');
		once (a->e_l->e_r, ',');
	    }
	}
	m = m->m_next;
	l = l->e_r;
    }

    if (blk) {		/* if parameter block is fixed size and predeclared */
	cprintf ("%s=%s%,%s=SRALIGN(sizeof(%s)),\n", p, blk, n, proto->p_def);

    } else {

	/*
	 * Allocate space for building and initializing parameter block.
	 *
	 * Start with the fixed-length part (the appropriate C struct).  The
	 * added constant 32 allows for overruns when initializing zero-length
	 * arrays; the extra space is not needed after initialization and is
	 * not passed as part of the invocation message.  This overallocation
	 * seems easier than generating explicit code to special-case zero-
	 * length arrays.
	 *
	 * A negative size value indicates that the block is not to be
	 * associated with this resource; it may need to survive beyond
	 * our own destruction.
	 */
	cprintf ("\n%s=sr_alc(-(32+(%s=SRALIGN(sizeof(%s)))", p,n,proto->p_def);

	/*
	 * Now add room for the individual parameters.
	 */
	m = proto->p_param;
	if (m->m_sig->g_type != T_VOID)
	    addparsize (m->m_sig, NULLNODE);	/* add room for return val */
	m = m->m_next;
	l = actuals;
	while (m != NULL && l != NULL) {
	    if (m->m_pass != O_REF)
		addparsize (m->m_sig, l->e_l);	/* add room for other params */
	    m = m->m_next;
	    l = l->e_r;
	}
	for (q = qlist; q != NULL; q = q->e_r)
	    cprintf ("+%d", SRALIGN(sizeof (int))); /*add room for quantifiers*/
	cprintf ("),1),\n");	/* the ,1 is the second parameter to sr_alc */
    }

    /*
     * Initialize parameter values.
     */
    for (q = qlist; q != NULL; q = q->e_r)	/* co quantifiers */
	cprintf ("*(int*)(%s+%s)=%e,%s+=%d,\n",
	    p, n, q->e_l->e_r->e_l->e_l, n, SRALIGN (sizeof (int)));
    m = proto->p_param;
    if (m->m_sig->g_type != T_VOID)
	alcparm (proto, p, n, m, NULLNODE);	/* allocate return param */
    m = m->m_next;
    l = actuals;
    while (m != NULL && l != NULL) {
	a = l->e_l;
	if (m->m_pass != O_VAL & (!lvalue (a)))
	    EFATAL (a, "var/res/ref parameter is not a variable");
	else if (m->m_pass == O_REF)		/* if by ref, store address */
	    cprintf ("((%s*)%s)->_%d=%a,\n", proto->p_def, p, m->m_seq, a);
	else {
	    alcparm (proto, p, n, m, a);	/* allocate parameter */
	    if (m->m_pass != O_RES) {		/* if not result-only */
		e = bnode (O_ASSIGN,		/* construct asgmt expression */
		    parmnode (proto->p_def, p, m->m_seq, m->m_sig),  a);
		e->e_locn = a->e_locn;
		attest (e);			/* set signatures */
		cprintf ("%e,\n", e);		/* generate assignment */
	    }
	}
	m = m->m_next;
	l = l->e_r;
    }
    ASSERT (m == NULL && l == NULL);

    /*
     *  Store size in block.
     */
    if (opr == O_CREATE)
	cprintf ("((CRB*)%s)->crb_size=%s,\n", p, n);	/* create */
    else
	cprintf ("((Pach)%s)->size=%s,\n", p, n);	/* call, send, etc. */

    /*
     *  Return pointer to block.
     */
    cprintf ("%s)", p);
    freetemp (T_INT, n);
    freetemp (T_PTR, p);
}



/*  gparback() -- gen code to copy back any var and res params  */

void
gparback (proto, actuals, pbvar)
Proptr proto;		/* prototype */
Nodeptr actuals;	/* tree list of actual params */
char *pbvar;		/* var that points to param block */
{
    Nodeptr l, a, e;
    Parptr m;

    m = proto->p_param->m_next;
    l = actuals;
    while (m != NULL && l != NULL) {
	if (m->m_pass == O_VAR || m->m_pass == O_RES) {
	    a = l->e_l;
	    if (a->e_opr==O_VERBATIM && a->e_r!=NULL && a->e_r->e_opr==O_SLICE)
		a = a->e_r;	/* need original slice info for storing */

	    /* non-lvalue diagnosed was earlier; skip to avoid 2nd diagnostic */
	    if (lvalue (a)) {
		e = bnode (O_ASSIGN, a,
		    parmnode (proto->p_def, pbvar, m->m_seq, m->m_sig));
		e->e_locn = a->e_locn;
		attest (e);
		cprintf (",\n%e", e);
	    }
	}
	m = m->m_next;
	l = l->e_r;
    }
    ASSERT (m == NULL && l == NULL);
}



/*  addparsize (g, a) -- add the size of parameter data.
 *
 *  Generates "+expr" to allow for the data of descriptor-type parameters
 *  (which are incompletely declared in the parameter block).
 *  g is the parameter's signature and a, if available, is the
 *  corresponding actual value (needed for "array[1:*] etc.).
 */

static void
addparsize (g, a)
Sigptr g;
Nodeptr a;
{
    if (DESCRIBED (g)) {
	cprintf ("+SRALIGN(");
	sze (g, a);
	cprintf (")");
    }
}



/*  alcparm (proto, pbptr, oname, m, a) -- allocate and init parameter n.
 *
 *  Generates code to initialize a descriptor-type parameter, 
 *  store its offset, and update the offset temp "oname".
 */

static void
alcparm	(proto, pbptr, oname, m, a)
Proptr proto;
char *pbptr, *oname;
Parptr m;
Nodeptr a;
{
    Sigptr g, h;
    Type t;
    char *p;
    int i, n;

    g = m->m_sig;
    if (!DESCRIBED (g))
	return;
    p = alctemp (T_PTR);

    /* store offset and set pointer to parameter data structure */
    cprintf ("%s=%s+(((%s*)%s)->o_%d=%s),\n",
	p, pbptr, proto->p_def, pbptr, m->m_seq, oname);

    /* store size and bump offset */
    cprintf ("%s+=SRALIGN(DSIZE(%s)=", oname, p);
    sze (g, a);
    cprintf ("),\n");

    /* if array, need to initialize bounds fields and possibly elements */
    if (g->g_type == T_ARRAY) {
	ASSERT (g->g_lb != NULL && g->g_ub != NULL);
	if ((g->g_lb->e_opr == O_ASTER || g->g_ub->e_opr == O_ASTER) 
	&& (a == NULL || g->g_lb->e_opr == g->g_ub->e_opr)) {
	    EFATAL (g->g_lb, "illegal use of `*' bounds");
	    freetemp (T_PTR, p);
	    return;
	}

	/* create initializer if needed */
	h = usig (g);
	t = h->g_type;			/* underlying type of array */
	if (t == T_STRING) {
	    cprintf ("((String*)(%s+%d))->size=", p,
		sizeof (Array) + ndim (g) * sizeof (Dim));
	    if (h->g_ub->e_opr == O_ASTER)
		cprintf ("((String*)((Ptr)%a+%d))->size,\n",
		    a, sizeof (Array) + ndim (g) * sizeof (Dim));
	    else
		cprintf ("%e+%d,\n", h->g_ub, STRING_OVH);
	}

	/* initialize array: pass address, element size, initializer address */
	cprintf ("sr_init_array(%t,(Array*)%s,", a, p);
	switch (t) {
	    case T_STRING:
		if (h->g_ub->e_opr == O_ASTER)
		    cprintf ("SRALIGN(((String*)((Ptr)%a+%d))->size),",
			a, sizeof (Array) + ndim (g) * sizeof (Dim));
		else
		    cprintf ("SRALIGN(%e+%d),", h->g_ub, STRING_OVH);
		cprintf ("%s+%d,\n", p, sizeof(Array) + ndim(g) * sizeof(Dim));
		break;
	    case T_REC:
		ASSERT (h->g_rec);
		cprintf ("sizeof(%g),", h);
		if (h->g_rec->k_init)
		    cprintf ("(Ptr)&%s,", h->g_rec->k_init);
		else
		    cprintf ("(Ptr)0,");
		break;
	    default:
		cprintf ("sizeof(%g),(Ptr)0,", h);
		break;
	}
	/* pass number of dimensions followed by bounds */
	n = ndim (g);
	cprintf ("%d", n);
	for (i = 1; g->g_type == T_ARRAY; g = g->g_usig, i++) {
	    ASSERT (g->g_lb != NULL && g->g_ub != NULL);
	    if (g->g_ub->e_opr == O_ASTER)
		cprintf ("%,%e,UB(%a,%d)-LB(%a,%d)+%e",
		    g->g_lb, a, n - i, a, n - i, g->g_lb);
	    else if (g->g_lb->e_opr == O_ASTER)
		cprintf ("%,%e-(UB(%a,%d)-LB(%a,%d)),%e",
		    g->g_ub, a, n - i, a, n - i, g->g_ub);
	    else
		cprintf ("%,%e,%e", g->g_lb, g->g_ub);
	}
	cprintf ("),\n");
    }
    freetemp (T_PTR, p);
}



/*  sze (g, a) -- generate expression for size of descriptor parameter.  */

/*  The expression is the actual size, not the aligned size.  */
/*  This is vital for setting the correct maxlength of strings.  */

static void
sze (g, a)
Sigptr g;
Nodeptr a;
{
    switch (g->g_type) {
	case T_STRING:
	    ASSERT (a == NULL || a->e_sig->g_type == T_STRING);
	    if (g->g_ub != NULL && g->g_ub->e_opr != O_ASTER)
		cprintf ("%e+%d", g->g_ub, STRING_OVH);
	    else if (a == NULL)
		BOOM ("can't determine size for parameter", "");
	    else
		cprintf ("%E->size", a);
	    break;
	case T_ARRAY:
	    ASSERT (a == NULL || a->e_sig->g_type == T_ARRAY);
	    if (isfixedsize (g)) {
		cprintf ("SRALIGN(%d+", sizeof(Array) + ndim(g) * sizeof(Dim));
		while (g->g_type == T_ARRAY) {
		    cprintf ("(%e-%e+1)*", g->g_ub, g->g_lb);
		    g = g->g_usig;
		}
		if (DESCRIBED (g)) {
		    cprintf ("SRALIGN(");
		    sze (g, NULLNODE);
		    cprintf (")");
		} else {
		    cprintf ("sizeof(%g)", g);
		}
		cprintf (")");
	    } else if (a == NULL)
		BOOM ("can't determine size for parameter", "");
	    else
		cprintf ("SRALIGN(%E->size)", a);
	    break;
	default:
	    BOOM ("bad type in sze", typetos (g->g_type));
	    break;
    }
}



/*  isfixedsize (g) -- return TRUE iff g contains only constant bounds  */

static Bool
isfixedsize (g)
Sigptr g;
{
    switch (g->g_type) {
	case T_STRING:
	    return g->g_ub != NULL && g->g_ub->e_opr == O_ILIT;
	case T_ARRAY:
	    if (g->g_lb != NULL && g->g_lb->e_opr == O_ILIT &&
	        g->g_ub != NULL && g->g_ub->e_opr == O_ILIT)
		    return isfixedsize (g->g_usig);
	    else
		return FALSE;
	default:
	    return TRUE;
    }
}
