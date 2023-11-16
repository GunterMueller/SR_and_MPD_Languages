/*  pregen.c -- generate code for predefined operations  */

#include "compiler.h"

static Nodeptr filearg	PARAMS ((Nodeptr *a, int dfault));
static void genminmax	PARAMS ((char *which, Nodeptr args));
static void gengarg	PARAMS ((Nodeptr args));
static void genwrite	PARAMS ((Predef f, Nodeptr args, Nodeptr parent));
static void argtypes	PARAMS ((Nodeptr args));



/*  pregen (e) -- generate code for predefined op called by tree e  */

void
pregen (e)
Nodeptr e;
{
    Symptr s;
    Predef f;
    Nodeptr fd, l, r;
    int n;

    ASSERT (e != NULL && e->e_opr == O_LIBCALL);
    ASSERT (e->e_l->e_opr == O_SYM && e->e_l->e_sym->s_kind == K_PREDEF);
    s = e->e_l->e_sym;
    f = s->s_pre;
    r = e->e_r;

    switch (f) {

	case PRE_lb:
	case PRE_ub:
	    ASSERT (r->e_r->e_l->e_opr == O_ILIT);
	    n = r->e_r->e_l->e_int;
	    cprintf ("%s(%e,%d-%d)", (f == PRE_ub) ? "UB" : "LB",
		r->e_l, ndim (r->e_l->e_sig), n);
	    return;
	case PRE_length:
	    ASSERT (r->e_l->e_sig->g_type == T_STRING);
	    cprintf ("%e->length", r->e_l);
	    return;
	case PRE_maxlength:
	    ASSERT (r->e_l->e_sig->g_type == T_STRING);
	    cprintf ("%e->size-%d", r->e_l, STRING_OVH);
	    return;
	case PRE_chars:
	    if (r->e_l->e_sig->g_type != T_STRING) {
		/* convert it first to string */
		r->e_l = bnode(O_CAST,unode(O_STRING,newnode(O_ASTER)),r->e_l);
		attest (r->e_l);
	    }
	    ASSERT (r->e_l->e_sig->g_type == T_STRING);
	    cprintf ("mpd_chars(%e)", r->e_l);
	    return;

	case PRE_pred:
	    cprintf ("(%e-1)", r->e_l);
	    return;
	case PRE_succ:
	    cprintf ("(%e+1)", r->e_l);
	    return;
	case PRE_abs:
	    if (r->e_l->e_sig->g_type == T_REAL)
		cprintf ("fabs(%e)", r->e_l);
	    else
		cprintf ("abs(%e)", r->e_l);
	    return;
	case PRE_min:
	    genminmax ("min", r);
	    return;
	case PRE_max:
	    genminmax ("max", r);
	    return;

	case PRE_nap:
	    cprintf ("mpd_nap(%t,%e)", r, r->e_l);
	    return;
	case PRE_age:
	    cprintf ("mpd_age()");
	    return;
	case PRE_setpriority:
	    cprintf ("mpd_setpri(%e)", r->e_l);
	    return;
	case PRE_mypriority:
	    cprintf ("mpd_mypri()");
	    return;
	case PRE_myresource:
	    cprintf ("(*(Rcap_%s*)rv)", curres->r_sym->s_gname);
	    return;
	case PRE_myvm:
	    cprintf ("mpd_my_vm");
	    return;
	case PRE_mymachine:
	    cprintf ("mpd_my_machine");
	    return;
	case PRE_locate:
	    cprintf ("mpd_locate(%t,%e,%S,", r, r->e_l, r->e_r->e_l);
	    if (r->e_r->e_r != NULL)
		cprintf ("%S)", r->e_r->e_r->e_l);
	    else
		cprintf ("(String*)0)");
	    return;
	case PRE_free:
	    if (r->e_l->e_sig->g_type == T_NLIT)
		cprintf ("0");			/* free(null) gens null expr */
	    else
		cprintf ("mpd_dispose(%t,%e)", r, r->e_l);
	    return;

	case PRE_sqrt:
	    cprintf ("sqrt(%e)", r->e_l);
	    return;
	case PRE_ceil:
	    cprintf ("ceil(%e)", r->e_l);
	    return;
	case PRE_floor:
	    cprintf ("floor(%e)", r->e_l);
	    return;
	case PRE_round:
	    cprintf ("mpd_round(%t,%e)", r, r->e_l);
	    return;

	case PRE_seed:
	    cprintf ("mpd_seed(%e)", r->e_l);
	    return;
	case PRE_random:
	    if (r == NULL)
		cprintf ("mpd_random(0.0,1.0)");
	    else if (r->e_r == NULL)
		cprintf ("mpd_random(0.0,%e)", r->e_l);
	    else
		cprintf ("mpd_random(%e,%e)", r->e_l, r->e_r->e_l);
	    return;

	case PRE_log:
	    if (r->e_r == NULL)
		cprintf ("log(%e)", r->e_l);
	    else {
		l = r->e_r->e_l;
		if (l->e_opr == O_RLIT && *l->e_rptr == 10.0)
		    cprintf ("log10(%e)", r->e_l);
		else
		    cprintf ("(log(%e)/log(%e))", r->e_l, l);
	    }
	    return;
	case PRE_exp:
	    if (r->e_r != NULL)
		cprintf ("pow(%e,%e)", r->e_r->e_l, r->e_l);
	    else
		cprintf ("exp(%e)", r->e_l);
	    return;

	/* we do not check for trig function arguments out of bounds. */
	case PRE_sin:
	    cprintf ("sin(%e)", r->e_l);
	    return;
	case PRE_cos:
	    cprintf ("cos(%e)", r->e_l);
	    return;
	case PRE_tan:
	    cprintf ("tan(%e)", r->e_l);
	    return;
	case PRE_asin:
	    cprintf ("asin(%e)", r->e_l);
	    return;
	case PRE_acos:
	    cprintf ("acos(%e)", r->e_l);
	    return;
	case PRE_atan:
	    if (r->e_r != NULL)
		cprintf ("atan2(%e,%e)", r->e_l, r->e_r->e_l);
	    else
		cprintf ("atan(%e)", r->e_l);
	    return;

	case PRE_numargs:
	    cprintf ("mpd_numargs()");
	    return;
	case PRE_getarg:
	    gengarg (r);
	    return;

	case PRE_open:
	    cprintf ("mpd_open(%S,%e)", r->e_l, r->e_r->e_l);
	    return;
	case PRE_flush:
	    cprintf ("mpd_flush(%t,%e)", r, r->e_l);
	    return;
	case PRE_close:
	    cprintf ("mpd_close(%t,%e)", r, r->e_l);
	    return;
	case PRE_remove:
	    cprintf ("mpd_remove(%S)", r->e_l);
	    return;
	case PRE_seek:
	    cprintf ("mpd_seek(%t,%e,%e,%e)",
		r, r->e_l, r->e_r->e_l, r->e_r->e_r->e_l);
	    return;
	case PRE_where:
	    cprintf ("mpd_where(%t,%e)", r, r->e_l);
	    return;

	case PRE_get:
	    fd = filearg (&r, STDIN);
	    if (r->e_l->e_sig->g_type == T_STRING)
		cprintf ("mpd_get_string(%t,%e,%e)", r, fd, r->e_l);
	    else {
		ASSERT (r->e_l->e_sig->g_type == T_ARRAY);
		ASSERT (r->e_l->e_sig->g_usig->g_type == T_CHAR);
		cprintf ("mpd_get_carray(%t,%e,%e)", r, fd, r->e_l);
	    }
	    return;

	case PRE_read:
	    fd = filearg (&r, STDIN);
	    cprintf ("mpd_read(%t%,%e%,", e, fd);
	    argtypes (r);
	    for (;  r != NULL;  r = r->e_r)
		cprintf ("%,%a", r->e_l);
	    cprintf (")");
	    return;

	case PRE_writes:
	case PRE_write:
	case PRE_put:
	    genwrite (f, r, e);
	    return;
	case PRE_printf:
	    fd = filearg (&r, STDOUT);
	    cprintf ("mpd_printf(%t%,%e%,(String*)0,%S%,", r, fd, r->e_l); 
	    argtypes (r->e_r);
	    for (l = r->e_r;  l;  l = l->e_r)
		cprintf ("%,%e", l->e_l);
	    cprintf (")");
	    return;

	case PRE_sprintf:
	    cprintf("mpd_printf(%t%,(File*)0,%S%,%S%,", r, r->e_l, r->e_r->e_l);
	    argtypes (r->e_r->e_r);
	    for (l = r->e_r->e_r;  l;  l = l->e_r)
		cprintf ("%,%e", l->e_l);
	    cprintf (")");
	    return;

	case PRE_scanf:
	    fd = filearg (&r, STDIN);
	    cprintf ("mpd_scanf(%t%,%e%,(String*)0%,%S%,", r, fd, r->e_l);
	    argtypes (r->e_r);
	    for (l = r->e_r;  l;  l = l->e_r)
		cprintf ("%,%a", l->e_l);
	    cprintf (")");
	    return;

	case PRE_sscanf:
	    cprintf ("mpd_scanf(%t%,(File*)0%,%S%,%S%,",r, r->e_l, r->e_r->e_l);
	    argtypes (r->e_r->e_r);
	    for (l = r->e_r->e_r;  l;  l = l->e_r)
		cprintf ("%,%a", l->e_l);
	    cprintf (")");
	    return;

	default:
	    BOOM ("unrecognized predef", symtos (s));
    }
}



/*  genminmax (which, args) -- generate min or max call  */

static void
genminmax (which, args)
char *which;
Nodeptr args;
{
    int nargs;
    Nodeptr a;
    char c;

    nargs = 0;
    for (a = args;  a;  a = a->e_r)
	nargs++;

    c = (args->e_l->e_sig->g_type == T_REAL) ? 'r' : 'i';
    cprintf ("mpd_%c%s(%d", c, which, nargs);

    for (a = args; a; a = a->e_r)
	cprintf ("%,%e", a->e_l);
    cprintf (")");
}



/*  gengarg (args) -- generate getarg call  */

static void
gengarg (args)
Nodeptr args;
{
    Nodeptr e1, e2;
    Type t;
    char *func;

    e1 = args->e_l;
    e2 = args->e_r->e_l;
    t = e2->e_sig->g_type;
    switch (t) {
	case T_BOOL:	func = "mpd_arg_bool";	break;
	case T_CHAR:	func = "mpd_arg_char";	break;
	case T_INT:	func = "mpd_arg_int";	break;
	case T_ENUM:	func = "mpd_arg_int";	break;
	case T_REAL:	func = "mpd_arg_real";	break;
	case T_PTR:	func = "mpd_arg_ptr";	break;
	case T_ARRAY:	func = "mpd_arg_carray"; break;	/* array of chars */
	case T_STRING:	func = "mpd_arg_string"; break;
	default:	BOOM ("bad type in getarg gen", typetos (t));
    }
    cprintf ("%s(%e,%a)", func, e1, e2);
}



/*  genwrite (func, arglist) -- generate write/writes/put call  */

static void
genwrite (func, args, parent)
Predef func;
Nodeptr args;
Nodeptr parent;
{
    Nodeptr fd, a;
    char format[1000], *f;
    static int fmtnum = 0;
    
    fd = filearg (&args, STDOUT);
    cprintf ("mpd_printf(%t%,%e%,(String*)0%,", parent, fd); 

    /* generate output format depending on arguments */
    f = format;
    for (a = args;  a;  a = a->e_r)  {
	*f++ = '%';
	switch (a->e_l->e_sig->g_type)  {
	    case T_BOOL:		*f++ = 'b';	break;
	    case T_CHAR:		*f++ = 'c';	break;
	    case T_INT:			*f++ = 'd';	break;
	    case T_ENUM:		*f++ = 'd';	break;
	    case T_REAL:   *f++ = '#';	*f++ = 'g';	break;
	    case T_PTR:			*f++ = 'p';	break;
	    case T_STRING:		*f++ = 'r';	break;
	    case T_ARRAY:		*f++ = 'r';	break;
	    default:	;  /* no problem, gets error in argtypes() below */
	}
	if (func == PRE_write)
	    *f++ = ' ';		/* add space separator for write() only */
    }
    if (func == PRE_write)  {
	if (f > format)
	    f--;		/* remove trailing space */
	*f++ = '\\';
	*f++ = 'n';		/* insert newline */
    }
    *f++ = '\0';
    fmtnum++;
    cprintf ("%7STRING(f%d,%d,\"%s\");\n", fmtnum, strlen (format), format);
    cprintf ("(String*)&f%d%,", fmtnum);

    argtypes (args);

    /* generate arguments */
    for (a = args;  a;  a = a->e_r)
	cprintf ("%,%e", a->e_l);
    cprintf (")");
}



/*  argtypes (args) -- generate string constant indicating arg types  */

static void
argtypes (a)
Nodeptr a;
{
    char buf[100], *s = buf;

    while (a) {
	switch (a->e_l->e_sig->g_type) {
	    case T_BOOL:    *s++ = 'b';  break;
	    case T_CHAR:    *s++ = 'c';  break;
	    case T_INT:     *s++ = 'i';  break;
	    case T_ENUM:    *s++ = 'e';  break;
	    case T_REAL:    *s++ = 'r';  break;
	    case T_PTR:     *s++ = 'p';  break;
	    case T_STRING:  *s++ = 's';  break;
	    case T_ARRAY:   *s++ = 'a';  break;  /* must be array of char */
	    default: BOOM ("illegal type in arglist", "");
	};
	a = a->e_r;
    }
    *s = '\0';
    cprintf ("\"%s\"", buf);
}



/*  filearg (&a) -- return file node; use & advance arg pointer if a file  */

static Nodeptr 
filearg (a, dfault)
Nodeptr *a;
int dfault;
{
    Nodeptr e = *a;

    if (e != NULLNODE && e->e_l->e_sig->g_type == T_FILE) {
	*a = e->e_r;
	return e->e_l;
    } else {
	e = newnode (O_FLIT);
	e->e_sig = file_sig;
	e->e_int = (int) dfault;
	return e;
    }
}
