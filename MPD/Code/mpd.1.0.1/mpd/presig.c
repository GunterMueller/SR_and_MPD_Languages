/*  presig.c -- set & check signatures for predefined function calls */

#include "compiler.h"

static int expect	PARAMS ((Nodeptr e, int argno, Sigptr g));
static int ioarg	PARAMS ((Nodeptr e));
static void iolist	PARAMS ((Nodeptr e, int n, int lval));
static void fixlub	PARAMS ((Nodeptr e));
static Sigptr badarg	PARAMS ((Nodeptr e, int argno, char *msg));



/* table of pointers to the "usual" signatures of the builtin functions */
static struct {
    int minn;
    int maxn;
} info[] = {
    { 0, 0 },		/* for bogus value */
#define premac(name,minn,maxn) {minn, maxn},
#include "predefs.h"
#undef premac
};



/*  presig (e) - check args and set signature for invocation e of a predef  */

Sigptr
presig (e)
Nodeptr e;
{
    Sigptr g;
    char *name;
    Nodeptr a;
    int i, nargs, pnum;
    Nodeptr arg[MAX_FIXED_ARGS+1];	/* 1-based array of args */
#define ARG1 arg[1]
#define ARG2 arg[2]
#define ARG3 arg[3]

    ASSERT (e != NULL && e->e_opr == O_LIBCALL);
    ASSERT (e->e_l != NULL && e->e_l->e_opr == O_SYM);
    ASSERT (e->e_l->e_sym != NULL && e->e_l->e_sym->s_kind == K_PREDEF);

    pnum = (int) e->e_l->e_sym->s_pre;	/* get predef index */
    name = e->e_l->e_sym->s_name;	/* get function name */

    /* count arguments & save the first few */
    nargs = 0;
    for (a = e->e_r; a != NULL; a = a->e_r) {
	nargs++;
	if (nargs <= MAX_FIXED_ARGS)
	    arg[nargs] = a->e_l;
    }
    for (i = nargs + 1; i <= MAX_FIXED_ARGS; i++)
	arg[i] = NULL;			/* fill rest of fixed array with NULLs*/

    /* check the argument count */
    if (nargs < info[pnum].minn)
	return badarg (e, 0, "too few arguments");
    if (nargs > info[pnum].maxn && info[pnum].maxn != -1)
	return badarg (e, 0, "too many arguments");

    /* make function-specific checks */
    switch (pnum) {

	/* type stuff */
	case PRE_lb1:
	case PRE_lb2:
	case PRE_ub1:
	case PRE_ub2:
	    fixlub (e);				/* change to newstyle call */
	    return presig (e);			/* reprocess */

	case PRE_lb:
	case PRE_ub:
	    if (e->e_r->e_l->e_sig->g_type != T_ARRAY)
		return badarg (e, 1, "not an array");
	    if (nargs > 1) {
		if (ARG2->e_opr != O_ILIT)
		    badarg (e, 2, "not integer literal");
		else if (ARG2->e_int < 1)
		    badarg (e, 2, "value must be greater than zero");
	    }
	    if (nargs < 2)
		e->e_r->e_r = unode (O_LIST, (ARG2 = intnode (1)));
	    g = ARG1->e_sig;
	    i = ARG2->e_int;
	    while (i-- > 1) {
		g = g->g_usig;
		if (g->g_type != T_ARRAY)
		    return badarg (e, 2,
			"array does not have that many dimensions");
	    }
	    if (g->g_ub != NULL && g->g_ub->e_opr != O_ASTER)
		return g->g_ub->e_sig;
	    else if (g->g_lb != NULL && g->g_lb->e_opr != O_ASTER)
		return g->g_lb->e_sig;
	    else
		return int_sig;		/* no way to find sig of bounds */

	case PRE_pred:
	case PRE_succ:
	    g = ARG1->e_sig;
	    if (ORDERED (g) && g->g_type != T_REAL)
		return g;
	    else
		return badarg (e, 1, "invalid type");

	case PRE_length:
	case PRE_maxlength:
	    expect (e, 1, string_sig);
	    return int_sig;

	case PRE_chars:
	    g = ARG1->e_sig;
	    if (!CONVERTIBLE (g))
		badarg (e, 1, "illegal type for conversion");
	    return array_sig;

	/* resources etc. */
	case PRE_myresource:
	    return newsig (T_RCAP, curres->r_sig);

	case PRE_mymachine:
	    return int_sig;

	case PRE_myvm:
	    return vcap_sig;

	case PRE_mypriority:
	    return int_sig;

	case PRE_setpriority:
	    expect (e, 1, int_sig);
	    return void_sig;

	case PRE_locate:
	    expect (e, 1, int_sig);
	    expect (e, 2, string_sig);
	    if (nargs > 2)
		expect (e, 3, string_sig);
	    return void_sig;

	/* memory management */
	case PRE_free:
	    expect (e, 1, ptr_sig);
	    return void_sig;

	/* math */
	case PRE_abs:
	    if (e->e_r->e_l->e_sig->g_type == T_REAL)
		return real_sig;			/* handle real arg */
	    expect (e, 1, int_sig);			/* else must be int */
	    return int_sig;

	case PRE_min:
	case PRE_max:
	    g = e->e_r->e_l->e_sig;
	    switch (g->g_type) {
		case T_BOOL:
		case T_CHAR:
		case T_ENUM:
		    for (i = 1; i <= nargs; i++)
			expect (e, i, g);		/* all must agree */
		    return g;
		case T_INT:
		case T_REAL:
		    g = int_sig;			/* assume int result */
		    for (a = e->e_r; a != NULL; a = a->e_r)
			if (a->e_l->e_sig->g_type == T_REAL) {
			    g = real_sig;		/* no, is real */
			    break;
			}
		    for (i = 1; i <= nargs; i++)
			expect (e, i, g);		/* now check args */
		    return g;				/* return corr type */
		default:
		    expect (e, 1, int_sig);		/* is illegal type */
		    return int_sig;
	    }

	case PRE_ceil:
	case PRE_floor:
	case PRE_round:
	case PRE_sqrt:
	case PRE_log:
	case PRE_exp:
	case PRE_sin:
	case PRE_cos:
	case PRE_tan:
	case PRE_asin:
	case PRE_acos:
	case PRE_atan:
	case PRE_random:
	    for (i = 1; i <= nargs; i++)
		expect (e, i, real_sig);
	    return real_sig;

	case PRE_seed:
	    expect (e, 1, real_sig);
	    return void_sig;

	/* argument operations */
	case PRE_numargs:
	    return int_sig;

	case PRE_getarg:
	    expect (e, 1, int_sig);
	    iolist (e, 2, TRUE);
	    return int_sig;

	/* timer functions */
	case PRE_nap:
	    expect (e, 1, int_sig);
	    return void_sig;

	case PRE_age:
	    return int_sig;

	/* basic I/O */
	case PRE_open:
	    expect (e, 1, string_sig);
	    if (!compat (ARG2->e_sig, mode_sig, C_COMPARE))
		badarg (e, 2, "accessmode argument is wrong type");
	    return file_sig;

	case PRE_flush:
	case PRE_close:
	    expect (e, 1, file_sig);
	    return void_sig;

	case PRE_remove:
	    expect (e, 1, string_sig);
	    return bool_sig;

	case PRE_get:
	    if (nargs > 1)
		expect (e, 1, file_sig);
	    if (!easylval (arg[nargs]))
		EFATAL (arg[nargs], "result arg must be a variable");
	    expect (e, nargs, array_sig);
	    return int_sig;

	case PRE_put:
	    if (nargs > 1)
		expect (e, 1, file_sig);
	    expect (e, nargs, array_sig);
	    return void_sig;

	case PRE_seek:
	    expect (e, 1, file_sig);
	    if (!compat (ARG2->e_sig, seek_sig, C_COMPARE))
		badarg (e, 2, "seektype argument is wrong type");
	    expect (e, 3, int_sig);
	    return int_sig;

	case PRE_where:
	    expect (e, 1, file_sig);
	    return int_sig;

	/* formatted I/O */
	case PRE_read:
	    i = ioarg (e);
	    iolist (e, i, TRUE);
	    return int_sig;

	case PRE_scanf:
	    i = ioarg (e);
	    expect (e, i, string_sig);
	    iolist (e, i + 1, TRUE);
	    return int_sig;

	case PRE_sscanf:
	    expect (e, 1, string_sig);
	    expect (e, 2, string_sig);
	    iolist (e, 3, TRUE);
	    return int_sig;

	case PRE_write:
	case PRE_writes:
	    i = ioarg (e);
	    iolist (e, i, FALSE);
	    return void_sig;

	case PRE_printf:
	    i = ioarg (e);
	    expect (e, i, string_sig);
	    iolist (e, i + 1, FALSE);
	    return void_sig;

	case PRE_sprintf:
	    if (!easylval (ARG1))
		EFATAL (e, "sprintf(), argument 1: not a variable");
	    expect (e, 1, string_sig);
	    expect (e, 2, string_sig);
	    iolist (e, 3, FALSE);
	    return void_sig;

	default:
	    BOOM ("undefined predef in presig()", name);
	    /* NOTREACHED */
	    return 0; 
    }
}



/*  expect (e, n, g) -- check that arg n of invocation e is compat w/ sig g.
 *
 *  returns TRUE iff the argument is legal; and writes error message if not.
 */
static int
expect (e, n, g)
Nodeptr e;
int n;
Sigptr g;
{
    Type t;
    Nodeptr a;
    Sigptr h;
    int i;
    char *msg;

    a = e;
    for (i = 1; i <= n; i++) {
	a = a->e_r;
	if (a == NULL) {
	    /* catch, e.g., printf(stderr) here */
	    err (e->e_locn, "%s(): too few arguments", e->e_l->e_sym->s_name);
	    return FALSE;
	}
    }
    t = g->g_type;
    h = a->e_l->e_sig;
    if (h->g_type == t && t != T_ENUM && t != T_ARRAY)
	return TRUE;			/* is okay, easy case */

    switch (t) {
	case T_ENUM:
	    if (compat (g, h, C_COMPARE))
		return TRUE;
	    msg = "not an enum";
	    break;
	case T_INT:
	    msg = "not an integer";
	    break;
	case T_REAL:
	    if (h->g_type == T_INT) {	/* cast int to real, & accept */
		if (a->e_l->e_opr == O_ILIT) {
		    a->e_l = realnode ((Real) a->e_l->e_int);
		} else {
		    a->e_l = bnode (O_CAST, newnode (O_REAL), a->e_l);
		    a->e_l->e_sig = real_sig;
		}
		return TRUE;
	    }
	    msg = "not numeric";
	    break;
	case T_FILE:
	    if (h->g_type == T_NLIT) {
		a->e_l->e_sig = file_sig;
		return TRUE;
	    }
	    msg = "not a file";
	    break;
	case T_PTR:
	    if (h->g_type == T_NLIT) {
		a->e_l->e_sig = ptr_sig;
		return TRUE;
	    }
	    msg = "not a pointer";
	    break;
	case T_ARRAY:	/* signifies char array or string */
	    if (h->g_type == T_ARRAY && h->g_usig->g_type == T_CHAR)
		return TRUE;
	    if (h->g_type == T_STRING)
		return TRUE;
	    msg = "not a char array or string";
	    break;
	case T_STRING:
	    msg = "not a string";
	    break;
	default:
	    msg = "incorrect type";
	    break;
    }
    badarg (e, n, msg);
    return FALSE;
}



/*  ioarg (e) -- return the number of the first I/O argument, skipping file  */

static int
ioarg (e)
Nodeptr e;
{
    Type t;

    e = e->e_r;
    if (e == NULL)
	return 1;
    t = e->e_l->e_sig->g_type;
    if (t == T_FILE)
	return 2;
    else if (t == T_NLIT) {
	e->e_l->e_sig = file_sig;
	return 2;
    } else
	return 1;
}



/*  iolist (e, n, lval) -- check args n and beyond for legal I/O types;
 *  if lval is TRUE they must also be "easy" (non-slice) lvalues.
 */

static void
iolist (e, n, lval)
Nodeptr e;
int n, lval;
{
    Nodeptr a;
    Sigptr g;
    int i;
    char buf[100];

    a = e;
    for (i = 0; i < n; i++)
	if (a != NULL)
	    a = a->e_r;

    while (a != NULL) {
	g = a->e_l->e_sig;
	if (!CONVERTIBLE (g)) {
	    sprintf (buf, "%%s(), argument %d: invalid type", i);
	    err (a->e_l->e_locn, buf, e->e_l->e_sym->s_name);
	} else if (lval && !easylval (a->e_l)) {
	    sprintf (buf, "%%s(), argument %d: not a variable", i);
	    err (a->e_l->e_locn, buf, e->e_l->e_sym->s_name);
	}
	a = a->e_r;
	i++;
    }
}



/*  fixlub (e) - change lb1/ub1/lb2/ub2 call to newstyle call, with warning  */

static void
fixlub (e)
Nodeptr e;
{
    int n;
    char *old, *new;
    char buf[100];

    old = e->e_l->e_sym->s_name;
    switch (e->e_l->e_sym->s_pre) {
	case PRE_lb1:	new = "lb";  n = 1;  break;
	case PRE_ub1:	new = "ub";  n = 1;  break;
	case PRE_lb2:	new = "lb";  n = 2;  break;
	case PRE_ub2:	new = "ub";  n = 2;  break;
    }
    sprintf (buf, "%s() changed to %s(,%d)", old, new, n);
    err (E_WARN + e->e_locn, buf, NULLSTR);
    e->e_l = idnode (new);
    identify (e->e_l);
    attest (e->e_l);
    e->e_r->e_r = unode (O_LIST, intnode (n));
    e->e_r->e_r->e_l->e_opr = O_ILIT;
}



/*  badarg (e, n, msg) -- give msg for arg n of invocation e.
 *
 *  If n is nonzero, arg n is identified in the message and replaced by (int) 1.
 *  If n is zero, no arg is identified and the entire call is replaced by 1.
 *  int_sig is always returned.
 */

static Sigptr
badarg (e, n, msg)
Nodeptr e;
int n;
char *msg;
{
    char buf [100];

    if (n > 0) {
	sprintf (buf, "%s(), argument %d: %%s", e->e_l->e_sym->s_name, n);
	while (n--)
	    e = e->e_r;
	e = e->e_l;
    } else
	sprintf (buf, "%s(): %%s", e->e_l->e_sym->s_name);

    err (e->e_locn, buf, msg);		/* use locn of arg if n>0, else top */
    e->e_opr = O_ILIT;
    e->e_sig = int_sig;
    e->e_int = 1;
    e->e_r = NULL;
    return int_sig;
}
