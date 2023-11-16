/*  signat.c -- signature management code  */

#include "compiler.h"



static Bool rcompat	PARAMS ((Sigptr g1, Sigptr g2, Compat c));
static int esize	PARAMS ((Sigptr g));



/*  initsig() -- initialize type flags and predefined signatures  */

void
initsig ()
{
    TFLAGS (T_BOOL) = TF_ORDERED + TF_CONVERT;  TSIZE (T_BOOL) = sizeof (Bool);
    TFLAGS (T_CHAR) = TF_ORDERED + TF_CONVERT;  TSIZE (T_CHAR) = sizeof (Char);
    TFLAGS (T_INT)  = TF_ORDERED + TF_CONVERT;  TSIZE (T_INT)  = sizeof (Int);
    TFLAGS (T_ENUM) = TF_ORDERED + TF_CONVERT;  TSIZE (T_ENUM) = sizeof (Enum);
    TFLAGS (T_REAL) = TF_ORDERED + TF_CONVERT;  TSIZE (T_REAL) = sizeof (Real);

    TFLAGS (T_PTR)  = TF_CONVERT;		TSIZE (T_PTR)  = sizeof (Ptr);

    TFLAGS (T_STRING) = TF_DESCR + TF_CONVERT;
    TFLAGS (T_ARRAY) = TF_DESCR;
    TFLAGS (T_REC)   = 0;

    TSIZE (T_FILE) = sizeof (File);
    TSIZE (T_VCAP) = sizeof (Vcap);
    TSIZE (T_OCAP) = sizeof (Ocap);
    TSIZE (T_OP)   = sizeof (Ocap);
    /* however RCAP is NOT fixed size */

    void_sig  = newsig (T_VOID, NULLSIG);  
    any_sig   = newsig (T_ANY,  NULLSIG);  
    nlit_sig  = newsig (T_NLIT, NULLSIG); 
    bool_sig  = newsig (T_BOOL, NULLSIG);
    char_sig  = newsig (T_CHAR, NULLSIG);
    int_sig   = newsig (T_INT,  NULLSIG);
    real_sig  = newsig (T_REAL, NULLSIG);
    file_sig  = newsig (T_FILE, NULLSIG);
    vcap_sig  = newsig (T_VCAP, NULLSIG);
    array_sig = newsig (T_ARRAY, char_sig);
    string_sig = newsig (T_STRING, NULLSIG);
    ptr_sig   = newsig (T_PTR,  any_sig);
}



/*  newsig(type,usig) -- allocate and initialize a new Sig structure  */

Sigptr
newsig (t, u)
Type t;
Sigptr u;
{
    Sigptr g;

    g = NEW (Signat);			/* allocate */
    g->g_type = t;			/* set type */
    g->g_usig = u;			/* set underlying sig */
    return g;				/* return new struct */
}



/*  symsig(s) -- return the signature of a Symbol  */

Sigptr
symsig (s)
Symptr s;
{
    Sigptr g;

    switch (s->s_kind) {
	case K_BLOCK:
	case K_PREDEF:
	    g = void_sig;	/* reference is void, though a CALL isn't */
	    break;
	case K_TYPE:
	case K_FVAL:
	case K_FVAR:
	case K_FRES:
	case K_FREF:
	case K_ELIT:
	    g = s->s_sig;
	    break;
	case K_VAR:
	case K_FIELD:
	    if (s->s_var)
		g = s->s_var->v_sig;
	    else
		g = void_sig;	/* error reported earlier */
	    break;
	case K_OP:
	    ASSERT (s->s_op != NULL);
	    g = s->s_op->o_asig;
	    break;
	case K_RES:
	    ASSERT (s->s_res != NULL);
	    g = s->s_res->r_sig;
	    break;
	default:
	    BOOM ("bad kind in symsig", symtos (s));
	    /* NOTREACHED */
    }
    if (g == NULL)
	g = void_sig;		/* recursive signature */
    return g;
}



/*  easylval (e) -- check if e is an "easy" lvalue: anything but a slice  */

Bool
easylval (e)
Nodeptr e;
{
    if (e->e_opr == O_SLICE)
	return FALSE;
    else
	return lvalue (e);
}



/*  lvalue (e) -- check if e is an lvalue  */

Bool
lvalue (e)
Nodeptr e;
{
    Symptr s;

    switch (e->e_opr) {
	case O_SLICE:
	case O_INDEX:
	    return easylval (e->e_l);
	case O_FIELD:
	    if (e->e_l->e_sig->g_type == T_GLB)
		return lvalue (e->e_r);
	    else if (e->e_l->e_sig->g_type == T_RES)
		return FALSE;
	    else
		return lvalue (e->e_l);
	case O_SYM:
	    s = e->e_sym;
	    return s->s_kind == K_VAR && s->s_var->v_dcl != O_CONDCL;
	case O_VERBATIM:
	    if (e->e_r)
		return lvalue (e->e_r);
	    else
		return TRUE;
	case O_PTRDREF:
	    return TRUE;
	default:
	    return FALSE;
    }
}



/*  compat(g1,g2,c) -- check signatures for compatibility
 *
 *  c is degree of compatibility needed: C_EXACT, C_ASSIGN, or C_COMPARE.
 *
 *  Note that the recursive calls below are always C_EXACT.  Real and int
 *  may sometimes be compatible, but "ptr real" and "ptr int" never are.
 *  Similarly for the other situations.
 */

Bool
compat (g1, g2, c)
Sigptr g1, g2;
Compat c;
{
    Type t1, t2;
    Symptr s1, s2, o1, o2;
    Parptr m1, m2;

    if (g1 == g2)			/* easiest case is identical sigs */
	return TRUE;

    if (!g1 || !g2)			/* if (just) one null, call mismatch */
	return FALSE;
    
    t1 = g1->g_type;
    t2 = g2->g_type;

    if (t2 == T_ANY && c != C_EXACT)
	return t1 != T_VOID;

    switch (t1) {

	case T_ANY:
	    if (c == C_EXACT)
		return t2 == T_ANY;
	    else
		return t2 != T_VOID;

	case T_NLIT:
	    if (t2 == T_NLIT)
		return TRUE;
	    else
		return compat (g2, g1, c);

	case T_VCAP:
	case T_FILE:
	    return t2 == t1 || t2 == T_NLIT;

	case T_VOID:
	case T_BOOL:
	case T_CHAR:
	    return t2 == t1;

	case T_INT:
	    return t2 == T_INT || (t2 == T_REAL && c == C_COMPARE);
	case T_REAL:
	    return t2 == T_REAL || (t2 == T_INT && c != C_EXACT);

	case T_STRING:
	    if (t2 != T_STRING)
		return FALSE;
	    if (c != C_EXACT)
		return TRUE;
	    if (g1->g_ub->e_opr != O_ILIT || g2->g_ub->e_opr != O_ILIT)
		return TRUE;	/* can't validate lengths */
	    return g1->g_ub->e_int == g2->g_ub->e_int;

	case T_ARRAY:
	    if (t2 != T_ARRAY)
		return FALSE;
	    g1 = g1->g_usig;  t1 = g1->g_type;
	    g2 = g2->g_usig;  t2 = g2->g_type;
	    if (c == C_ASSIGN && t1 == T_STRING && t2 == T_STRING)
		return TRUE;	/* string array elems need not be same size */
	    if (!rcompat (g1, g2, C_EXACT))
		return FALSE;
	    return TRUE;

	case T_OP:
	    if (t2 == T_OCAP)
		return compat (g1, g2->g_usig, c);
	    if (t2 != T_OP)
		return t2 == T_NLIT;
	    if (g1->g_proto->p_rstr != g2->g_proto->p_rstr)
		return FALSE;
	    m1 = g1->g_proto->p_param;
	    m2 = g2->g_proto->p_param;
	    if (m1 == m2)
		return TRUE;
	    while (m1 != NULL && m2 != NULL) {
		if (!rcompat (m1->m_sig, m2->m_sig, C_EXACT))
		    return FALSE;
		m1 = m1->m_next;
		m2 = m2->m_next;
	    }
	    return m1 == m2;
	    
	case T_RES:
	    if (t2 != T_RES)
		return t2 == T_NLIT;
	    /* for resources, the exported ops must be compatible */
	    /* note that the param lists DO NOT have to be compatible */
	    s1 = o1 = g1->g_sym;
	    s2 = o2 = g2->g_sym;
	    for (;;) {
		do {			/* find next op in resource 1 */ 
		    o1 = o1->s_next;
		} while (o1 != NULL && (o1->s_imp != s1 || o1->s_kind != K_OP));

		do {			/* find next op in resource 2 */ 
		    o2 = o2->s_next;
		} while (o2 != NULL && (o2->s_imp != s2 || o2->s_kind != K_OP));

		if (o1 == NULL || o2 == NULL)
		    return o1 == o2;	/* quit at end of symtab */

		if (!compat (o1->s_op->o_asig, o2->s_op->o_asig, C_EXACT))
		    return FALSE;
	    }

	case T_ENUM:
	    if (t2 != T_ENUM)
		return FALSE;
	    return esize (g1) == esize (g2);

	case T_REC:
	    if (t2 != T_REC)
		return FALSE;
	    s1 = g1->g_sym;
	    s2 = g2->g_sym;
	    if (s1 == s2)
		return TRUE;
	    while (s1 != NULL && s2 != NULL) {
		if (!rcompat (s1->s_var->v_sig, s2->s_var->v_sig, C_EXACT))
		    return FALSE;
		s1 = s1->s_next;
		s2 = s2->s_next;
	    }
	    return s1 == s2;

	case T_PTR:
	    return t2 == T_NLIT
		|| (t2 == T_PTR && g2->g_usig->g_type == T_ANY)
		|| (t2 == T_PTR && rcompat (g1->g_usig, g2->g_usig, C_EXACT))
		|| (g1->g_usig->g_type == T_ANY && t2 == T_PTR);

	case T_RCAP:
	    if (t2 == T_RCAP)
		return rcompat (g1->g_usig, g2->g_usig, C_EXACT);
	    else
		return t2 == T_NLIT;

	case T_OCAP:
	    if (t2 == T_OP)
		return compat (g1->g_usig, g2, c);
	    else if (t2 == T_OCAP)
		return rcompat (g1->g_usig, g2->g_usig, C_EXACT);
	    else
		return t2 == T_NLIT;

	default:
	    BOOM ("bad sig in compat()", "");
	    /* NOTREACHED */
    }
}



/*  rcompat (g1, g2, c) -- call compat recursively, up to a limit  */

#define RECURSION_LIMIT 5

static Bool
rcompat (g1, g2, c)
Sigptr g1, g2;
Compat c;
{
    static int depth = 0;
    Bool r;

    depth++;
    if (depth > RECURSION_LIMIT)
	r = TRUE;
    else
	r = compat (g1, g2, c);
    depth--;
    return r;
}



/*  ndim (g) -- return number of dimensions of signature g  */

int
ndim (g)
Sigptr g;
{
    int n = 0;
    while (g->g_type == T_ARRAY) {
	g = g->g_usig;
	n++;
    }
    return n;
}



/*  usig (g) -- return the base signature of the array type g  */

Sigptr
usig (g)
Sigptr g;
{
    while (g->g_type == T_ARRAY)
	g = g->g_usig;
    return g;
}



/*  esize (g) -- count the number of members of enum g  */

static int
esize (g)
Sigptr g;
{
    Symptr s;
    int n;

    ASSERT (g->g_type == T_ENUM);
    ASSERT (g->g_sym->s_kind == K_ELIT);
    g = g->g_sym->s_sig;		/* get original anchor for this enum */
    ASSERT (g->g_type == T_ENUM);
    n = 0;
    for (s = g->g_sym; s != NULL && s->s_sig == g; s = s->s_next)
	n++;
    return n;
}
