/*  gexpr.c -- generate code for expressions  */

#include "compiler.h"
#include <ctype.h>

/* "../arch.h" must define real min/max if <float.h> is unavailable  */

#include "../arch.h"
#ifndef LOW_REAL
#include <float.h>
#define LOW_REAL DBL_MIN
#define HIGH_REAL DBL_MAX
#endif

static void gassign	PARAMS ((Nodeptr e));
static void gcompare	PARAMS ((Nodeptr e, char *test));
static Nodeptr gindex	PARAMS ((Nodeptr e));
static void capcmp	PARAMS ((Nodeptr l, Nodeptr r, char *test));
static void catargs	PARAMS ((Nodeptr e));
static void strlit	PARAMS ((Nodeptr e));
static void gcast	PARAMS ((Nodeptr e));
static void gslice	PARAMS ((Nodeptr source, Nodeptr slice));
static void augmop	PARAMS ((Nodeptr e, Operator o));


/*  gexpr (e) -- generate code for expression e
 *
 *  note that this may be used as an lvalue under some circumstances.
 *
 *  Expressions involving binary operators are parenthesized to insure
 *  correct precedence.  Prefix expressions are parenthesized if the
 *  result is a type that can be subscripted or dereferenced.
 *
 *  The "value" of a String or Array expression is a String* or Array*;
 *  i.e. its address.
 */
void
gexpr (e)
Nodeptr e;
{
    Nodeptr l, r, a, b;
    Sigptr g;
    Symptr s;
    Classptr cl;
    Type t;
    Operator o;
    Opptr op;
    Proptr p;
    int i, n, ndims;
    char *fname, *tname, *rname, *lname, *oname, *pname, *gname, c;
    char buf[100];
    static int rtcount = 0;	/* record constructor temps */
    Bool quickcall;

    ASSERT (e != NULL);
    l = LNODE (e);
    r = RNODE (e);
    g = e->e_sig;
    if (g != NULL)
	t = g->g_type;
    else
	t = T_VOID;

    switch (e->e_opr) {

	/* constants and literals */

	case O_SLIT:
	    strlit (e);
	    break;
	case O_BLIT:
	case O_CLIT:
	case O_ILIT:
	    cprintf ("%d", e->e_int);
	    break;
	case O_RLIT:
	    sprintf (buf, " %.15e", *e->e_rptr);
	    cprintf ("%s", buf);
	    break;
	case O_FLIT:
	    switch (e->e_int) {
		case STDIN:   cprintf ("stdin");   break;
		case STDOUT:  cprintf ("stdout");  break;
		case STDERR:  cprintf ("stderr");  break;
		default:      BOOM ("bad file const", "");
	    }
	    break;
	case O_NLIT:
	    n = (e->e_int == NULL_SEQN);	/* else is NOOP */
	    switch (t) {
		case T_FILE:
		    cprintf ("(File)%s", n ? "NULL_FILE" : "NOOP_FILE");
		    break;
		case T_PTR:
		    if (n)
			cprintf ("(%g)0", g);
		    else
			EFATAL (e, "can't use `noop' in pointer context");
		    break;
		case T_OP:
		case T_OCAP:
		    cprintf (n ? "sr_nu_ocap" : "sr_no_ocap");
		    break;
		case T_RES:
		case T_RCAP:
		    tname = alctrans ();
		    rname = g->g_usig->g_sym->s_gname;
		    cprintf
			("*(%g*)(%s=sr_literal_rcap(sizeof(Rcap_%s),C_%s,&%s))",
			g, tname, rname, rname,
			(n ? "sr_nu_ocap" : "sr_no_ocap"));
		    break;
		case T_VCAP:
		    cprintf (n ? "NULL_VM" : "NOOP_VM");
		    break;
		case T_NLIT:
		    /* result never used, as in "noop;" stmt */
		    /* just gen something legal that won't work in an expr */
		    cprintf (n ? "\"null\"" : "\"noop\"");
		    break;
		default:
		    BOOM ("bad type for null/noop generation", typetos (t));
		    break;
	    }
	    break;

	/* function-like operators */
	case O_CAST:
	    gcast (e);
	    break;
	case O_NEWOP:
	    cl = digop (g) -> o_class;
	    if (g->g_type == T_OCAP) {
		cprintf ("sr_new_op(%t,%C)", e, cl);
	    } else {
		ASSERT (g->g_type == T_ARRAY);
		cprintf (
		    "(Array*)sr_make_inops(sr_init_array(%t,(Array*)-1,", e);
		garray (e, g);
		cprintf ("),%C,%d,%d)", cl, ndim (g), DYNAMIC_OP);
	    }
	    break;
	case O_NEW:
	    g = l->e_sig;	/* signature of type being allocated */
	    switch (g->g_type) {
		case T_STRING:
		    pname = alctemp (T_PTR);
		    tname = alctemp (T_INT);
		    cprintf ("(%s=%e+STRING_OVH,", tname,
			(l->e_opr == O_STRING) ? l->e_l : g->g_ub);
		    cprintf ("%s=sr_new(%t,%s),", pname, e, tname);
		    cprintf ("((String*)%s)->size=%s,", pname, tname);
		    cprintf ("%s)", pname);
		    freetemp (T_INT, tname);
		    freetemp (T_PTR, pname);
		    break;
		case T_ARRAY:
		    cprintf ("(Ptr)sr_init_array(%t,(Array*)-1,", e);
		    garray (e, g);
		    cprintf (")");
		    break;
		case T_REC:
		    if (g->g_rec->k_init) {
			pname = alctemp (T_PTR);
			cprintf ("(%s=sr_new(%t,sizeof(%g)),", pname, e, g);
			cprintf ("*(%g*)%s=%s,", g, pname, g->g_rec->k_init);
			cprintf ("%s)", pname);
			freetemp (T_PTR, pname);
		    } else
			cprintf ("sr_new(%t,sizeof(%g))", e, g);
		    break;
		default:
		    ASSERT (!DESCRIBED (g));
		    cprintf ("sr_new(%t,sizeof(%g))", e, g);
		    break;
	    }
	    break;
	case O_LOW:
	    switch (t) {
		case T_BOOL:
		case T_CHAR:
		case T_ENUM:
		    cprintf ("0");
		    break;
		case T_INT:
		    /* done this way because "-2147483648" is illegal C */
		    /* (2147483648 is too big for a legal constant) */
		    cprintf ("(-2147483647-1)");	
		    break;
		case T_REAL:
		    sprintf (buf, "%.20e", LOW_REAL);
		    cprintf (buf);
		    break;
	    }
	    break;
	case O_HIGH:
	    switch (t) {
		case T_BOOL:
		    cprintf ("1");
		    break;
		case T_CHAR:
		    cprintf ("255");
		    break;
		case T_INT:
		    cprintf ("2147483647");
		    break;
		case T_ENUM:
		    n = 0;
		    s = g->g_sym; 
		    g = s->s_sig;
		    for (s = s->s_next;
			    s != NULL && s->s_kind == K_ELIT && s->s_sig == g;
			    s = s->s_next)
			n++;
		    cprintf ("%d", n);
		    break;
		case T_REAL:
		    sprintf (buf, "%.20e", HIGH_REAL);
		    cprintf (buf);
		    break;
	    }
	    break;

	/* invocations */

	case O_QMARK:
	    if (l->e_opr == O_SYM && l->e_sym->s_kind == K_OP
	    && l->e_sym->s_op->o_impl == I_SEM)
		cprintf ("(*(Int*)%n)", l->e_sym);	/* semaphore */
	    else if (
		l->e_opr != O_SYM &&
		(a = subop (l)) &&
		a->e_opr == O_SYM && /* redundant with subop's check */
		a->e_sym->s_kind == K_OP &&
		a->e_sym->s_op->o_impl == I_SEM)
		cprintf ("(*(Int*)%e)", l);		/* sem array elt */
	    else
		cprintf ("sr_query_iop(%t,%e)", e, l);	/* other op */
	    break;

	case O_LIBCALL:
	    pregen (e);
	    break;

	case O_RECCONS:
	    sprintf (buf, "rt%d", ++rtcount);	/* invent a new temporary */
	    cprintf ("%8%g %s;\n", g, buf);	/* declare it */
	    l = vbmnode (buf, g);		/* make a node for it */
	    s = g->g_sym;
	    cprintf ("(");
	    if (g->g_rec->k_init)
		cprintf ("%s=%s,", buf, g->g_rec->k_init);
	    while (r != NULL && s != NULL) {
		a = bnode (O_ASSIGN, bnode(O_FIELD,l,newnode(O_SYM)), r->e_l);
		a->e_l->e_r->e_sym = s;
		a->e_sig = a->e_l->e_sig = a->e_l->e_r->e_sig = s->s_var->v_sig;
		cprintf ("%e,", a);
		s = s->s_next;
		while (s != NULL && s->s_kind != K_VAR)
		    s = s->s_next;
		r = r->e_r;
	    }
	    ASSERT (r == NULL && s == NULL);
	    cprintf ("%s)", buf);		/* return its value */
	    break;

	case O_CALL:
	case O_SEND:
	case O_FORWARD:
	    o = e->e_opr;
	    p = eproto (e);

	    if (l->e_opr == O_SYM && l->e_sym->s_kind == K_OP &&
		l->e_sym->s_op->o_impl == I_SEM) {
		ASSERT (o == O_SEND);
		ASSERT (r == NULL);
		cprintf ("sr_V(%t,%e)", e, l);	/* `send' to semaphore */
		break;
	    }

	    if (o == O_SEND && 
		l->e_opr != O_SYM &&
		(a = subop (l)) &&
		a->e_opr == O_SYM && /* redundant with subop's check */
		a->e_sym->s_kind == K_OP &&
		a->e_sym->s_op->o_impl == I_SEM) {
		ASSERT (r == NULL);
		cprintf ("sr_V(%t,%e)", e, l);	/* `send' to sem array elt */
		break;
	    }

	    /* check to see if we can do a direct call */
	    quickcall =
		(o == O_CALL) &&			/* if CALL... */
		(l->e_opr == O_SYM) &&			/* ...by name... */
		(!IMPORTED(s=l->e_sym) || s->s_imp->s_res->r_opr==O_GLOBAL) &&
							/* ...local or glb... */
		(s->s_kind == K_OP) &&			/* ...op not cap... */
		(! (op = s->s_op) -> o_sepctx) &&	/* ...no reply &c... */
		(op->o_impl == I_PROC || op->o_impl == I_EXTERNAL) && /* proc */
		(! option_P);				/* ...not pessimizing */

	    fname = NULL;			/* fixed param block name */
	    if (quickcall)
		fname = fixedpb (p);		/* still null if not fixed sz */

	    if (fname != NULL || o != O_CALL)
		tname = alctemp (T_PTR);
	    else
		tname = alctrans ();

	    cprintf ("(%s=", tname);
	    gparblk (fname, p, r, NULLNODE, e->e_opr);

	    if (o != O_FORWARD && !quickcall)
		cprintf (",\n((Invb)%s)->type=%s", tname,
		    (o == O_CALL ? "CALL_IN" : "SEND_IN"));

	    if (!quickcall)
		cprintf (",\n((Invb)%s)->opc=%e", tname, l);

	    if (o == O_FORWARD) {
		if (indepth > 0)
		    sprintf (buf, "pb%d", indepth);
		else
		    strcpy (buf, "pb");
		cprintf (",\n%s=sr_forward(%t,%s,%s)", tname, e, buf, tname);
	    } else if (quickcall) {
		cprintf (",\n(sr_trc_flag?sr_trace(\"CALL\",%t,0):0)", e);
		cprintf (",\nP_%s(rp,rv,%s,!RTS_CALL)", s->s_name, tname);
	    } else
		cprintf (",\n%s=sr_invoke(%t,%s)", tname, e, tname);

	    if (o == O_CALL) {
		gparback (p, r, tname);
		if (g->g_type != T_VOID)
		    if (DESCRIBED (g))
			cprintf (",\n((%s)(%s+((%s*)%s)->o_0))",
			    csig (g), tname, p->p_def, tname);
		    else
			cprintf (",\n((%s*)%s)->_0", p->p_def, tname);
	    }
	    cprintf (")");
	    if (fname != NULL || o != O_CALL)
		freetemp (T_PTR, tname);	/* discard temp after send */
	    break;
	
	case O_REPLY:
	    if (!curproto)
		cprintf ("sr_reply(%t,(Invb)0)", e);
	    else if ((curproto->p_rstr == O_SEND)		/* send restr */
		|| (indepth > 0 && invptr[indepth] == NULL))	/* or sem impl*/
		    err (E_WARN + e->e_locn,
			"fruitless reply (all calls are asynchronous)",NULLSTR);
	    else if (curproto->p_rstr == O_FINAL)
		err (E_WARN + e->e_locn,
		    "reply in final ignored", NULLSTR);
	    else {
		if (indepth > 0)
		    sprintf (buf, "pb%d", indepth);
		else
		    strcpy (buf, "pb");
		cprintf ("%s=(%s*)sr_reply(%t,%s)",buf,curproto->p_def,e,buf);
	    }
	    break;

	/* subscripting */

	case O_FIELD:
	    ASSERT (r->e_opr == O_SYM);
	    t = l->e_sig->g_type;
	    if (t == T_RES || t == T_GLB) {
		gexpr (e->e_r);		/* was resolved earlier */
	    } else if (t == T_REC) {
		ASSERT (r->e_opr == O_SYM);
		if (g->g_type == T_ARRAY && adouble (g))
		    cprintf ("(%g)", g);	/* if was declared funny */
		cprintf ("%e.%N", l, r->e_sym);
	    } else {
		cprintf ("%a->_%s", l, r->e_sym->s_name);
	    }
	    break;
	case O_INDEX:
	    if (DESCRIBED (g))		/* if element type uses descriptors */
		cprintf ("%a", e);
	    else 
		cprintf ("(*%a)", e); 
	    break;
	case O_SLICE:
	    /* 
	     * Extract a slice for use as an rvalue.  (The lvalue case is
	     * handled under O_ASSIGN and in the var/res parameter code.)
	     */
	    tname = alctrans ();
	    if (l->e_sig->g_type == T_STRING) {
		cprintf ("((String*)(%s=sr_sslice(%t,%e,%e,%e)))",
		    tname, e, l, r->e_l->e_l, r->e_l->e_r);
		nreplace (e, "String*", tname);
		break;
	    }
	    cprintf ("((Array*)(%s=", tname);
	    gslice (NULLNODE, e);
	    cprintf ("))");
	    nreplace (e, "Array*", tname);
	    break;

	case O_ARCONS:

	    /* this is awfully complex... turn into an rts call sometime? */
	    /* would simplify generated code as well as the code below */
	    /* (generated code overflows some C compilers) */
	    ASSERT (l->e_opr == O_LIST);
	    g = l->e_sig;			/* signature of one entry */
	    oname = alctemp (T_INT);		/* size of entry */
	    tname = alctemp (T_INT);		/* number of entries */
	    pname = alctemp (T_PTR);		/* store pointer */
	    rname = alctrans ();		/* pointer to result array */
						/* gname = size of base type */
	    ndims = ndim (e->e_sig);

	    /* calculate total number of items */
	    cprintf ("(%s=", tname);
	    n = 0;
	    for (a = l; a != NULL; a = a->e_r)
		if (a->e_l->e_opr == O_CLONE) {
		    cprintf ("(%E<0?", a->e_l->e_l);	/* n.b. may chg node */
		    cprintf ("sr_runerr(%t,%d,%e):%e)+",
			a->e_l->e_l, E_AREP, a->e_l->e_l, a->e_l->e_l);
		} else
		    n++;
	    cprintf ("%d,\n", n);

	    /* set temp to sizeof(entry) and store sizeof base type in gname */
	    if (DESCRIBED (g)) {
		if (g->g_type == T_ARRAY) {
		    cprintf ("%s=%E->size-%d,\n",
			oname, l->e_l->e_opr == O_CLONE ? l->e_l->e_r : l->e_l,
			sizeof (Array) + (ndims - 1) * sizeof (Dim));
		    gname = alctemp (T_INT);
		    cprintf ("%s=STRIDE(%E,0),\n", 
			gname, l->e_l->e_opr == O_CLONE ? l->e_l->e_r : l->e_l);
		} else {
		    cprintf ("%s=SRALIGN(DSIZE(%E)),\n",
			oname, l->e_l->e_opr == O_CLONE ? l->e_l->e_r : l->e_l);
		    gname = oname;
		}
	    } else {
		cprintf ("%s=sizeof(%g),\n", oname, g);
		gname = oname;
	    }

	    /* allocate array */
	    cprintf ("%s=%d+(%s=(Ptr)sr_init_array(%t,(Array*)0,%s,(Ptr)0,%d,",
		pname, sizeof (Array) + ndims * sizeof (Dim), rname, e, 
		gname, ndims);
	    /* generate dimension for the sr_init_array call */
	    cprintf ("1,%s", tname); /* first dimension */

	    if (ndims > 1) {
		if (l->e_l->e_opr == O_CLONE)
		    b = l->e_l->e_r;
		else
		    b = l->e_l;

		cprintf (",1,%u-%l+1", b, b);	/* do dimension 2 this way */

		/* dims > 2 are done the horrid way */
		for (i = 3; i <= ndims; i++) {
		    cprintf (
		    ",1,((Dim*)(%a+1))[%d-%d].ub-((Dim*)(%a+1))[%d-%d].lb+1",
		    b, ndims, i, b, ndims, i);
		}
	    }
	    cprintf (")),\n");

	    /* fill in the array data */
	    for (a = l; a != NULL; a = a->e_r) {
		r = a->e_l;			/* find data object */
		if (r->e_opr == O_CLONE) {
		    cprintf ("%e==0?0:(", r->e_l);	/* store iff count>0 */
		    r = r->e_r;
		    n = ntrans ();
		}

		if (DESCRIBED (g)) {		/* copy once */
		    if (g->g_type == T_STRING)
			cprintf ("memcpy(%s,%a,%s),", pname, r, oname);
		    else {		/* arrays have different offset */
			once (r, ',');
			cprintf ("memcpy(%s,ADATA(%a),%a->size-%d),",
			    pname, r, r, 
			    sizeof (Array) + ndim (r->e_sig) * sizeof (Dim));
		    }
		}
		else
		    cprintf ("*(%g*)%s=%e, ", g, pname, r);
		if (a->e_l->e_opr == O_CLONE) {	/* clone, or just bump addr */
		    cprintf ("%s=sr_clone(%t,%s,%s,%e)",
			pname, e, pname, oname, a->e_l->e_l);
		    freetrans (n, ',');
		    cprintf (",0)%,");
		} else
		    cprintf ("%s+=%s,\n", pname, oname);
	    }
	    cprintf ("(Array*)%s)", rname);
	    if (gname != oname)
		freetemp (T_INT, gname);
	    freetemp (T_PTR, pname);
	    freetemp (T_INT, tname);
	    freetemp (T_INT, oname);
	    break;

	/* arithmetic */

	case O_POS:
	    cprintf ("%e", l);
	    break;
	case O_NEG:
	    cprintf (" -%e", l);	/* space prevents =- and -- */
	    break;

	case O_PREINC:
	case O_PREDEC:
	    oname = (e->e_opr == O_PREINC ? "++" : "--");
	    if (!lvalue (l))
		err (l->e_locn, "%sexpr: not a variable", oname);
	    if (t != T_PTR)
		cprintf ("(%s%e)", oname, l);
	    else if (!DESCRIBED (g->g_usig))
		cprintf ("((Ptr)%s*(%g**)%a)", oname, g->g_usig, l);
	    else {
		lname = alctemp (T_PTR);
		cprintf ("(%s=%a,", lname, l);
		cprintf ("*(Ptr*)%s%c=DSIZE(%s))", lname, *oname, lname);
		freetemp (T_PTR, lname);
	    }
	    break;

	case O_POSTINC:
	case O_POSTDEC:
	    oname = (e->e_opr == O_POSTINC ? "++" : "--");
	    if (!lvalue (l))
		err (l->e_locn, "expr%s: not a variable", oname);
	    if (t != T_PTR)
		cprintf ("((%e)%s)", l, oname);
	    else if (!DESCRIBED (g->g_usig))
		cprintf ("(Ptr)((*(%g**)%a)%s)", g->g_usig, l, oname);
	    else {
		lname = alctemp (T_PTR);
		tname = alctemp (T_PTR);
		cprintf ("(%s=%a,", lname, l);
		cprintf ("%s=*(Ptr*)%s,", tname, lname);
		cprintf ("*(Ptr*)%s=%s%cDSIZE(%s),", lname,tname,*oname,lname);
		cprintf ("%s)", tname);
		freetemp (T_PTR, tname);
		freetemp (T_PTR, lname);
	    }
	    break;

	case O_ADD:
	    if (r->e_sig->g_type == T_PTR) {
		l = e->e_r;
		r = e->e_l;	/* chg from (int + ptr) to (ptr + int)  */
	    }
	    /*NOBREAK*/
	case O_SUB:
	    c = (e->e_opr == O_ADD ? '+' : '-');
	    if (l->e_sig->g_type != T_PTR) {
		cprintf ("(%e%c%e)", l, c, r);	/* no pointers involved */
		break;
	    }
	    g = l->e_sig->g_usig;	/* type of underlying element */
	    if (g->g_type == T_ANY && r->e_sig->g_type == T_PTR)
		g = r->e_sig->g_usig;
	    if (!DESCRIBED (g)) {
		if (t == T_PTR)				
		    cprintf ("(%e%csizeof(%g)*%e)", l, c, g, r);   /* p +- i */
		else
		    cprintf ("((%g*)%e%c(%g*)%e)", g, l, c, g, r);  /* p - p */
	    } else {
		tname = alctemp (T_PTR);
		cprintf ("(%s=%e,", tname, l);
		if (t == T_PTR)
		    cprintf ("%s%cDSIZE(%s)*%e", tname, c, tname, r);
		else
		    cprintf ("(%s%c%e)/DSIZE(%s)", tname, c, r, tname);
		cprintf (")");
		freetemp (T_PTR, tname);
	    }
	    break;

	case O_MUL:
	    cprintf ("(%e*%e)", l, r);
	    break;
	case O_DIV:
	    cprintf ("(%e/ %e)", l, r);	/* space prevents '/' followed by '*' */
	    break;
	case O_REMDR:
	    if (t == T_REAL) {
#ifdef NO_FMOD
		cprintf ("(sr_rmod(%t,%e,%e))", e, l, r);
#else
		cprintf ("(fmod(%e,%e))", l, r);
#endif
	    } else
		cprintf ("(%e%%%e)", l, r);
	    break;
	case O_MOD:
	    cprintf ("%s(%t,%e,%e)", t==T_REAL ? "sr_rmod" : "sr_imod", e,l,r);
	    break;
	case O_EXPON:
	    if (t != T_REAL)
		fname = "sr_itoi";
	    else if (r->e_sig->g_type != T_REAL)
		fname = "sr_rtoi";
	    else
		fname = "sr_rtor";
	    cprintf ("%s(%t,%e,%e)", fname, e, l, r);
	    break;
	case O_LSHIFT:
	    cprintf ("(%e<<%e)", l, r);
	    break;
	case O_RSHIFT:
	    cprintf ("(%e>>%e)", l, r);
	    break;

	/* augmented assignment */

	case O_AUG_ADD:		augmop (e, O_ADD);	break;
	case O_AUG_SUB:		augmop (e, O_SUB);	break;
	case O_AUG_MUL:		augmop (e, O_MUL);	break;
	case O_AUG_EXPON:	augmop (e, O_EXPON);	break;
	case O_AUG_DIV:		augmop (e, O_DIV);	break;
	case O_AUG_REMDR:	augmop (e, O_REMDR);	break;
	case O_AUG_AND:		augmop (e, O_AND);	break;
	case O_AUG_OR:		augmop (e, O_OR);	break;
	case O_AUG_RSHIFT:	augmop (e, O_RSHIFT);	break;
	case O_AUG_LSHIFT:	augmop (e, O_LSHIFT);	break;
	case O_AUG_CONCAT:	augmop (e, O_CONCAT);	break;

	/* comparisons */

	case O_EQ:	gcompare (e, "==");	break;
	case O_NE:	gcompare (e, "!=");	break;
	case O_GT:	gcompare (e, ">");	break;
	case O_GE:	gcompare (e, ">=");	break;
	case O_LT:	gcompare (e, "<");	break;
	case O_LE:	gcompare (e, "<=");	break;

	/* boolean and bitwise operations */
	/* note use of %f in short-circuit operations */

	case O_NOT:
	    cprintf (t == T_BOOL ? "!%e" : "~%e", l);
	    break;
	case O_AND:
	    cprintf (t == T_BOOL ? "(%f&&%f)" : "(%e&%e)", l, r);
	    break;
	case O_OR:
	    cprintf (t == T_BOOL ? "(%f||%f)" : "(%e|%e)", l, r);
	    break;
	case O_XOR:
	    cprintf (t == T_BOOL ? "(!%e^!%e)" : "(%e^%e)", l, r);
	    break;
	
	/* resources */

	case O_CREVM:
	    if (r == NULL)
		cprintf ("sr_crevm(%t,sr_my_machine)", e);
	    else if (r->e_sig->g_type == T_INT) 
		cprintf ("sr_crevm(%t,%e)", e, r);
	    else
		cprintf ("sr_crevm_name(%t,%S)", e, r);
	    break;

	case O_CREATE:
	    ASSERT (l->e_l->e_opr == O_SYM);
	    s = l->e_l->e_sym;
	    ASSERT (s->s_kind == K_RES);
	    rname = s->s_gname;
	    tname = alctrans ();

	    cprintf ("(%s=", tname);
	    gparblk (NULLSTR,s->s_res->r_sig->g_proto,l->e_r,NULLNODE,O_CREATE);
	    cprintf (",\n%s=sr_create_resource(%t,N_%s,", tname, e, rname);
	    if (r)
		cprintf ("%e", r);
	    else
		cprintf ("sr_my_vm");
	    cprintf (",%s,sizeof(Rcap_%s))%,", tname, rname);
	    cprintf ("*(Rcap_%s*)%s)", rname, tname);
	    break;
	
	case O_DESTROY:
	    t = l->e_sig->g_type;
	    if (t == T_RCAP)
		cprintf ("sr_destroy(%t,(%e).r)", e, l);
	    else if (t == T_VCAP)
		cprintf ("sr_destvm(%t,%e)", e, l);
	    else if (t == T_OCAP)
		cprintf ("sr_dest_op(%t,%e)", e, l);
	    else {
		ASSERT (usig (l->e_sig) -> g_type == T_OCAP);
		cprintf ("sr_dest_array(%t,%e)", e, l);
	    }
	    break;

	/* special and miscellaneous */

	case O_SYM:
	    s = e->e_sym;
	    if (s->s_depth == 1 && s->s_kind == K_VAR && s->s_var->v_value != 0)
		gexpr (s->s_var->v_value);	/* EOF constant (etc?) */
	    else if (s->s_kind == K_PREDEF)
		EFATAL (e, "illegal use of predefined function");
	    else if (s->s_kind == K_OP && IMPORTED (s)
		&& s->s_imp->s_sig->g_type != T_GLB)
		    EFATAL (e,
			"imported op must be referenced via a capability");
	    else if (s->s_kind == K_TYPE)
		err (e->e_locn, "%s: not a variable", e->e_sym->s_name);
	    else
		cprintf ("%s", sname (s, 1));
	    break;

	case O_PTRDREF:
	    if (DESCRIBED (g))
		cprintf ("((%g)%e)", g, l);
	    else
		cprintf ("(*(%g*)%e)", g, l);
	    break;
	
	case O_ADDR:
	    if (!lvalue (l))
		EFATAL (e, "object of `@' is not a variable");
	    cprintf ("((Ptr)(%a))", l);
	    break;

	case O_VERBATIM:
	    cprintf ("%s", e->e_name);
	    break;
	
	case O_ASSIGN:
	    gassign (e);
	    break;

	case O_SWAP:
	    if (l->e_opr == O_SLICE || r->e_opr == O_SLICE) {
		EFATAL (e, "cannot swap into or out of a slice");
		break;
	    }
	    if (!lvalue (l) || !lvalue (r)) {
		EFATAL (e, "both sides of swap operator must be variables");
		break;
	    }
	    switch (t) {
		case T_BOOL:
		case T_CHAR:
		case T_INT:
		case T_ENUM:
		case T_REAL:
		case T_PTR:
		case T_OCAP:
		    /* all this complication is to avoid side effect problems */
		    /* from calculating the addresses more than once */
		    tname = alctemp (t);
		    lname = alctemp (T_PTR);
		    rname = alctemp (T_PTR);
		    cprintf ("(%s=(Ptr)%a%,", lname, l);  /* addr of l side */
		    cprintf ("%s=(Ptr)%a%,", rname, r);   /* addr of r side */
		    l = refnode (lname, g);
		    r = refnode (rname, g);
		    cprintf("%s = %e%,%e = %e%,%e = %s)",tname,r,r,l,l,tname);
		    freetemp (T_PTR, rname);
		    freetemp (T_PTR, lname);
		    if (t != T_REC)
			freetemp (t, tname);
		    break;
		case T_STRING:
		    cprintf ("sr_sswap(%t,%e,%e)", e, l, r);
		    break;
		case T_ARRAY:
		    cprintf ("sr_aswap(%t,%e,%e)", e, l, r);
		    break;
		default:
		    cprintf ("(*(%g*)sr_gswap((Ptr)%a,(Ptr)%a,sizeof(%g)))",
			g, l, r, g);
		    break;
	    }
	    break;

	case O_CONCAT:
	    tname = alctrans ();
	    cprintf ("((String*)(%s=sr_cat(", tname);
	    catargs (e);
	    cprintf ("NULL)))");
	    break;

	case O_ASTER:
	    cprintf ("ASTERISK");
	    break;

	default:
	    BOOM ("bad expression in gexpr()", oprtos (e->e_opr));
    }
}



/*  gcast (e) -- generate conversion code  */

static void
gcast (e)
Nodeptr e;
{
    Nodeptr r;
    Type t;
    char *tname, *fname;

    r = e->e_r;
    t = r->e_sig->g_type;

    switch (e->e_sig->g_type) {		/* switch on result type */

	case T_BOOL:
	    switch (t) {
		case T_STRING: 
		case T_ARRAY:
		    cprintf ("sr_boolval(%t,%S)", e, r);
		    break;
		default:
		    cprintf ("(%e!=0)", r);
		    break;
	    }
	    break;

	case T_CHAR:
	    switch (t) {
		case T_STRING:
		case T_ARRAY:
		    cprintf ("sr_charval(%S)", r);
		    break;
		case T_BOOL:
		case T_CHAR:
		    cprintf ("%e", r);
		    break;
		case T_INT:
		case T_ENUM:
		case T_REAL:
		case T_PTR:
		    if (option_O)	/* no error checking if optimizing */
			cprintf ("(Char)%e", r);
		    else {
			cprintf ("(");
			once (r, ',');
			cprintf ("(%e>=256||%e<=-256)?sr_runerr(%t,%d,%e):0,",
			    r, r, e, E_CCNV, r);
			cprintf ("(Char)%e)", r);
		    }
		    break;
	    }
	    break;

	case T_INT:
	case T_ENUM:
	    switch (t) {
		case T_STRING:
		case T_ARRAY:
		    cprintf ("sr_intval(%t,%S)", e, r);
		    break;
		default:	
		    cprintf ("(int)%e", r);   
		    break;
	    }
	    break;

	case T_REAL:
	    switch (t) {
		case T_STRING: 
		case T_ARRAY:
		    cprintf ("sr_realval(%t,%S)", e, r);
		    break;
		case T_PTR:
		    cprintf ("(Real)(unsigned int)%e", r);
		    break;
		default:
		    cprintf ("(Real)%e", r);
		    break;
	    }
	    break;

	case T_PTR:
	    switch (t) {
		case T_STRING:
		case T_ARRAY:
		    cprintf ("sr_ptrval(%t,%S)", e, r);
		    break;
		case T_REAL:
		    cprintf ("(Ptr)(int)%e", r);
		    break;
		default:	
		    cprintf ("(Ptr)%e", r);   
		    break;
	    }
	    break;

	case T_STRING:	
	    switch (t) {
		case T_INT:	fname = "sr_fmt_int";	break;
		case T_ENUM:	fname = "sr_fmt_int";	break;
		case T_CHAR:	fname = "sr_fmt_char";	break;
		case T_BOOL:	fname = "sr_fmt_bool";	break;
		case T_REAL:	fname = "sr_fmt_real";	break;
		case T_PTR:	fname = "sr_fmt_ptr";	break;
		case T_ARRAY:	fname = "sr_fmt_arr";	break;
		case T_STRING:	cprintf ("%e", r);	return;
		default:	EFATAL (e, "unsupported conversion"); return;
	    }
	    tname = alctrans ();	/* need temp so can free later */
	    cprintf ("((String*)(%s=%s(%e)))", tname , fname, r);
    }
}



/*  gaddr (e) -- generate address of an expression  */

void
gaddr (e)
Nodeptr e;
{
    Nodeptr l, r, a;
    Sigptr g;
    
    ASSERT (e != NULL);
    g = e->e_sig;

    if (e->e_opr == O_INDEX) {

	l = LNODE (e);
	r = RNODE (e);
	if (l->e_sig->g_type == T_STRING) {
	    if (option_O)
		cprintf ("(DATA(%e)+%e-1)", l, r);
	    else {
		cprintf ("(");
		once (l, ',');
		once (r, ',');
		if (!option_O)	/* gen bounds check if not optimizing */
		    cprintf (
			"%e<1||%e>%e->length?sr_runerr(%t,%d,%e,%e,%e):0%,",
			r, r, l, e, E_SSUB, r, l, l);
		cprintf ("(Char*)DATA(%e)+%e-1)", l, r);
	    }
	} else {
	    ASSERT (l->e_sig->g_type == T_ARRAY);	/* parent is array */
	    ASSERT (g->g_type != T_ARRAY);		/* result is not */
	    cprintf ("(");
	    for (a = l; a->e_opr == O_INDEX; a = a->e_l)
		;
	    once (a, ',');
	    once (r, ',');
	    /* unfortunately no separate type for an OP that's a SEM.
	     * and csig doesn't have enough info to make this test.
	     * so, do the test here and hardwire in the C equivalent.
	     */
	    if (a->e_opr == O_SYM &&
		a->e_sym->s_kind == K_OP &&
		a->e_sym->s_op->o_impl == I_SEM)
		cprintf (DESCRIBED (g) ? "((Sem)(((" : "((Sem*)(");
	    else
		cprintf (DESCRIBED (g) ? "((%g)(((" : "((%g*)(", g);
	    a = gindex (l);
	    cprintf ("))+("); 
	    if (!option_O) {       /* gen bounds check if not optimizing */
		cprintf (
		"(%e<%l||%e>%u)?sr_runerr(%t,%d,%e,((Dim*)(%e+1))+%d):0%,",
		r, l, r, l, e, E_ASUB, r, a, ndim (g));
	    }
	    if (! DESCRIBED (g))
		cprintf ("%e-%l))", r, l); 
	    else {
		cprintf ("%e-%l)*", r, l);
		gstride (a, g);
		cprintf (")))");
	    }
	}

    } else if (DESCRIBED (g))
	cprintf ("%e", e);
    else
	cprintf ("(&%e)", e);
}



/*
 *  gindex (e) -- gen address of array or higher dimension containing element
 *
 *  Returns a node representing the base of the underlying array.
 *
 *  A pointer expression is generated; the caller should provide enclosing
 *  parentheses.
 */
static Nodeptr
gindex (e)
Nodeptr e;
{
    Nodeptr l, r, arr;
    Sigptr g;

    ASSERT (e != NULL);
    g = e->e_sig;

    if (e->e_opr != O_INDEX) {
	/* e is array */
	cprintf ("(Ptr)%e+%d", e, sizeof (Array) + ndim (g) * sizeof (Dim));
	return e;
    }

    /* e is not an array. recurse to array, then continue */
    l = LNODE (e);
    r = RNODE (e);
    arr = gindex (l);
    cprintf ("+("); 
    once (r, ',');
    if (!option_O)	       /* gen bounds check if not optimizing */
	cprintf ("(%e<%l||%e>%u)?sr_runerr(%t,%d,%e,((Dim*)(%e+1))+%d):0%,",
	    r, l, r, l, e, E_ASUB, r, arr, ndim (g));
    cprintf ("%e-%l)*", r, l); 
    gstride (arr, g);
    return arr;
}



/*  gfexpr (e) -- generate expression and free any transient memory it allocs */

void
gfexpr (e)
Nodeptr e;
{
    Type t;
    char *b;
    int n;

    t = e->e_sig->g_type;
    b = alctemp (t);
    n = ntrans ();
    cprintf ("(%s=%e", b, e);
    if (ntrans () > n) {
	freetrans (n, ',');
	cprintf ("%,%s", b);
    }
    cprintf (")");
    freetemp (t, b);
}



/*  augmop (e, o) -- generate augmented assignment for using binary opr o  */

static void
augmop (e, o)
Nodeptr e;
Operator o;
{
    Nodeptr a, l;
    Sigptr g;
    char *tname;

    g = e->e_sig;
    tname = alctemp (T_PTR);
    l = refnode (tname, g);
    a = bnode (O_ASSIGN, l, bnode (o, l, e->e_r));
    attest (a);
    cprintf ("(%s=(Ptr)%a%,%e)", tname, e->e_l, a);
    freetemp (T_PTR, tname);
}



/*  gcompare (e, test) -- generate comparison depending on signature  */

static void
gcompare (e, test)
Nodeptr e;
char *test;
{
    Type lt, rt;

    lt = e->e_l->e_sig->g_type;
    rt = e->e_r->e_sig->g_type;
    if (lt == T_RES || lt == T_RCAP || lt == T_OP || lt == T_OCAP)
	capcmp (e->e_l, e->e_r, test);
    else if (rt == T_RES || rt == T_RCAP || rt == T_OP || rt == T_OCAP)
	capcmp (e->e_r, e->e_l, test);
    else if (lt == T_STRING)
	cprintf ("(sr_strcmp(%S,%S)%s0)", e->e_l, e->e_r, test);
    else
	cprintf ("(%e%s%e)", e->e_l, test, e->e_r);
}



/*  capcmp (l, r, test) -- generate capability comparison
 *
 *  only the right operand can be a null/noop literal.
 *
 *  This code is ugly because either or both operands could be rvalues;
 *  so we must copy them to temps in order to pass addresses to memcmp.
 */

static void
capcmp (l, r, test)
Nodeptr l, r;
char *test;
{
    Sigptr g;
    static int ncmp = 0;

    g = l->e_sig;
    if (g->g_type == T_RCAP)
	g = g->g_usig;

    if (r->e_opr == O_NLIT) {
	cprintf ("((%e)%s.seqn%s%d)",
	    l, g->g_type == T_RES ? ".r" : "", test, r->e_int);
	return;
    }
    ncmp++;
    cprintf ("%8%g ct%da; %g ct%db;\n", l->e_sig, ncmp, r->e_sig, ncmp);
    cprintf
	("(ct%da=%e,ct%db=%e%,memcmp((Ptr)&ct%da,(Ptr)&ct%db,sizeof(%g))%s0)",
	ncmp, l, ncmp, r, ncmp, ncmp, g, test);
}



/*  gassign (e) -- generate assignment expression  */

static void
gassign (e)
Nodeptr e;
{
    Nodeptr l, r;
    Sigptr g;
    int hlen;

    l = e->e_l;
    r = e->e_r;
    g = e->e_sig;

    if (!lvalue (l)) {
	EFATAL (l, "assignment target is not a variable");
	return;
    }
    
    switch (g->g_type) {

	case T_BOOL:
	case T_CHAR:
	case T_INT:
	case T_ENUM:
	case T_REAL:
	case T_FILE:
	case T_PTR:
	case T_VCAP:
	case T_OP:
	case T_OCAP:
	    cprintf ("(%e=%e)", l, r);
	    break;

	case T_REC:
	    if (l->e_sig->g_rec == r->e_sig->g_rec)
		/* same C structs; no cast needed */
		cprintf ("(%e=%e)", l, r);
	    else
		/* cast L side; R side may not be an lvalue */
		cprintf ("(*(%g*)&%e=%e)", r->e_sig, l, r);
	    break;

	case T_RCAP:
	    if (l->e_sig->g_usig->g_sym == r->e_sig->g_usig->g_sym)
		/* same C structs; no cast needed */
		cprintf ("(%e=%e)", l, r);
	    else
		/* cast L side; R side may not be an lvalue */
		cprintf ("(*(%g*)&%e=%e)", r->e_sig, l, r);
	    break;

	case T_STRING:
	    ASSERT (r->e_sig->g_type == T_STRING);
	    if (l->e_opr != O_SLICE) {
		 
		/* normal string assignment */
		cprintf ("(");
		once (l, ',');
		once (r, ',');
		if (!option_O)		/* gen length check if not optimizing */
		    cprintf (
			"(%e->length>%e->size-%d)?sr_runerr(%t,%d,%e,%e):0%,",
			r, l, STRING_OVH, e, E_SSIZ, r, l);
		cprintf (
		"(String*)((char*)memcpy(4+(Ptr)%e,4+(Ptr)%e,4+%e->length)-4)",
		    l, r, r);
		cprintf (")");

	    } else {

		/* assignment to slice */
		/* (lots of args here, but no part of tree is ref'd twice) */
		cprintf ("sr_chgstr(%t%,%e%,%e%,%e%,%e)",
		    e, l->e_l, l->e_r->e_l->e_l, l->e_r->e_l->e_r, r);
	    }
	    break;

	case T_ARRAY:
	    ASSERT (r->e_sig->g_type == T_ARRAY);
	    if (l->e_opr == O_SLICE) {

		/* assignment to slice */
		if (l->e_l->e_sig->g_usig->g_type == T_STRING) {
		    /* slice of array of strings */
		    cprintf ("sr_strarr(%t%,%e%,%e%,%e%,%e)",
			e, l->e_l, l->e_r->e_l->e_l, l->e_r->e_l->e_r, r);
		} else {
		    /* any other array */
		    gslice (r, l);	
		}

	    } else if (l->e_sig->g_usig->g_type == T_STRING) {

		/* elementwise assignment of string array */
		cprintf ("sr_strarr(%t%,%a%,ASTERISK%,ASTERISK%,%a)", e, l, r);

	    } else if (r->e_sig->g_usig->g_type != T_ARRAY) {
		/* 1-D array assignment */
		cprintf ("(");
		once (l, ',');
		once (r, ',');
		if (!option_O)	/* gen size check if not optimizing */
		    cprintf ("(%u-%l!=%u-%l)?sr_runerr(%t,%d,%e+1,%e+1):0%,",
		    l, l, r, r, e, E_ASIZ, l, r);
		hlen = sizeof (Array) + ndim (g) * sizeof (Dim);
		cprintf (
	    "(Array*)((char*)memcpy(((Ptr)%e)+%d,((Ptr)%e)+%d,%e->size-%d)-%d)",
		    l, hlen, r, hlen, l, hlen, hlen);
		cprintf (")");

	    } else {

		    /* multi-dimensional array assignment */
		    cprintf ("sr_acopy(%t,%a,%a)", e, l, r);
	    }
	    break;

	default:
	    BOOM ("unrecognized type in gassign()", typetos (g->g_type));
	    break;
    }
}



/*  gslice (source, slice) -- generate slice expression
 *
 *  source is the data source (slice assignment) or NULL (slice extraction).
 *  slice is the slice expression.
 */

static void
gslice (source, slice)
Nodeptr source, slice;
{
    Nodeptr r;
    Sigptr g;
    int n, d;

    ASSERT (slice->e_opr == O_SLICE);
    g = slice->e_l->e_sig;
    d = ndim (g);
    n = 0;
    for (r = slice->e_r; r != NULL; r = r->e_r) {
	g = g->g_usig;			/* underlying element signature */
	n++;				/* number of index positions */
    }

    cprintf ("sr_slice(%t,", slice);
    if (source != NULL)
	cprintf ("%e%,", source);
    else
	cprintf ("(Array*)0%,");
    cprintf ("%e%,", slice->e_l);

    if (g->g_type == T_RCAP || g->g_type == T_REC)
	cprintf ("sizeof(%g)", g);
    else
	cprintf ("%d", TSIZE (g->g_type));	/* 0 ok, is flag to RTS */

    cprintf (",%d", d);

    for (r = slice->e_r; r != NULL; r = r->e_r)
	if (r->e_l->e_l == NULL)
	    cprintf ("%,%e,SINGLEINDEX", r->e_l->e_r);
	else
	    cprintf ("%,%e,%e", r->e_l->e_l, r->e_l->e_r);
    while (n++ < d)
	cprintf ("%,ASTERISK,ASTERISK");
    cprintf (")");
}



/*  catargs (e) -- generate the args of concatenation from left to right  */

static void
catargs (e)
Nodeptr e;
{
    if (e->e_opr == O_CONCAT) {
	catargs (e->e_l);
	catargs (e->e_r);
    } else if (e->e_sig->g_type == T_CHAR) {
	cprintf ("(String*)(%e<<2|1),", e);	/* pass char with tag */
    } else {
	cprintf ("%S,", e);			/* pass String pointer */
    }
}



/*  strlit (e) -- generate string literal  */

static void
strlit (e)
Nodeptr e;
{
    int c, n;
    char *s;
    static int strnum = 0;

    if (e->e_r == NULL) {	/* if first time for this string */
	++strnum;
	s = DATA (e->e_sptr);
	n = e->e_sptr->length;
	cprintf ("%7STRING(s%d,%d,\"", strnum, n);
	while (n--) {
	    c = *s++ & 0xFF;
	    if (!isascii (c))
		cprintf ("%7\\%o", c);
	    else if (c == '"' || c == '\\')
		cprintf ("%7\\%c", c);
	    else if (isprint (c))
		cprintf ("%7%c", c);
	    else if (isspace (c))
		cprintf ("%7\\%c", "tnvfr"[c-9]);
	    else
		cprintf ("%7\\%o", c);
	}
	cprintf ("%7\");\n");
	e->e_r = intnode (strnum);
    }

    cprintf ("((String*)&s%e)", e->e_r);
}
