/*  gmisc.c -- miscellaneous small code generation routines  */

#include "compiler.h"


static void initrec	PARAMS ((Nodeptr e));
static int fixedsize	PARAMS ((Sigptr g, int units));
static int fixedstride	PARAMS ((Sigptr g));



/*  once (e, c) -- generate e only once.
 *
 *  once() is called if expression e needs to be referenced more than once.
 *  If e has potential side effects then it is saved in a statement-lifetime
 *  temporary and the node is rewritten as an O_VERBATIM node referencing
 *  the temporary.
 *
 *  If c is ',' code is generated only if there is the side-effect danger.
 *  The generated code is followed by a comma, presumably as part of a
 *  comma-expression. 
 *
 *  If c is 0, the expression is generated unconditionally, and with no comma.
 *
 *  Note that only types supported by alctemp() can be handled.
 */

Nodeptr
once (e, c)
Nodeptr e;
int c;
{
    Sigptr g;
    Type t;
    char *tname;

    if (e == NULL)
	return e;
    switch (e->e_opr) {
	case O_ILIT:		/* simple nodes, no problem to ref twice */
	case O_RLIT:
	case O_BLIT:
	case O_CLIT:
	case O_NLIT:
	case O_FLIT:
	case O_SLIT:
	case O_SYM:
	case O_VERBATIM:	/* already stored in temporary */
	    if (c == 0)
		gexpr (e);
	    return e;
	case O_SLICE:
	    gexpr (e);		/* always gens VERBATIM node */
	    if (c != 0)
		cprintf ("%c", c);
	    return e;
    }
    g = e->e_sig;
    t = g->g_type;
    tname = alctemp (t);
    cprintf ("(%s=%e)", tname, e);
    if (c == ',')
	cprintf (",");
    nreplace (e, NULLSTR, tname);
    freetemp (t, tname);
    return e;
}



/*  goctal (n) -- generate three-digit octal constant  */

void
goctal (n)
int n;
{
    char s[4];
    s[0] = ('0' + ((n >> 6) & 7));
    s[1] = ('0' + ((n >> 3) & 7));
    s[2] = ('0' + ((n >> 0) & 7));
    s[3] = '\0';
    cprintf (s);
}



/*  opdecl (o) -- generate opcap decl including any array header  */

void
opdecl (o)
Opptr o;
{
    Symptr s;
    Sigptr g;

    s = o->o_sym;
    g = o->o_asig;
    ASSERT (g->g_type == T_OP || g->g_type == T_ARRAY);
    if (o->o_impl == I_SEM) {			/* if optimized into semaphore*/
	if (g->g_type == T_OP)
		/* probably should be Sem instead of Ptr? (but same now.) */
	    cprintf ("Ptr _%s;\n", s->s_gname);
	else
	    cprintf ("Array *_%s;\n", s->s_gname);
    }
    else if (g->g_type == T_OP)			/* if other simple opcap */
	cprintf ("Ocap _%s;\n", s->s_gname);
    else if (s->s_imp)				/* if imported/exported array */
	cprintf ("Array _%s[%d];\n", s->s_gname, fixedsize (g, sizeof (Array)));
    else					/* if private array */
	cprintf ("Array *_%s;\n", s->s_gname);
}



/*  sname (s, wantfull) -- return name of symbol in C terms
 *
 *  The return value points to a static buffer; use it or lose it!
 */

char *
sname (s, f)
Symptr s;
int f;
{
    Varptr v;
    Symptr p;
    static char buf[100];
    char *o, *pbvar;

    o = buf;
    switch (s->s_kind) {
	case K_VAR:
	    v = s->s_var;
	    switch (v->v_vty) {
		case V_GLOBAL:
		    if (f) {
			sprintf (o, "gv_%s.", s->s_imp->s_gname);
			o += strlen (o);
		    }
		    break;
		case V_RVAR:
		    if (f) {
			strcpy (o, "rv->");
			o += 4;
		    }
		    if (IMPORTED (s)) {
			sprintf (o, "_%s", s->s_imp->s_gname);
			o += strlen (o);
		    }
		    break;
		case V_RPARAM:
		    if (DESCRIBED (v->v_sig))
			sprintf (buf, "((%s)((Ptr)rp+rp->o_%d))",
			    csig (v->v_sig), v->v_seq);
		    else
			sprintf (buf, "rp->_%d", v->v_seq);
		    return buf;
		case V_PARAM:
		    for (p = s->s_prev; p->s_kind != K_BLOCK; p = p->s_prev)
			;
		    if (p->s_pb != NULL)
			pbvar = p->s_pb;
		    else
			pbvar = "pb";
		    if (DESCRIBED (v->v_sig))
			sprintf (buf, "((%s)((Ptr)%s+%s->o_%d))",
			    csig (v->v_sig), pbvar, pbvar, v->v_seq);
		    else
			sprintf (buf, "%s->_%d", pbvar, v->v_seq);
		    return buf;
		case V_REFNCE:
		    for (p = s->s_prev; p->s_kind != K_BLOCK; p = p->s_prev)
			;
		    if (p->s_pb != NULL)
			pbvar = p->s_pb;
		    else
			pbvar = "pb";
		    if (DESCRIBED (v->v_sig))
			sprintf (buf, "%s->_%d", pbvar, v->v_seq);
		    else
			sprintf (buf, "(*%s->_%d)", pbvar, v->v_seq);
		    return buf;
		case V_LOCAL:
		    sprintf (buf, "_%s", s->s_gname);
		    return buf;
		case V_RMBR:
		    return s->s_name;
		default:
		    BOOM ("unexpected variety in sname", varitos (v->v_vty));
	    }
	    break;
	case K_OP:
	    if (f) {
		if (s->s_depth == 2 || s->s_imp != NULL) {
		    if (s->s_imp!=NULL && s->s_imp->s_res->r_sig->g_type==T_GLB)
			sprintf (o, "G_%s->", s->s_imp->s_gname);
		    else
			strcpy (o, "rv->");
		    o += strlen (o);
		}
	    }
	    break;
	case K_ELIT:
	    sprintf (buf, "%d", s->s_seq);
	    return buf;
	default:
	    /* nothing special */
	    ;
    }

    /*
     *  Now, finally, the name.
     */
    *o++ = '_';
    strcpy (o, s->s_gname);
    return buf;
}



/*  csig (g) -- return C type corresponding to signature g.  */

char *
csig (g)
Sigptr g;
{
    static char buf[100];

    switch (g->g_type) {

	case T_BOOL:	return "Bool";
	case T_CHAR:	return "Char";
	case T_INT:	return "Int";
	case T_ENUM:	return "Enum";
	case T_REAL:	return "Real";
	case T_FILE:	return "File";
	case T_STRING:	return "String *";
	case T_ARRAY:	return "Array *";
	case T_PTR:	return "Ptr";
	case T_ANY:	return "char";
	case T_REC:	ASSERT (g->g_rec->k_tdef); return g->g_rec->k_tdef;
	case T_VCAP:	return "Vcap";
	case T_OCAP:	return "Ocap";
	case T_OP:	return "Ocap";

	case T_RCAP:
	    g = g->g_usig;
	    /*NOBREAK*/
	case T_RES:
	    sprintf (buf, "Rcap_%s", g->g_sym->s_gname);
	    return buf;
	    
	default:	BOOM ("unknown type in csig", typetos (g->g_type));
			/*NOTREACHED*/
    }
}



/*  adouble (g) -- return FALSE if g does not need to be double-aligned  */

Bool
adouble (g)
Sigptr g;
{
    switch (g->g_type) {
	case T_REAL:
	case T_REC:
	    return TRUE;
	case T_ARRAY:
	    return adouble (usig (g));
	default:
	    return FALSE;
    }
}



/*  fixtypes (e) -- nail down type information from within tree e.
 *
 *  Saves away array bounds, string maxlengths, etc. and declares rec structs.
 */

void
fixtypes (e)
Nodeptr e;
{
    Nodeptr l, r;

    if (!e)
	return;
    l = LNODE (e);
    r = RNODE (e);

    switch (e->e_opr) {
	case O_STRING:
	case O_BOUNDS:
	    fixvalue (l);
	    fixvalue (r);
	    break;
	case O_REC:
	    fixtypes (l);
	    initrec (e);
	default:
	    fixtypes (l);
	    fixtypes (r);
	    break;
    }
}



/*  fixvalue (e) -- if e is an expression, calc and save its value now  */

void
fixvalue (e)
Nodeptr e;
{
    static int nvalues = 0;
    char buf[20];

    if (e == NULL)
	return;
    switch (e->e_opr) {
	case O_BLIT:
	case O_CLIT:
	case O_ILIT:
	case O_VERBATIM:
	    /* already okay */
	    break;
	case O_ASTER:
	    /* can't handle, hope it's okay */
	    break;
	default:
	    /* put in resource or local variables as appropriate */
	    ++nvalues;
	    if (curproto) {		/* if in a proc */
		cprintf ("%8int v%d;\n", nvalues);
		sprintf (buf, "v%d", nvalues);
	    } else {			/* must be at resource level */
		cprintf ("%6int v%d;\n", nvalues);
		sprintf (buf, "rv->v%d", nvalues);
	    }
	    cprintf ("%s=%e;\n", buf, e);
	    nreplace (e, NULLSTR, buf);
	    break;
    }
}



/*  initrec (e) -- allocate and initialize a record template  */

static void
initrec (e)
Nodeptr e;
{
    Nodeptr a;
    Sigptr g, h;
    Symptr s;
    int n, nbytes;
    char *tdef;				/* typedef name */
    char init[20];			/* initializer name */
    Bool needinit;			/* initializer needed? */

    g = e->e_sig;
    if (g->g_rec->k_size)
	return;				/* already processed */

    /*
     * declare typedef struct and calculate size
     *
     * We are making some pessimistic assumptions about struct packing; 
     * if we're wrong, we'll waste a little space on arrays of records,
     * but otherwise no harm is done.
     *
     * (We need the size for compile-time allocations, but we always reference
     *  members via the C struct.)
     */
    tdef = g->g_rec->k_tdef;		/* get name assigned earlier */
    nbytes = 0;
    cprintf ("%2typedef struct {\n");
    for (a = e->e_l; a != NULL; a = a->e_r) {
	s = a->e_l->e_l->e_l->e_sym;
	h = a->e_l->e_sig;
	switch (h->g_type) {
	    case T_ARRAY:
		if (adouble (h)) {	/* if needs double alignment */
		    n = fixedsize (h, sizeof (Real));
		    cprintf ("%2Real %N[%d];\n", s, n);
		    nbytes = SRALIGN (nbytes) + sizeof (Real) * n;
		} else {
		    n = fixedsize (h, sizeof (Array));
		    cprintf ("%2Array %N[%d];\n", s, n);
		    nbytes = SRALIGN (nbytes) + sizeof (Array) * n;
		}
		break;
	    case T_STRING:
		n = fixedsize (h, sizeof (String));
		cprintf ("%2String %N[%d];\n", s, n);
		nbytes = SRALIGN (nbytes) + sizeof (String) * n;
		break;
	    default:
		cprintf ("%2%g %N;\n", h, s);
		n = fixedsize (h, 1);
		if (n <= sizeof (int))
		    nbytes += sizeof (int);
		else
		    nbytes = SRALIGN (nbytes) + SRALIGN (n);
		break;
	}
    }
    cprintf ("%2} %s;\n\n", g->g_rec->k_tdef);
    g->g_rec->k_size = SRALIGN (nbytes);

    /* reserve a name for the initializer */
    if (curproto)				/* if in a proc */
	sprintf (init, "ri%s", tdef + 2);
    else					/* must be at resource level */
	sprintf (init, "rv->ri%s", tdef + 2);

    /* initialize initializer */
    needinit = FALSE;
    for (a = e->e_l; a != NULL; a = a->e_r) {
	s = a->e_l->e_l->e_l->e_sym;
	h = a->e_l->e_sig;
	switch (h->g_type) {
	    case T_ARRAY:
		cprintf ("sr_init_array(%t,(Array*)%s.%N,", e, init, s);
		garray (e, h);
		cprintf (");\n");
		needinit = TRUE;
		break;
	    case T_STRING:
		cprintf ("%s.%N->size=%d;\n", init, s,
		    h->g_ub->e_int + STRING_OVH);
		cprintf ("%s.%N->length=-1;\n", init, s);
		needinit = TRUE;
		break;
	    case T_REC:
		if (h->g_rec->k_init) {
		    cprintf ("%s.%N=%s;\n", init, s, h->g_rec->k_init);
		    needinit = TRUE;
		}
		break;
	    default:
		ASSERT (!DESCRIBED (h));
		break;
	}
    }

    /*
     * If the initializer was really needed, declare it.
     */
    if (needinit) {
	if (curproto)				/* if in a proc */
	    cprintf ("%8%s ri%s;\n", tdef, tdef + 2);
	else					/* must be at resource level */
	    cprintf ("%6%s ri%s;\n", tdef, tdef + 2);
	g->g_rec->k_init = rsalloc (init);
    }
}



/*  fixedsize (g, units) -- return fixed size of type g in specified units.
 *
 *  This is to allow declaring a variable as an array of g even when its
 *  true structure is more complex.
 *
 *  An error message is issued, and a value of 1 returned, if the size
 *  is not constant at compile time.
 */  

static int
fixedsize (g, units)
Sigptr g;
int units;
{
    Type t;
    int nbytes;
    Symptr s, o;

    t = g->g_type;
    switch (t) {
	case T_ARRAY:
	    fixvalue (g->g_ub);
	    fixvalue (g->g_lb);
	    nbytes = fixedsize (g->g_usig, 1);
	    if (g->g_lb->e_opr == O_ILIT && g->g_ub->e_opr == O_ILIT)
		nbytes = sizeof (Array) + ndim (g) * sizeof (Dim)
		    + nbytes * (g->g_ub->e_int - g->g_lb->e_int + 1);
	    else {
		EFATAL (g->g_ub, "non-constant array bounds in spec or record");
		g->g_lb = g->g_ub = intnode (1);
		return 1;
	    }
	    nbytes = SRALIGN (nbytes);
	    break;
	case T_STRING:
	    fixvalue (g->g_ub);
	    if (g->g_ub->e_opr == O_ILIT)
		nbytes = SRALIGN (g->g_ub->e_int + STRING_OVH);
	    else {
		EFATAL(g->g_ub, "non-constant string length in spec or record");
		g->g_ub = intnode (1);
		return 1;
	    }
	    break;
	case T_REC:
	    ASSERT (g->g_rec != NULL);
	    nbytes = g->g_rec->k_size;
	    if (nbytes == 0) {
		FATAL ("recursively defined record");	/* no line ref avail */
		nbytes = sizeof (Real);
	    } else
		ASSERT (nbytes >= sizeof (int));
	    break;
	case T_RCAP:
	    nbytes = sizeof (Rcap);
	    g = g->g_usig;		/* signature of underlying resource */
	    s = g->g_sym;		/* beginning of res's symtab entries */
	    for (o = s->s_next; o != NULL; o = o->s_next)
		if (o->s_kind == K_OP && o->s_imp == s && o->s_sig)  /* if op */
		    if (o->s_sig->g_type == T_ARRAY)
			nbytes +=
			    sizeof(Array) * fixedsize(o->s_sig, sizeof(Array));
		    else
			nbytes += SRALIGN (sizeof (Ocap));
	    break;
	default:
	    nbytes = TSIZE (t);
	    ASSERT (nbytes > 0);
	    break;
    }

    return (nbytes + units - 1) / units;
}



/*  gstride (e, g) -- generate expression for stride.
 *
 *  e is the underlying array.
 *  g is a signature indicating the desired dimension.
 */
void
gstride (e, g)
Nodeptr e;
Sigptr g;
{
    int nbytes;

    if ((nbytes = fixedstride (g)) != 0)
	cprintf ("%d", nbytes);
    else
	cprintf ("STRIDE(%e,%d)", e, ndim (g));
}

static int
fixedstride (g)
Sigptr g;
{
    Type t;
    int nbytes;

    t = g->g_type;
    switch (t) {
	case T_ARRAY:
	    if ((nbytes = fixedstride (g->g_usig)) != 0)
		return 0;
	    fixvalue (g->g_lb);
	    if (g->g_lb == NULL || g->g_lb->e_opr != O_ILIT)
		return 0;
	    fixvalue (g->g_ub);
	    if (g->g_ub == NULL || g->g_ub->e_opr != O_ILIT)
		return 0;
	    return nbytes * (g->g_ub->e_int - g->g_lb->e_int + 1);
	case T_STRING:
	    fixvalue (g->g_ub);
	    if (g->g_ub->e_opr != O_ILIT)
		return 0;
	    return SRALIGN (g->g_ub->e_int + STRING_OVH);
	case T_REC:
	case T_RCAP:
	    return 0;		/* can't know *exact* size at compile time */
	default:
	    return TSIZE (t);
    }
}



/*  garray (e, g) -- generate 2nd etc sr_init_array args for signature g  */

/*  node e is used only to give the location of an error  */

void
garray (e, g)
Nodeptr e;
Sigptr g;
{
    Sigptr u;
    int n;

    u = usig (g);
    switch (u->g_type) {
	case T_STRING:
	    cprintf ("-%e,(Ptr)0", u->g_ub);
	    break;
	case T_REC:
	    ASSERT (u->g_rec && u->g_rec->k_tdef);
	    cprintf ("sizeof(%s),", u->g_rec->k_tdef);
	    if (u->g_rec->k_init)
		cprintf ("(Ptr)&%s", u->g_rec->k_init);
	    else
		cprintf ("(Ptr)0");
	    break;
	case T_RCAP:
	    cprintf ("SRALIGN(sizeof(Rcap_%s))%,(Ptr)0",
		u->g_usig->g_sym->s_gname);
	    break;
	default:
	    n = TSIZE (u->g_type); 
	    ASSERT (n > 0);
	    cprintf ("%d,(Ptr)0", n);
	    break;
    }
    cprintf ("%,%d", ndim (g));
    for (; g->g_type == T_ARRAY; g = g->g_usig) {
	if (g->g_lb->e_opr == O_ASTER || g->g_ub->e_opr == O_ASTER) {
	    EFATAL (e, "illegal context for `*' as array bound");
	    break;
	}
	cprintf (",%e,%e", g->g_lb, g->g_ub);
    }
}
