/*  attest.c -- affix signatures to nodes and symbols  */

#include "compiler.h"

static Sigptr	ncast	PARAMS ((Nodeptr l, Nodeptr *r));
static Sigptr	subsig	PARAMS ((Nodeptr e, Sigptr g));
static Sigptr	nfix	PARAMS ((Nodeptr e, char *msg));
static Sigptr	ixattest PARAMS ((Nodeptr e, int try));
static void	ixslice	PARAMS ((Nodeptr e));
static void	parlist	PARAMS ((Sigptr g, Nodeptr r, char *kwd));
static void	rparms	PARAMS ((Resptr r));
static void	ckargs	PARAMS ((Nodeptr e, Proptr p));
static void	nostars	PARAMS ((Nodeptr e, Sigptr g));
static void	ckcfop	PARAMS ((Nodeptr l));



static List opstack;		/* stack of ops being implemented */

static Segment seg;		/* current segment */

static Bool inspec;		/* are we in a spec? */



/*  attest (e) -- affix signatures to nodes and symbols
 *
 *  This is a major compiler pass, implemented as a recursive tree walk.
 *
 *  -- Process all declarations.
 *  -- Put signatures in all tree nodes.
 *  -- Check signatures of operands.
 *
 *  This code does *not* check that assignment targets (etc.) are lvalues.
 *
 *  attest (e) returns the signature of e, if any.
 */

Sigptr
attest (e)
Nodeptr e;
{
    Nodeptr l, r, a, id;
    Sigptr f, g, h;
    Varptr v;
    Variety vty;
    Symptr p, s;
    Op unkop;
    Opptr o;
    Parptr m;
    Type t, u;
    Resptr res;
    Operator opr;
    Segment oldseg;
    int n;
    char *name;
    char buf[100];
    static int blknest = 0;
    static int varseq = 0;
    static Op zop;		/* empty op struct, for initializing */
    static Recdata zrec;	/* zero, for initializing */
    static int nrecs = 0;	/* number of record types processed */

    if (e == NULL)
	return void_sig;
    if (e->e_sig != NULL)
	return e->e_sig;		/* already processed sub-dag */

    g = void_sig;			/* set default signature */
    l = LNODE (e);			/* set left and right nodes */
    r = RNODE (e);

    /* Some nodes cannot be processed in a bottom-up manner.
     * Process these first; each case returns when done.
     */
    switch (e->e_opr) {

	case O_COMPONENT:
	case O_IMPORT:
	    opr = e->e_opr;
	    if (!r)
		return e->e_sig = void_sig;
	    ASSERT (l->e_opr == O_SYM);
	    
	    oldseg = seg;
	    s = l->e_sym;
	    res = s->s_res;
	    if (r->e_opr == O_GLOBAL) {
		g = newsig (T_GLB, NULLSIG);
	    } else {
		g = newsig (T_RES, NULLSIG);
		if (e->e_opr == O_COMPONENT)	/* so we only report once */
		    for (a = r->e_l->e_r; a != NULL; a = a->e_r)
			if (a->e_l->e_r->e_r->e_opr != O_VAL) {
			    EFATAL (a->e_l->e_r->e_r,
				"resources can only have value parameters");
			    break;
			}
	    }
	    g->g_sym = s;
	    s->s_sig = res->r_sig = g;

	    if (opr == O_COMPONENT) {
		inspec = TRUE;
		opstack = list (sizeof (Opptr));
		if (r->e_opr == O_GLOBAL)
		    seg = SG_GLOBAL;
		else
		    seg = SG_RESOURCE;
	    } else if (r->e_opr == O_GLOBAL) {
		seg = SG_GLOBAL;
	    } else {
		seg = SG_IMPORT;
	    }

	    /* process spec */
	    attest (r->e_l->e_l);
	    attest (r->e_l->e_r);
	    g->g_proto = prototype (r->e_l->e_r);
	    protoname (g->g_proto, s);
	    if (opr == O_COMPONENT) {
		rparms (res);
		inspec = FALSE;
	    }

	    /* process body */
	    if (opr == O_COMPONENT)
		seg = SG_RESOURCE;	/* may have been SG_GLOBAL */
	    attest (r->e_r);

	    if (opr == O_IMPORT && r->e_opr == O_GLOBAL)
		rimpl (l);		/* read global implementation info */

	    seg = oldseg;
	    return e->e_sig = l->e_sig = r->e_sig = g;

	case O_EXTERNAL:
	    if (l->e_r != NULL) {
		EFATAL (e, "no subscripts allowed on external");
		l->e_r = NULL;
	    }
	    /*NOBREAK*/
	case O_OP:
	    attest (l);				/* process name and subscripts*/
	    s = l->e_l->e_sym;
	    o = newop (s, seg);			/* make Op for ref in params */
	    g = attest (r);			/* now process params */
	    if (r->e_opr == O_PROTO)		/* if prototype, not optype */
		protoname (g->g_proto, s);	/* fill in name */
	    else if (r->e_opr == O_SYM) {
		p = r->e_sym;
		if (p->s_kind != K_TYPE || g->g_type != T_OP) {
		    err (e->e_locn, "`%s' is not an optype", p->s_name);
		    g = int_sig;
		}
	    }
	    *o->o_usig = *g;
	    g = subsig (l->e_r, g);		/* handle subscripts */
	    *o->o_asig = *g;			/* change sig to reflect subs */
	    while (g->g_type == T_ARRAY) /* if array, find underlying op. */
		g = g->g_usig;

	    o->o_exported = inspec;
	    if (e->e_opr == O_EXTERNAL		/* if external, */
	    ||  g->g_type != T_OP		/* or if not op (is opcap), */
	    ||  g->g_usig->g_type != T_VOID	/* or if returns a value, */
	    ||  g->g_proto->p_param->m_next	/* or if has params, */
	    ||  (!IMPORTED (s) && s->s_depth>2)	/* or if in a block, */
	    ||  inspec)				/* or if exported, */
		o->o_nosema = TRUE;		/* can't be impl as semaphore */
	    if (e->e_opr == O_EXTERNAL) {
		o->o_impl = I_EXTERNAL;
		if (g->g_proto->p_rstr != O_RNONE)
		    EFATAL (e, "no restrictions allowed on external");
		if (!IMPORTED (s) && s->s_depth > 2)
		    EFATAL (e, "external must be declared at resource level");
	    }
	    return e->e_sig = g;

	case O_PROC:
	    ASSERT (r->e_opr == O_BLOCK && r->e_l->e_opr == O_SYM);
	    oldseg = seg;
	    seg = SG_PROC;
	    s = r->e_l->e_sym;
	    if (s->s_kind == K_OP) {
		o = s->s_op;
		parlist (o->o_usig, l, "proc");
		o->o_nosema = TRUE;
		if (o->o_impl != I_UNIMPL && o->o_impl != I_CAP)
		    err (l->e_locn, "duplicate implementation of %s",s->s_name);
		if (o->o_asig->g_type == T_ARRAY)
		    EFATAL (l, "a proc cannot implement an array op");
		if (IMPORTED (o->o_sym))
		    EFATAL (l, "a proc cannot implement an imported op");
		o->o_impl = I_PROC;
		* (Opptr *) lpush (opstack) = o;
		attest (r);
		lpop (opstack);
	    } else
		err (l->e_locn, "not an op: %s", s->s_name);
	    
	    seg = oldseg;
	    return e->e_sig = void_sig;

	case O_INOP:
	    /* l is currently null; replace with SYM node if input by opname */
	    a = r;
	    while (a->e_opr == O_INDEX || a->e_opr == O_SLICE)
		a = a->e_l;			/* descend past subscripts */
	    s = NULL;
	    if (a->e_opr == O_SYM) {		/* if a symbol name */
		s = a->e_sym;			/* get info */
		if (s->s_kind == K_OP) {
		    /* this is an input by opname, possibly subscripted */
		    /* put underlying symbol, with signature, in l */
		    l = e->e_l = a;
		    l->e_sig = symsig (s);
		}
	    }

	    if (l != r)		/* skip if same; would inhibit optimization */
		attest (r);

	    if (l != NULL) {
		if (s->s_kind != K_OP)
		    err (l->e_locn, "not an op: %s", s->s_name);
		else if (r->e_sig->g_type != T_OP)
		    err (l->e_locn, "incorrect subscripting of %s", s->s_name);
	    } else if (r->e_sig->g_type != T_OP && r->e_sig->g_type != T_OCAP) {
		if (s != NULL)
		    err (a->e_locn, "not an op: %s", s->s_name);
		else
		    EFATAL (r, "cannot in/receive/P: not an op");
	    }
	    return e->e_sig = void_sig;

	case O_INPARM:
	    ASSERT (l->e_opr == O_INOP);
	    attest (l);
	    parlist (l->e_r->e_sig, r, "in");
	    if (l->e_l != NULL) {
		/* input by opname */
		s = l->e_l->e_sym;
		ASSERT (s->s_kind == K_OP);
		o = s->s_op;
		* (Opptr *) lfirst (opstack) = o;	/* replace stk entry */
		if (o->o_impl == I_UNIMPL || o->o_impl == I_CAP)
		    o->o_impl = I_IN;
		else if (o->o_impl != I_IN)
		    err (e->e_locn,"duplicate implementation of %s",s->s_name);
	    } else {
		/* input from capability, or error */
		o = * (Opptr *) lfirst (opstack);
		o->o_usig = l->e_r->e_sig;	/* put sig in opstack entry */
	    }
	    return e->e_sig = void_sig;

	case O_ARM:
	    /*
	     *  Need separate entry on opstack for each nested input stmt.
	     *  Put skeleton entry in case it's an input from a capability;
	     *  this is replaced by real entry if input from named opertion.
	     */
	    unkop = zop;	/* zero local struct variable */
	    * (Opptr *) lpush (opstack) = &unkop;	
	    attest (l);		/* replaces unkop if named op used */
	    if (nfatals == 0)
		attest (r);
	    lpop (opstack);
	    return e->e_sig = void_sig;

	case O_QUANT:
	    ASSERT (l->e_opr == O_QSTEP);
	    ASSERT (r->e_opr == O_QTEST);
	    ASSERT (r->e_l->e_l->e_opr == O_SYM);

	    /* important: follow same sequence as in identify() */

	    g = attest (l->e_l);		/* initial */
	    if (!ORDERED (g)) {
		EFATAL (l->e_l, "illegal quantifier type");
		g = int_sig;
	    }
	    l->e_sig = g;

	    id = r->e_l->e_l;			/* quantifier variable */
	    s = id->e_sym;
	    v = newvar (s, V_LOCAL, O_VARDCL, g);
	    v->v_set = v->v_used = TRUE;
	    attest (id);

	    h = attest (r->e_l->e_r);		/* limit */
	    if (!compat (g, h, C_COMPARE))
		EFATAL (r->e_l->e_r, "incompatible quantifier limit");
	    r->e_l->e_sig = r->e_sig = bool_sig;

	    h = attest (l->e_r);		/* increment */
	    if (h->g_type == T_REAL) {
		if (g->g_type != T_REAL)
		    nfix (r, "incompatible quantifier increment");
	    } else if (h->g_type != T_INT)
		nfix (r, "illegal quantifier increment");

	    if (r->e_r != NULL) {		/* suchthat */
		h = attest (r->e_r);
		if (h->g_type != T_BOOL)
		    EFATAL (r->e_r, "`st' expression must be boolean");
	    }

	    return e->e_sig = void_sig;

	case O_PARDCL:
	    attest (l->e_r);		/* subscript list */
	    attest (r);			/* type */
	    g = subsig (l->e_r, r->e_l->e_sig);
	    id = l->e_l;
	    ASSERT (id != NULL && id->e_opr == O_SYM);
	    s = id->e_sym;
	    ASSERT (s != NULL);
	    return e->e_sig = s->s_sig = g;

	case O_VARDCL:
	case O_CONDCL:
	case O_FLDDCL:
	    attest (l->e_r);			/* process range expressions */
	    attest (r);				/* process type declaration */
	    if (r->e_l)	{			/* if type is explicit */
		g = subsig (l->e_r, r->e_l->e_sig);	/* use, plus subs */
		if (r->e_r != NULL && !compat (g, r->e_r->e_sig, C_ASSIGN))
		    EFATAL (e, "incompatible initialization");
	    } else {
		g = r->e_r->e_sig;		/* else use signat of initval */
		if (g->g_type == T_VOID || g->g_type == T_NLIT) {
		    EFATAL (e, "illegal initializer");
		    r = e->e_r = NULL;
		    g = int_sig;
		}
		if (g->g_type == T_OP)
		    g = newsig (T_OCAP, g);
		if (l->e_r != NULL) {
		    /* left side has subscripts; find base type of right side */
		    for (g = r->e_sig; g->g_type == T_ARRAY; g = g->g_usig)
			;
		    g = subsig (l->e_r, g);
		}
	    }
	    nostars (r, g);

	    id = l->e_l;
	    ASSERT (id != NULL && id->e_opr == O_SYM && id->e_sym != NULL);
	    s = id->e_sym;
	    if (e->e_opr == O_FLDDCL)
		vty = V_RMBR;
	    else if (seg == SG_GLOBAL)
		vty = V_GLOBAL;
	    else if (blknest > 0 || seg == SG_PROC)
		vty = V_LOCAL;
	    else 
		vty = V_RVAR;
	    v = newvar (s, vty, e->e_opr, g);
	    if (e->e_opr == O_CONDCL && r != NULL)  {
		v->v_value = r->e_r;
		v->v_set = TRUE;
	    } else if (e->e_opr == O_VARDCL && 
		    (seg == SG_IMPORT || (seg == SG_RESOURCE && inspec))) {
		EFATAL (e, "var decl in spec part of resource");
	    }
	    if (blknest > 0)
		v->v_seq = ++varseq;
	    return e->e_sig = g;

	case O_FIELD:		
	    ASSERT (r->e_opr == O_ID);
	    id = r;
	    name = id->e_name;
	    attest (l);
	    g = l->e_sig;
	    if (g->g_type == T_RCAP)
		g = g->g_usig;
	    if (g->g_type == T_RES || g->g_type == T_GLB) {

		/* looking for something imported from a resource or global */
		p = g->g_sym;
		for (s = p->s_res->r_parm->s_prev; s != NULL; s = s->s_prev)
		    if (s->s_imp == p && s->s_name == name) {
			id->e_opr = O_SYM;
			id->e_sym = s;
			if (s->s_sig)
			    return e->e_sig = id->e_sig = symsig (s);
			else {
			    err (e->e_locn,
				"forward reference in recursive import: %s",
				s->s_name);
			    return nfix (e, NULLSTR);
			}
		    }
		err (e->e_locn, "undefined identifier: %s", name);
		return nfix (e, NULLSTR);
	    }

	    if (g->g_type != T_REC)
		return nfix (e, "not a rec");

	    /* looking for a field of a record */
	    for (s = g->g_sym; s != NULL; s = s->s_next)
		if (s->s_name == name) {
		    id->e_opr = O_SYM;
		    id->e_sym = s;
		    return e->e_sig = id->e_sig = s->s_var->v_sig;
		}
	    err (e->e_locn, "unknown field: %s", name);
	    return nfix (e, NULLSTR);

	case O_TYPENAME:
	    /*
	     * Verify that L side is a type.  If R side is not null, also
	     * allow an optype; zero R side if not.
	     */
	    if (l->e_opr == O_FIELD) {	/* handle qualified_id */
		attest (l);
		if (l->e_opr != O_FIELD)	/* if fatal error */
		    return e->e_sig = int_sig;
		l = l->e_r;
	    }
	    ASSERT (l->e_opr == O_SYM);
	    s = l->e_sym;
	    g = s->s_sig;
	    if (s->s_kind != K_TYPE || (g->g_type == T_OP && r == NULL)) {
		err (e->e_locn, "`%s' is not a type", s->s_name);
		g = int_sig;
	    } else if (g->g_type == T_OP)
		g = newsig (T_OCAP, s->s_sig);	/* for optype, use cap sig */
	    else
		e->e_r = NULL;		/* indicate not an optype name */
	    return e->e_sig = g;

	case O_BLOCK:
	case O_SUBS:
	    /* left side is id; leave alone */
	    blknest++;
	    attest (r);
	    blknest--;
	    return e->e_sig = void_sig;
	
	case O_TYPE:
	case O_OPTYPE:
	    /* space for signat was allocated earlier, to handle forward refs */
	    ASSERT (l->e_opr == O_SYM);
	    ASSERT (l->e_sym->s_sig != NULL);
	    ASSERT (l->e_sym->s_sig->g_type == T_BOGUS);
	    g = attest (r);		/* process signature */
	    if (r->e_opr == O_PROTO)	/* if optype, save the name */
		protoname (g->g_proto, l->e_sym);
	    s = l->e_sym;
	    *s->s_sig = *g;		/* fill signat alloc'd in identify() */
	    g = r->e_sig = s->s_sig;	/* use that sig ptr instead */
	    if (e->e_opr == O_OPTYPE && s->s_op == NULL) {
		/* need to make op entry */
		o = newop (g->g_sym->s_prev, SG_RESOURCE);
		*o->o_asig = *o->o_usig = *g;
		o->o_impl = I_DCL;
	    }
	    return e->e_sig = g;

	case O_ENUM:		
	    ASSERT (l->e_opr == O_LIST);
	    ASSERT (l->e_l->e_opr == O_SYM)
	    s = l->e_l->e_sym;
	    g = newsig (T_ENUM, NULLSIG);
	    g->g_sym = s;
	    while (l != NULL) {
		l->e_l->e_sym->s_sig = g;
		l = l->e_r;
	    }
	    return e->e_sig = g;

	case O_CALL:
	case O_SEND:
	case O_FORWARD:
	    /* don't fully process a SYM to avoid marking no-semaphore status */
	    /* note: similar code for O_QMARK */
	    if (l->e_opr == O_SYM)
		l->e_sig = symsig (l->e_sym);	/* just set signat if a SYM */
	    else if (e->e_opr == O_SEND && subop (l))
		/* maybe subscripted sem op, so keep it that way */
		l->e_sig = ixattest (l, TRUE);
	    else
		attest (l);			/* else process normally */
	    attest (r);
	    g = l->e_sig;
	    if (g->g_type == T_OCAP)
		g = g->g_usig;
	    if (g->g_type != T_OP) {
		if (l->e_opr == O_SYM)
		    err (e->e_locn, "not an op: %s", l->e_sym->s_name);
		else
		    EFATAL (e, "invocation: not an op");
		return e->e_sig = nfix (e, NULLSTR);
	    }
	    ckargs (e, g->g_proto);		/* check argument list */

	    switch (e->e_opr) {
		case O_CALL:
		    ckcfop (l);
		    if (g->g_proto->p_rstr == O_RSEND)
			EFATAL (e, "op is restricted {send}");
		    return e->e_sig = g->g_usig;
		case O_SEND:		
		    if (g->g_proto->p_rstr == O_RCALL)
			EFATAL (e, "send: op is restricted {call}");
		    return e->e_sig = void_sig;
		case O_FORWARD:		
		    if (lfirst (opstack) == NULL)
			EFATAL (e, "forward: not allowed outside a proc");
		    else {
			o = * (Opptr *) lfirst (opstack);
			if (!compat (o->o_usig, g, C_EXACT))
			    EFATAL (e, "forward to incompatible op");
			o->o_sepctx = TRUE;	/* force separate context */
			/* forwarding makes both the enclosing op */
			/* and target op not optimizable by semaphore */
			/* (code generator assumes have invocation block) */
			o->o_nosema = TRUE;
			ckcfop (l);
		    }
		    if (g->g_proto->p_rstr == O_RCALL)
			EFATAL (e, "forward: op is restricted {call}");
		    return e->e_sig = void_sig;
	    }

	case O_QMARK:		
	    /* don't fully process a SYM to avoid marking no-semaphore status */
	    /* note: similar code for O_CALL, O_SEND, O_FORWARD */
	    if (l->e_opr == O_SYM)
		l->e_sig = symsig (l->e_sym);
	    else if (subop (l))
		/* maybe subscripted sem op, so keep it that way */
		l->e_sig = ixattest (l, TRUE);
	    else
		attest (l);
	    t = l->e_sig->g_type;
	    if (t != T_OP && t != T_OCAP)
		nfix (e, "`?' requires an operation");
	    return e->e_sig = int_sig;

	case O_INDEX:  /* note: not relevant in sem opt */
	    g = ixattest (e, FALSE);	/* set signatures of indices */
	    if (g->g_type == T_ARRAY)
		ixslice (e);		/* turn subarray into slice tree */
	    return e->e_sig = g;	/* set result signature */

	case O_SLICE:
	    if (l->e_opr == O_INDEX) /* don't turn INDEX into SLICE */
		l->e_sig = ixattest (l, FALSE); /* not relevant for sem opt */
	    else
		attest (l);
	    attest (r);
	    g = h = l->e_sig;
	    if (g->g_type == T_STRING) {
		t = r->e_l->e_sig->g_type;
		if (r->e_r != NULL)
		    EFATAL (r, "too many subscripts");
		else if (t != T_INT)
		    EFATAL (r, "wrong subscript type");
		return e->e_sig = string_sig;
	    }
	    if (g->g_type != T_ARRAY)
		return e->e_sig = nfix(e,"subscripted object is not an array");

	    while (r != NULL) {
		ASSERT (r->e_opr == O_LIST);
		a = r->e_l;
		ASSERT (a->e_opr == O_RANGE);
		t = h->g_type;
		if (t != T_ARRAY)
		    return e->e_sig = nfix (e, "too many subscripts");
		if (h->g_ub == NULL)
		    f = int_sig;
		else
		    f = h->g_ub->e_sig;
		if (!compat (f, a->e_sig, C_ASSIGN))
		    return e->e_sig = nfix (e, "wrong subscript type");
		h = h->g_usig;
		r = r->e_r;
		if (a->e_l == NULL)	/* if this dimension not a slice */
		    g = g->g_usig;	/* reduce result dimension */
	    }
	    if (r == NULL) {		/* if no earlier errors */
	        /* copy signature */
	        f = h = newsig (T_ARRAY, g->g_usig);
	        h->g_lb = intnode (1);
	        h->g_ub = NULL;

		while (f->g_usig != NULL && f->g_usig->g_type == T_ARRAY) {
		    f->g_usig = newsig (T_ARRAY, f->g_usig->g_usig);
		    f = f->g_usig;
		    f->g_lb = intnode (1);
		    f->g_ub = NULL;
		}
		g = h;	/* set g to the newly created signature */

		/* if L side is INDEX, combine with this SLICE */
		if (l->e_opr == O_INDEX) {
		    ixslice (l);	/* turn INDEX tree into SLICE list */
		    a = e->e_r;		/* save our own list */
		    e->e_l = l->e_l;	/* set our L to underlying array */
		    e->e_r = l->e_r;	/* start with list from INDEX node */
		    r = e->e_r;
		    while (r->e_r != NULL && r->e_r->e_l->e_l == NULL)
			r = r->e_r;	/* find last explicit index */
		    r->e_r = a;		/* append our own list of indices */
		}
	    }
	    return e->e_sig = g;
    }

    /* process remaining node types bottom-up: process children first */
    if (l)
	g = attest (l);
    if (r)
	g = attest (r);

    /* now finish the present node, knowing the children are done.
     * set g to the result signature.  not all nodes require processing,
     * in which case the signature g is inherited or void. */
    switch (e->e_opr) {
	
	/********************  statements  ********************/

	case O_IN:
	    /* most of the work has been done in processing child nodes */
	    /* now we just need to take a global view */
	    instmt (e);		/* combine op classes etc. */
	    g = void_sig;
	    break;

	case O_SCHED:
	    if (!r)
		g = void_sig;
	    else if (!ORDERED (g))
		EFATAL (r, "`by' expression must be ordered type");
	    break;

	case O_SYNC:
	    if (!r)
		g = void_sig;
	    else if (g->g_type != T_BOOL)
		EFATAL (r, "sync expr must be boolean expression");
	    break;

	case O_RECEIVE:		
	    g = void_sig;
	    h = l->e_r->e_sig;
	    if (h->g_type != T_OP && h->g_type != T_OCAP)  /* earlier error */
		break;
	    if (l->e_l != NULL) {		/* if receive by name */
		ASSERT (l->e_l->e_opr == O_SYM);
		s = l->e_l->e_sym;
		ASSERT (s->s_kind == K_OP);
		o = s->s_op;
		if (o->o_impl == I_UNIMPL || o->o_impl == I_CAP)
		    o->o_impl = I_IN;
		else if (o->o_impl != I_IN)
		    err (e->e_locn,"duplicate implementation of %s",s->s_name);
	    }
	    m = eproto (l->e_r) -> p_param;
	    if (m->m_sig->g_type != T_VOID)
		EFATAL (e, "cannot receive an op that returns a value");
	    m = m->m_next;
	    while (r != NULL && m != NULL) {
		ASSERT (r->e_opr == O_LIST);
		if (m->m_pass == O_VAR || m->m_pass == O_RES)
		    EFATAL (e,
			"cannot receive an op having a var or res param");
		else if (!compat (r->e_l->e_sig, m->m_sig, C_ASSIGN)) {
		    sprintf (buf,
			"receive: argument %d is incompatible", m->m_seq);
		    EFATAL (e, buf);
		}
		r = r->e_r;
		m = m->m_next;
	    }
	    if (r != NULL || m != NULL)
		EFATAL (l, "receive: wrong number of arguments");
	    break;

	case O_REPLY:
	    if (lfirst (opstack) != NULL) {	/* if in an op */
		o = * (Opptr *) lfirst (opstack);
		o->o_sepctx = TRUE;		/* reply forces separate ctx */
	    }
	    break;

	case O_DESTROY:		
	    g = l->e_sig;
	    t = g->g_type;
	    if (t != T_RCAP && t != T_VCAP && t != T_NLIT
		    && usig (g) -> g_type != T_OCAP)
		EFATAL (e, "destroy object is not a capability");
	    g = void_sig;
	    break;
	
	case O_COSEL:		/* l: QUANT; r: stmt */
	    if (r->e_opr == O_ASSIGN)
		r = r->e_r;
	    if (r->e_opr != O_CALL && r->e_opr != O_SEND)
		EFATAL (e, "invalid co-invocation");
	    else if (r->e_opr == O_SEND) {
	        /* don't sem optimize op that is sent to w/i co. */
		/* (call is handled elsewhere) */
	        ckcfop (r->e_l);
	    }
	    break;
	
	/********************  types  ********************/

	case O_VARATT:
	    if (l != NULL)
		g = l->e_sig;
	    else
		g = r->e_sig;
	    break;

	case O_ANY:		
	    g = any_sig;
	    break;

	case O_VOID:
	case O_VM:
	    g = void_sig;
	    break;

	case O_BOOL:		
	    g = bool_sig;
	    break;

	case O_CHAR:		
	    g = char_sig;
	    break;

	case O_INT:		
	    g = int_sig;
	    break;

	case O_REAL:		
	    g = real_sig;
	    break;

	case O_FFILE:
	    g = file_sig;
	    break;

	case O_PTR:		
	    g = newsig (T_PTR, l->e_sig);
	    break;

	case O_ARRAY:
	    g = newsig (T_ARRAY, r->e_sig);
	    g->g_lb = l->e_l;
	    g->g_ub = l->e_r;
	    break;

	case O_BOUNDS:
	    if (l == NULL) {
		e->e_l = intnode (1);
		/* language specifies that a single declared bound be an int */
		if (r->e_opr == O_ASTER)
		    r->e_sig = int_sig;
		else if (r->e_sig->g_type != T_INT)
		    nfix (r, "single bound is not integer");
		g = int_sig;
	    } else if (r->e_opr == O_ASTER) {
		if (l->e_opr == O_ASTER)
		    nfix (l, "both bounds are `*'");
		g = r->e_sig = l->e_sig;
	    } else {
		if (l->e_opr == O_ASTER)
		    l->e_sig = r->e_sig;
		else if (!compat (l->e_sig, r->e_sig, C_EXACT))
		    EFATAL (e, "incompatible bounds");
		g = r->e_sig;
	    }
	    if (!ORDERED (g) || g->g_type == T_REAL) {
		nfix (r, "illegal bounds type");
		e->e_l = r;
	    }
	    break;

	case O_NEWOP:
	    while (l->e_opr == O_ARRAY)
		l = l->e_r;
	    if (l->e_opr == O_TYPENAME)
		s = l->e_l->e_sym->s_next;
	    else {
		ASSERT (l->e_opr == O_CAP);
		s = l->e_l->e_l->e_l->e_l->e_l->e_sym->s_prev;
		}
	    ASSERT (s->s_kind == K_OP);
	    o = s->s_op;
	    if (o == NULL) {
		o = newop (s, SG_RESOURCE);	/* always resource level */
		*o->o_asig = *o->o_usig = *l->e_sig->g_usig;
		}
	    o->o_madecap = TRUE;
	    if (o->o_impl == I_UNIMPL || o->o_impl == I_DCL)
		o->o_impl = I_CAP;
	    break;

	case O_CAP:		
	    if (l->e_opr == O_VM) {		/* dispose of "cap vm" case */
		g = vcap_sig;
		break;
	    }
	    g = l->e_sig;
	    if (!g) {
		EFATAL (e, "illegal object of capability");
		g = vcap_sig;
		break;
	    }
	    while (g->g_type == T_ARRAY)
		g = g->g_usig;
	    t = g->g_type;

	    if (l->e_opr == O_PROTO) {		/* if defining a new op type */
		s = l->e_l->e_l->e_l->e_l->e_sym->s_prev;
		protoname (g->g_proto, s);
		o = newop (s, seg);		/* make Op entry */
		o->o_impl = I_DCL;
		*o->o_asig = *o->o_usig = *g;
		g = newsig (T_OCAP, g);
	    } else if (t == T_OP) {		/* if opcap */
		g = newsig (T_OCAP, g);
	    } else if (t == T_RES) {		/* if rescap */
		g = newsig (T_RCAP, g);
	    } else {
		EFATAL (e, "illegal object of capability");
		g = vcap_sig;
	    }
	    break;

	case O_STRING:		
	    g = newsig (T_STRING, NULLSIG);
	    if (l->e_opr != O_ASTER && l->e_sig->g_type != T_INT)
		nfix (l, "maxlength is not an integer");
	    g->g_ub = l;
	    break;

	case O_REC:		
	    ASSERT (l->e_opr == O_LIST);
	    ASSERT (l->e_l->e_opr == O_FLDDCL);
	    ASSERT (l->e_l->e_l->e_opr == O_SUBS);
	    ASSERT (l->e_l->e_l->e_l->e_opr == O_SYM);
	    g = newsig (T_REC, NULLSIG);
	    g->g_sym = l->e_l->e_l->e_l->e_sym;
	    g->g_rec = NEW (Recdata);
	    *g->g_rec = zrec;
	    sprintf (buf, "rs%d", ++nrecs);	/* name the typedef now */
	    g->g_rec->k_tdef = rsalloc (buf);	/* and save for use later */
	    break;

	/********************  ops  ********************/
	
	case O_PROTO:
	    ASSERT (l->e_opr == O_LIST);
	    l = l->e_l;
	    ASSERT (l->e_opr == O_PARDCL)
	    ASSERT (l->e_l->e_opr == O_SUBS);
	    ASSERT (l->e_l->e_l->e_opr == O_SYM);
	    nostars (l, l->e_sig);		/* disallow "returns [*]int" */
	    g = newsig (T_OP, l->e_sig);	/* signature */
	    g->g_proto = prototype (e);		/* prototype */
	    g->g_sym = l->e_l->e_l->e_sym;	/* retval heads argument list */
	    s = g->g_sym->s_prev;
	    ASSERT (s != NULL && s->s_kind == K_OP);
	    break;

	case O_SEM:
	    ASSERT (l->e_l->e_l->e_opr == O_SYM);
	    s = l->e_l->e_l->e_sym;
	    ASSERT (s->s_kind == K_OP);
	    s->s_op->o_dclsema = TRUE;
	    g = void_sig;
	    if (r != NULL) {			/* if initialized */
		h = subsig (l->e_l->e_r, int_sig);
		if (!compat (h, r->e_sig, C_ASSIGN))
		    EFATAL (r, "incompatible semaphore initializer");
	    }
	    break;

	/********************  expressions  ********************/

	case O_LIBCALL:
	    g = presig (e);
	    if (l->e_sym->s_pre == PRE_setpriority && lfirst(opstack) != NULL) {
		o = * (Opptr *) lfirst (opstack);
		o->o_sepctx = TRUE;	/* setpriority() forces sep context */
	    }
	    break;

	case O_NEW:		
	    /*
	     * If underlying type is an optype name, mutate into O_NEWOP node.
	     * (This is indicated by a TYPENAME op with non-null right member.)
	     */
	    a = l;
	    while (a->e_opr == O_ARRAY)		/* get past arrays */
		a = a->e_r;
	    if (a->e_opr == O_TYPENAME && a->e_r != NULL) {
		e->e_opr = O_NEWOP;
		g = attest (e);
	    } else
		g = newsig (T_PTR, l->e_sig);	/* not opname; type is PTR */
	    break;

	case O_LOW:		
	case O_HIGH:		
	    g = l->e_sig;
	    if (!ORDERED (g))
		EFATAL (e, "low/high: not an ordered type");
	    break;
	
	case O_NOT:		
	    g = l->e_sig;
	    t = g->g_type;
	    if (t != T_BOOL && t != T_INT)
		g = nfix (e, "NOT operand is not bool or int");
	    break;

	case O_POS:		
	case O_NEG:		
	    g = l->e_sig;
	    t = g->g_type;
	    if (t != T_INT && t != T_REAL)
		g = nfix (e, "unary +/- operand is not simple numeric");
	    break;

	case O_PREINC:		
	case O_PREDEC:		
	case O_POSTINC:		
	case O_POSTDEC:		
	    g = l->e_sig;
	    if (!ORDERED (g) && (g->g_type!=T_PTR || g->g_usig->g_type==T_ANY))
		g = nfix (e, "increment/decrement: not an ordered type");
	    break;

	case O_ADD:		
	    if (r->e_sig->g_type == T_PTR && r->e_sig->g_usig->g_type != T_ANY
		    && l->e_sig->g_type == T_INT) {
		g = r->e_sig;
		break;
	    }
	    /*NOBREAK*/
	case O_SUB:		
	    if (l->e_sig->g_type == T_PTR) {
		if (l->e_sig->g_usig->g_type!=T_ANY && r->e_sig->g_type==T_INT)
		    g = l->e_sig;
		else if (e->e_opr == O_SUB && compat(l->e_sig,r->e_sig,C_EXACT))
		    g = int_sig;
		else
		    g = nfix (e, "incompatible pointer subtraction");
		break;
	    }
	    /*NOBREAK*/
	case O_MUL:		
	case O_DIV:		
	case O_MOD:		
	case O_REMDR:		
	    /* cast either side to real if necessary for agreement */
	    g = ncast (e->e_l, &e->e_r);
	    g = ncast (e->e_r, &e->e_l);
	    if (!g)
		g = nfix (e, "non-numeric arithmetic operand");
	    break;

	case O_EXPON:		
	    /* cast left side to real if exponent is real */
	    g = ncast (e->e_r, &e->e_l);
	    /* process real**int without casting */
	    if (l->e_sig->g_type == T_REAL && r->e_sig->g_type == T_INT)
		g = real_sig;
	    if (!g)
		g = nfix (e, "non-numeric exponentiation operand");
	    break;

	case O_AND:		
	case O_OR:		
	case O_XOR:		
	    g = l->e_sig;
	    h = r->e_sig;
	    t = g->g_type;
	    if (t != h->g_type || (t != T_BOOL && t != T_INT))
		g = nfix (e, "illegal or mismatched and/or/xor operands");
	    break;

	case O_RSHIFT:		
	case O_LSHIFT:		
	    g = l->e_sig;
	    h = r->e_sig;
	    if (g->g_type != T_INT || h->g_type != T_INT)
		g = nfix (e, "non-integer shift operand");
	    break;

	case O_GE:		
	case O_LE:		
	case O_GT:		
	case O_LT:		
	    if (!ORDERED (l->e_sig) && l->e_sig->g_type != T_STRING) {
		EFATAL (e, "illegal comparison for unordered type");
		g = bool_sig;
		break;
	    }
	    /*NOBREAK*/
	case O_EQ:		
	case O_NE:		
	    /* cast either side to real if necessary for agreement */
	    ncast (e->e_l, &e->e_r);
	    ncast (e->e_r, &e->e_l);
	    g = bool_sig;	/* result of comparison is boolean */
	    if (!compat (l->e_sig, r->e_sig, C_COMPARE)) {
		EFATAL (e, "incompatible comparison");
		e->e_l = e->e_r = intnode (1);
	    }
	    t = l->e_sig->g_type;
	    if (t == T_ARRAY || t == T_REC) {
		EFATAL (e, "illegal type for comparison");
		e->e_l = e->e_r = intnode (1);
	    }
	    if (r->e_opr == O_NLIT)
		r->e_sig = l->e_sig;
	    else if (l->e_opr == O_NLIT)
		l->e_sig = r->e_sig;
	    break;

	case O_CONCAT:		
	    t = l->e_sig->g_type;
	    u = r->e_sig->g_type;
	    if((t != T_STRING && t != T_CHAR) || (u != T_STRING && u != T_CHAR))
		EFATAL (e, "non-string concatenation");
	    g = string_sig;
	    break;

	case O_ADDR:		
	    g = newsig (T_PTR, l->e_sig);
	    break;

	case O_PTRDREF:		
	    if (l->e_sig->g_type == T_PTR) {
		g = l->e_sig->g_usig;
		if (g->g_type == T_ANY)
		    nfix (e, "cannot dereference a `ptr any'");
	    } else
		g = nfix (e, "dereference object is not a pointer");
	    break;

	case O_CAST:
	    g = l->e_sig;
	    if (g->g_type == T_REC) {	/* if cast to record type */
		e->e_opr = O_RECCONS;	/* change into record constructor */
		return attest (e);	/* and reprocess */
	    }
	    if (r == NULL || (r->e_opr == O_LIST && r->e_r != NULL)) {
		EFATAL (e, "illegal conversion");
		break;
	    }
	    if (r->e_opr == O_LIST)	/* if list (from cast to named type) */
		e->e_r = r = r->e_l;	/* change list of 1 to simple node */
	    if (!CONVERTIBLE (g) || !CONVERTIBLE (r->e_sig))
		EFATAL (e, "illegal conversion");
	    break;

	case O_RECCONS:
	    ASSERT (l->e_opr == O_SYM && l->e_sym->s_kind == K_TYPE);
	    g = l->e_sig;
	    ASSERT (g->g_type == T_REC);
	    s = g->g_sym;
	    n = 1;
	    while (r != NULL && s != NULL) {
		ASSERT (s->s_kind == K_VAR && s->s_var->v_vty == V_RMBR);
		ASSERT (r->e_opr == O_LIST);
		if (!compat (s->s_var->v_sig, r->e_l->e_sig, C_ASSIGN)){
		    sprintf (buf,
			"item %d of rec constructor is incompatible", n);
		    EFATAL (e, buf);
		    return e->e_sig = nfix (e, NULLSTR);
		}
		if (r->e_l->e_sig->g_type == T_NLIT)
		    r->e_l->e_sig = s->s_var->v_sig;
		s = s->s_next;
		while (s != NULL && s->s_kind != K_VAR)	
		    s = s->s_next;		/* skip anonymous caps &c */
		r = r->e_r;
		n++;
	    }
	    if (r != NULL || s != NULL)
		EFATAL (e, "wrong number of items for rec constructor");
	    break;

	case O_ARCONS:
	    g = l->e_l->e_sig;			/* signature of first entry */
	    if (g->g_type == T_NLIT)
		EFATAL (l, "ambigous: don't know what kind of null/noop");

	    /* everything must agree with first sig */
	    for (r = l->e_r, n = 2; r != NULL; r = r->e_r, n++) {
		if (!compat (g, r->e_l->e_sig, C_EXACT)) {
		    sprintf (buf,"element %d of constructor is incompatible",n);
		    nfix (r->e_l, buf);
		    break;
		}
	    }
	    g = newsig (T_ARRAY, g);		/* signature of array */
	    break;

	case O_CLONE:
	    /*
	     * The signature of a clone expr should really be "array of xxx",
	     * but a clone always appears in an array constructor, and it's
	     * more convenient to let THAT apply the "array" attribute.
	     */
	    if (l->e_sig->g_type != T_INT)
		nfix (l, "repetition count is not an integer");
	    g = r->e_sig;
	    break;

	case O_RANGE:
	    if (r->e_opr == O_ASTER) {
		if (l == NULL || l->e_opr == O_ASTER)
		    r->e_sig = int_sig;
		else
		    r->e_sig = l->e_sig;
	    }
	    if (l != NULL) {
		if (l->e_opr == O_ASTER)
		    l->e_sig = r->e_sig;
		else if (!compat (l->e_sig, r->e_sig, C_EXACT))
		    EFATAL (e, "incompatible slice indices");
	    }
	    g = r->e_sig;
	    break;
	
	case O_CREATE:
	    if (l->e_opr == O_ILIT) {		/* if earlier error */
		nfix (e, NULLSTR);
		break;
	    }
	    ASSERT (l->e_l->e_opr == O_SYM);
	    s = l->e_l->e_sym;
	    if (l->e_l->e_sig->g_type != T_RES) {
		err (l->e_l->e_locn, "not a resource: %s", s->s_name);
		g = nfix (e, NULLSTR);
		break;
	    }
	    ckargs (l, s->s_res->r_sig->g_proto);
	    g = newsig (T_RCAP, s->s_res->r_sig);
	    if (r != NULL && r->e_sig->g_type != T_VCAP) {
		EFATAL (r, "location is not a vm cap");
		e->e_r = NULL;
	    }
	    break;

	case O_CREVM:
	    if (l->e_r != NULL)
		EFATAL (e, "no arguments allowed in `create vm()'");
	    if (r != NULL && r->e_sig->g_type != T_INT
		&& r->e_sig->g_type != T_STRING)
		    nfix (r, "vm location is neither integer nor string");
	    g = vcap_sig;
	    break;

	case O_AUG_ADD:		
	case O_AUG_SUB:		
	    if (l->e_sig->g_type == T_PTR && l->e_sig->g_usig->g_type != T_ANY
		    && r->e_sig->g_type == T_INT) {
		g = l->e_sig;
		break;
	    }
	    /*NOBREAK*/
	case O_AUG_MUL:		
	case O_AUG_DIV:		
	case O_AUG_REMDR:		
	case O_AUG_EXPON:		
	    g = ncast (e->e_l, &e->e_r);
	    if (!g)
		g = nfix (e, "non-numeric operand or incompatible assignment");
	    break;

	case O_AUG_AND:		
	case O_AUG_OR:		
	    g = l->e_sig;
	    h = r->e_sig;
	    t = g->g_type;
	    if (t != h->g_type || (t != T_BOOL && t != T_INT))
		g = nfix (e, "illegal or mismatched and/or/xor operands");
	    break;

	case O_AUG_RSHIFT:		
	case O_AUG_LSHIFT:		
	    if (l->e_sig->g_type != T_INT || r->e_sig->g_type != T_INT)
		g = nfix (e, "non-integer shift operand");
	    g = int_sig;
	    break;

	case O_AUG_CONCAT:		
	    t = l->e_sig->g_type;	/* L side must be string */
	    u = r->e_sig->g_type;	/* R side can be string or char */
	    if (t != T_STRING || (u != T_STRING && u != T_CHAR))
		EFATAL (e, "non-string concatenation");
	    g = string_sig;
	    break;
	
	case O_ASSIGN:		
	    if (compat (l->e_sig, r->e_sig, C_ASSIGN)) {
		g = l->e_sig;
		if (r->e_opr == O_NLIT)
		    r->e_sig = l->e_sig;
	    } else {
		g = nfix (e, "incompatible assignment");
	    }
	    break;

	case O_SWAP:		
	    if (compat (l->e_sig, r->e_sig, C_ASSIGN)
	    &&  compat (r->e_sig, l->e_sig, C_ASSIGN))
		g = l->e_sig;
	    else
		g = nfix (e, "incompatible swap");
	    break;

	case O_GUARD:
	    if (l->e_sig->g_type != T_BOOL)
		nfix (l, "guard expression is not boolean");
	    g = void_sig;
	    break;

	/********************  miscellaneous leaf nodes  ********************/

	case O_ID:
	    BOOM ("unresolved ID found in attest()", e->e_name);
	    break;

	case O_SYM:
	    s = e->e_sym;
	    g = symsig (s);
	    /* if it's an op, note use of its capability */
	    /* (this is for all refs except O_SEND & O_QMARK; see code above) */
	    if (s->s_kind == K_OP) {
		o = s->s_op;
		o->o_nosema = TRUE;
		o->o_madecap = TRUE;
		if (o->o_impl == I_UNIMPL)
		    o->o_impl = I_CAP;
	    }
	    break;

	case O_ASTER:
	    g = any_sig;
	    break;

	case O_ILIT:
	case O_RLIT:
	case O_BLIT:
	case O_CLIT:
	case O_SLIT:
	case O_NLIT:
	case O_FLIT:
	    g = e->e_sig;	/* signature was set when constructed */
	    break;
    }

    return e->e_sig = g;
}



/*  ncast(l,&r) -- check signatures and cast right operand for numeric op.
 *
 *  Checks both operands for a numeric operation.
 *  If left is real and right is int, casts the right operand.
 *
 *  Returns the real or int signature, if both operands now agree, else NULL.
 */
static Sigptr
ncast (l, pr)
Nodeptr l, *pr;
{
    Type lt = l->e_sig->g_type;
    Type rt = (*pr)->e_sig->g_type;

    if (lt == T_INT) {
	if (rt == T_INT)
	    return int_sig;			/* both int => result int */
	else
	    return NULL;			/* int . real => illegal */
    }
    if (lt == T_REAL) {
	if (rt == T_REAL)
	    return real_sig;			/* both real => result real */
	else if (rt == T_INT) {
	    *pr = bnode (O_CAST, newnode (O_REAL), *pr);
	    (*pr)->e_sig = real_sig;
	    return real_sig;			/* real . int => coerce real */
	}
    }
    return NULL;				/* else => illegal */
}



/*  nfix(e,msg) -- issue error message (if not null) and replace node.
 *
 *  Node e is mutated into an ILIT node with value 1; int_sig is returned.
 */
static Sigptr
nfix (e, msg)
Nodeptr e;
char *msg;
{
    if (msg)
	EFATAL (e, msg);
    e->e_opr = O_ILIT;
    e->e_sig = int_sig;
    e->e_int = 1;
    e->e_r = NULL;
    return int_sig;
}



/*  subsig(list,sig) -- construct a signature from a subscript list  */

static Sigptr
subsig (e, g)
Nodeptr e;
Sigptr g;
{
    if (!e)
	return g;
    ASSERT (e->e_opr == O_LIST);
    ASSERT (e->e_l->e_opr == O_BOUNDS);
    g = newsig (T_ARRAY, subsig (e->e_r, g));
    g->g_lb = e->e_l->e_l;
    g->g_ub = e->e_l->e_r;
    return g;
}



/*  parlist (g,r,kwd) -- process param list r for op with signature g  */

static void
parlist (g, r, kwd)
Sigptr g;
Nodeptr r;
char *kwd;
{
    Varptr v;
    Variety y;
    Parptr m;
    int l;

    if (g->g_type == T_OCAP)
	g = g->g_usig;
    else if (g->g_type != T_OP)
	return;				/* error diagnosed earlier */
    m = g->g_proto->p_param;
    l = r->e_locn;
    while (r != NULL && m != NULL) {
	ASSERT (r->e_opr == O_LIST);
	ASSERT (r->e_l != NULL && r->e_l->e_opr == O_SYM);
	if (m->m_sig->g_type == T_VOID) {
	    if (r->e_l->e_sym->s_name != NULL)
		EFATAL (r->e_l, "unexpected return parameter");
	} else if (r->e_l->e_sym->s_name == NULL) {
	    if (m == g->g_proto->p_param)
		EFATAL (r->e_l, "missing return parameter");
	    else
		err (E_WARN + r->e_l->e_locn,
		    "unnamed parameter is inaccessible", NULLSTR);
	}
	y = (m->m_pass == O_REF) ? V_REFNCE : V_PARAM;
	v = newvar (r->e_l->e_sym, y, O_PARDCL, m->m_sig);
	v->v_seq = m->m_seq;
	r = r->e_r;
	m = m->m_next;
    }
    if (r != NULL || m != NULL)
	err (l, "%s: wrong number of arguments", kwd);

    while (r != NULL) {			/* define extras as ints */
	ASSERT (r->e_opr == O_LIST);
	ASSERT (r->e_l != NULL && r->e_l->e_opr == O_SYM);
	v = newvar (r->e_l->e_sym, V_PARAM, O_PARDCL, int_sig);
	r = r->e_r;
    }
}



/*  rparms (r) -- declare resource parameter variables for resource r  */

static void
rparms (r)
Resptr r;
{
    Symptr f, s;
    Varptr v;
    Kind k;
    int n;

    s = r->r_parm;
    ASSERT (s->s_kind == K_BLOCK);
    f = s->s_child;			/* first formal in prototype */
    s = s->s_next;			/* first sym for local var */
    n = 1;
    while (f != NULL) {			/* make a var for each param */
	k = f->s_kind;
	/* check all param kinds; errors are caught later */
	if (k == K_FVAL || k == K_FVAR || k == K_FRES || k == K_FREF) {
	    v = newvar (s, V_RPARAM, O_PARDCL, f->s_sig);
	    v->v_seq = n++;
	    s = s->s_next;
	}
	f = f->s_next;
    }
}



/*  ckargs (e, p) -- check arglist of invocation e against prototype p  */

static void
ckargs (e, p)
Nodeptr e;
Proptr p;
{
    Bool c;
    Nodeptr a = e->e_r;
    Parptr m = p->p_param->m_next;
    int n = 0;
    char *name;
    char buf[100];

    if (e->e_l->e_opr == O_SYM)
	name = e->e_l->e_sym->s_name;
    else
	name = "<expr>";

    while (m != NULL && a != NULL) {
	n++;
	if (m->m_pass == O_VAL)
	    c = compat (m->m_sig, a->e_l->e_sig, C_ASSIGN);
	else if (m->m_pass == O_RES)
	    c = compat (a->e_l->e_sig, m->m_sig, C_ASSIGN);
	else if (m->m_pass == O_VAR)
	    c = compat (m->m_sig, a->e_l->e_sig, C_ASSIGN)
		&& compat (a->e_l->e_sig, m->m_sig, C_ASSIGN);
	else {
	    ASSERT (m->m_pass == O_REF);
	    c = compat (a->e_l->e_sig, m->m_sig, C_EXACT);
	}
	if (!c) {
	    sprintf (buf, "%%s(): argument %d is incompatible", n);
	    err (a->e_l->e_locn, buf, name);
	}
	a = a->e_r;
	m = m->m_next;
    }

    if (m == NULL && a == NULL)
	return;

    if (m != NULL)
	err (e->e_locn, "%s(): not enough arguments", name);
    else
	err (e->e_locn, "%s(): too many arguments", name);
}



/*  nostars (e, g) -- diagnose use of '*' bounds by node e in signature g  */

static void
nostars (e, g) 
Nodeptr e;
Sigptr g;
{
    for (; g->g_type == T_ARRAY; g = g->g_usig)
	if (g != NULL && ((g->g_lb != NULL && g->g_lb->e_opr == O_ASTER)
		|| (g->g_ub != NULL && g->g_ub->e_opr == O_ASTER))) {
	    EFATAL (e, "illegal context for array bound `*'");
	    return;
	}
    if (g->g_type == T_STRING && g->g_ub != NULL && g->g_ub->e_opr == O_ASTER)
	EFATAL (e, "illegal context for string bound `*'");
}



/*  ixattest (e, try) -- process O_INDEX node
 *
 *  try is true if should try to do semaphore optimization
 *  (i.e., don't call attest directly for l since attest's
 *   O_SYM case sets o_nosema.)
 */

static Sigptr
ixattest (e, try)
Nodeptr e;
int try;
{
    Nodeptr l, r;
    Sigptr g, h;

    l = e->e_l;
    r = e->e_r;

    if (l->e_opr == O_INDEX)
	ixattest (l, try);
    else if (try) {
	ASSERT (l->e_opr == O_SYM); /* checked by subop before call ixattest */
	l->e_sig = symsig (l->e_sym);	/* just set signat if a SYM */
    }
    else
	attest (l);
    attest (r);

    if (l->e_sig->g_type == T_STRING)
	g = char_sig;
    else if (l->e_sig->g_type == T_ARRAY) {
	g = l->e_sig->g_usig;
	if (l->e_sig->g_ub == NULL)
	    h = int_sig;
	else
	    h = l->e_sig->g_ub->e_sig;
	if (r->e_opr == O_ASTER)
	    r->e_sig = h;
	else if (!compat (h, r->e_sig, C_EXACT))
	    EFATAL (e, "wrong subscript type");
    } else
	g = nfix (e, "subscripted object is not an array");

    return e->e_sig = g;
}



/*  ixslice (e) -- turn index node into slice node.
 *
 *  When an index tree does not go all the way down to specifying an
 *  individual element, it must be turned into a slice operation.
 *  This must be done now, in the attest() phase, for purposes of
 *  lvalue checking.
 */

static void
ixslice (e)
Nodeptr e;
{
    Nodeptr l, r, a;
    int n;

    r = NULLNODE;
    a = newnode (O_ASTER);
    a->e_locn = e->e_locn;
    for (n = ndim (e->e_sig); n--; )
	r = bnode (O_LIST, bnode (O_RANGE, a, a), r);
    for (l = e; l->e_opr == O_INDEX; l = l->e_l)
	r = bnode (O_LIST, bnode (O_RANGE, NULLNODE, l->e_r), r);
    e->e_opr = O_SLICE;
    e->e_l = l;		/* array being subscripted */
    e->e_r = r;		/* list of indices */
    attest (r);		/* set index signatures */
}

/*  ckcfop (l)
 *
 *  Check whether calling or forwarding or co-sending to an op, not an opcap.
 *  If so, mark op as not optimizable as a semaphore.
 */

static void
ckcfop (l)
Nodeptr l;
{
    Nodeptr a;

    if (l->e_sig->g_type == T_OP) {	/* op, not opcap */
	/* if call or forward or co-sending by name to unsubscripted op, */
	/* mark the op as no sem */
	/* (if call by capability, was marked on cap asgmt */
	if (l->e_opr == O_SYM) {
	    ASSERT (l->e_sym->s_kind == K_OP);
	    l->e_sym->s_op->o_nosema = TRUE;
	}
	else if ((a = subop (l)) &&	/* subscripted op */
		a->e_opr == O_SYM) {	/* redundant with subop's check */
	    ASSERT (a->e_sym->s_kind == K_OP);
	    a->e_sym->s_op->o_nosema = TRUE;
	}
    }
}

/*  subop (l)
 *
 *  checks if l is a subscripted operation,
 *  i.e., a non-empty sequence of O_INDEXs followed by an O_SYM.
 *  if so, return pointer to the O_SYM; if not, return NULL.
 */

Nodeptr
subop (l)
Nodeptr l;
{
    Nodeptr a;
    Bool subbed = FALSE;

    ASSERT (l);
    ASSERT (l->e_opr != O_SYM);

    a = l;
    while (a->e_opr == O_INDEX) {
	subbed = TRUE;
	a = a->e_l;
	ASSERT (a);
    }

    return (subbed && a->e_opr == O_SYM) ? a : NULL;
}
