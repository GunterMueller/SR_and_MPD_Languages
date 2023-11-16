/*  print.c -- print structures on stdout for debugging  */



#include <ctype.h>
#include "compiler.h"
#include "print.h"

static void pnode	PARAMS ((Nodeptr e));
static void plist	PARAMS ((Nodeptr e));
static void pseq	PARAMS ((Nodeptr e));
static void ppro	PARAMS ((Proptr p));
static void pvar	PARAMS ((Varptr v));
static char *contos	PARAMS ((Nodeptr node));
static char *vartos	PARAMS ((Varptr v));
static char *getname	PARAMS ((char **a, int n, char *s, int i));



#define DENT 2
#define INDENT spacing -= DENT
#define UNDENT spacing += DENT

static char spaces[] =
"                                                                            ";
static char *spacing = spaces + sizeof (spaces) - 1;



/*  ptree(e) -- print a node and its descendents  */

void
ptree (e)
Nodeptr e;
{
    printf (spacing);
    pnode (e);
    putchar ('\n');
    if (!e)
	return;
    INDENT;
    if (e->e_opr == O_LIST)
	plist (e);
    else if (e->e_opr == O_SEQ)
	pseq (e);
    else {
	switch (leftchild [(int) e->e_opr]) {
	    case CH_NODE:
		if (e->e_l)
		    ptree (e->e_l);
		break;
	    case CH_SYM:
		printf (spacing);
		psym (e->e_sym);
		putchar ('\n');
		break;
	    case CH_VAR:
		printf (spacing);
		pvar (e->e_var);
		putchar ('\n');
		break;
	    default:
		break;
	}
	if (e->e_r != NULL)
	    ptree (e->e_r);
    }
    UNDENT;
}



/*  pnode(e) -- print short information about a node  */

static void
pnode (e)
Nodeptr e;
{
    Operator t;
    int l;

    if (!e) {
	printf ("null Nodeptr");
	SFLUSH (stdout);
	return;
    }
    t = e->e_opr;
    printf ("%s (", oprtos (t));

    switch (leftchild [(int) t]) {
	case CH_NIL:
	    if (e->e_l)
		printf ("%08X [?!]", e->e_l);
	    break;
	case CH_NODE:
	    printf ("%s", e->e_l ? oprtos (e->e_l->e_opr) : "NULL");
	    break;
	case CH_SYM:
	    printf ("\"%s\"", symtos (e->e_sym));
	    break;
	case CH_VAR:
	    printf ("%s", vartos (e->e_var));
	    break;
	case CH_INT:
	case CH_REAL:
	    printf ("%s", contos (e));
	    break;
	case CH_STR:
	    l = e->e_sptr->length;
	    wescape (stdout, DATA (e->e_sptr), (l <= 15 ? l : 12), '"');
	    if (l > 15)
		printf ("...");
	    printf (",%d", l);
	    break;
	case CH_NAME:
	    if (e->e_name)
		printf ("\"%s\"", e->e_name);
	    else
		printf ("NULL");
	    break;
    }

    if (e->e_r)
	printf (", %s", oprtos (e->e_r->e_opr));
    printf (") ");
    if ((e->e_locn & E_FILE) != 0)
	printf ("f%d", (e->e_locn & E_FILE) >> E_FSHIFT);
    printf ("l%d: ", e->e_locn & E_LINE);
    psig (e->e_sig);
    SFLUSH (stdout);
}



/*  plist(e) -- print a tree of LIST nodes without indentation  */

static void
plist (e)
Nodeptr e;
{
    while (e != NULL && e->e_opr == O_LIST) {
	ptree (e->e_l);
	e = e->e_r;
    }
    if (e != NULL) {
	printf (spacing);
	printf ("**** final rnode:\n");
	INDENT;
	ptree (e);
	UNDENT;
    }
}



/*  pseq(e) -- print a tree of SEQ nodes without indentation  */

static void
pseq (e)
Nodeptr e;
{
    if (!e)
	return;
    if (e->e_opr == O_SEQ) {
	pseq (e->e_l);
	pseq (e->e_r);
    } else
	ptree (e);
}



/*  pstab(s) -- print a symbol table beginning with symbol s  */

void
pstab (s)
Symptr s;
{
    while (s != NULL) {
	printf (spacing);
	psym (s);
	putchar ('\n');
	if (s->s_kind != K_IMP && s->s_child != NULL) {
	    INDENT;
	    pstab (s->s_child);
	    UNDENT;
	}
	s = s->s_next;
    }
}



/*  psym(s) -- print a single symbol struct  */

void
psym (s)
Symptr s;
{
    if (!s) {
	printf ("null Symptr");
	SFLUSH (stdout);
	return;
    }
    printf ("SYM%d %s", s->s_depth, symtos (s));
    if (s->s_kind == K_IMP) {
	s = s->s_child;
	printf ("->%s", symtos (s));
    }
    if (s->s_gname != s->s_name && s->s_gname != NULL)
	printf ("(%s)", s->s_gname);
    printf (": %s ", kindtos (s->s_kind));
    switch (s->s_kind) {
	case K_ELIT:
	    printf ("=%d ", s->s_seq);
	    psig (s->s_sig);
	    break;
	case K_TYPE:
	case K_RES:
	case K_OP:
	case K_FVAL:
	case K_FVAR:
	case K_FRES:
	case K_FREF:
	    psig (s->s_sig);
	    break;
	case K_VAR:
	    pvar (s->s_var);
	    break;
	case K_BLOCK:
	    if (s->s_pb != NULL)
		printf ("%s->", s->s_pb);
	    break;
    }
    if (s->s_prev != NULL && s->s_prev->s_next != s && s->s_prev->s_child != s)
	printf (" *** prev->%s", symtos (s->s_prev));
    SFLUSH (stdout);
}



/*  psig(g) -- print synopsis of signature  */

void
psig (g)
Sigptr g;
{
    int limit;
    Type t;
    Recptr k;

    limit = 10;
    if (g == NULL) {
	printf ("NOSIG");
	SFLUSH (stdout);
	return;
    }
    while (g != NULL && --limit > 0) {
	t = g->g_type;
	if (t == T_ARRAY || t == T_STRING) {		/* print bounds */
	    if (t == T_STRING)
		printf ("T_STRING");
	    printf ("[");
	    if (t == T_ARRAY) {
		if (g->g_lb != NULL && g->g_lb->e_opr == O_ILIT)
		    printf ("%d", g->g_lb->e_int);
		if (g->g_lb != NULL)
		    printf (":");
	    }
	    if (g->g_ub != NULL && g->g_ub->e_opr == O_ILIT)
		printf ("%d", g->g_ub->e_int);
	    printf ("]");
	} else {
	    printf ("%s", typetos (t));		/* for op/res, print params */
	    if (t == T_OP || t == T_RES || t == T_GLB) { 
		ppro (g->g_proto);
		if (g->g_usig)
		    printf (":");
	    } else if (t == T_REC) {
		if ((k = g->g_rec) != NULL)
		    printf ("(%s)", k->k_tdef ? k->k_tdef : "???");
	    } else if (g->g_usig)
		printf (".");
	}
	g = g->g_usig;
    }
    if (g)
	printf ("..");
    SFLUSH (stdout);
}



/*  ppro(p) -- print a prototype  */

static void
ppro (p)
Proptr p;
{
    Parptr m;

    if (p == NULL) {
	printf ("(NO PROTOTYPE)");
	return;
    }
    printf ("(");
    m = p->p_param;
    if (m != NULL)
	m = m->m_next;		/* skip return val; is printed by caller */
    while (m != NULL) {
	if (m->m_sig == NULL)
	    printf ("NULLSIG");
	else
	    printf ("%s", typetos (m->m_sig->g_type));
	m = m->m_next;
	if (m != NULL)
	    printf (",");
    }
    printf (")");
    switch (p->p_rstr) {
	case O_RESOURCE:					break;
	case O_RNONE:						break;
	case O_RCALL:	printf ("{call}");			break;
	case O_RSEND:	printf ("{send}");			break;
	default:	printf ("{%s}", oprtos (p->p_rstr));	break;
    }
}



/*  pvar(v) -- print a var struct  */

static void
pvar (v)
Varptr v;
{
    if (!v) {
	printf ("null Varptr");
	SFLUSH (stdout);
	return;
    }
    printf ("VAR");
    if (v->v_sym)
	printf (" \"%s\"", symtos (v->v_sym));
    printf (": %s ", varitos (v->v_vty));
    if (v->v_seq)	
	printf ("#%d ", v->v_seq);
    if (v->v_set)	
	putchar ('s');
    if (v->v_used)	
	putchar ('u');
    putchar (' ');
    psig (v->v_sig);
    if (v->v_value) {
	printf (" := ");
	pnode (v->v_value);
    }
    SFLUSH (stdout);
}



/*  contos(node) -- return a string representation of a constant node  */

static char *
contos (node)
Nodeptr node;
{
    static char buf[50];

    if (!node)
	return "(NODELESS CONST!)";
    if (!node->e_sig)
	sprintf (buf, "untyped %d", node->e_int);
    else switch (node->e_sig->g_type) {
	case T_BOOL:
	    return node->e_int ? "true": "false";
	case T_CHAR:
	    if (node->e_int == '\'' || node->e_int == '\\')
		sprintf (buf, "'\\%c'", node->e_int);
	    else if (isprint (node->e_int))
		sprintf (buf, "'%c'", node->e_int);
	    else
		sprintf (buf, "'\\x%02x'", node->e_int);
	    break;
	case T_INT:
	    sprintf (buf, "%d", node->e_int);
	    break;
	case T_ENUM:
	    sprintf (buf, "enum(%d)", node->e_int);
	    break;
	case T_REAL:
	    if (node->e_rptr)
		sprintf (buf, "%#g", *node->e_rptr);
	    else
		return "(REALVAL MISSING)";
	    break;
	default:
	    sprintf (buf, "const(%s,%d)",
		typetos (node->e_sig->g_type), node->e_int);
	    break;
    }
    return buf;
}



/*  vartos(v) -- return a string representation of a var  */

static char *
vartos (v)
Varptr v;
{
    static char buf[50];

    sprintf (buf, "var \"%s\"", symtos (v->v_sym));
    return buf;
}



/*  symtos(s) -- return a string representation of a symbol  */
char *
symtos (s)
Symptr s;
{
    static char buf[50];
    char *bp;

    if (!s)
	return "[NOSYM]";
    if (!s->s_name)
	return "[anon]";
    bp = buf;
    if (s->s_imp) {
	strcpy (buf, symtos (s->s_imp));
	bp += strlen (buf);
	*bp++ = '.';
    }
    sprintf (bp, "%.20s", s->s_name);
    return buf;
}



/*  xxxtos -- convert enum xxx to a string, with error checking  */

#define NELEM(a) (sizeof(a)/sizeof(a[0]))

char *
oprtos (mytok)
Operator mytok;
{
    return getname (operator_names, NELEM (operator_names), "O", (int) mytok);
}

char *
kindtos (kind)
Kind kind;
{
    return getname (kind_names, NELEM (kind_names), "K", (int) kind);
}

char *
typetos (type)
Type type;
{
    return getname (type_names, NELEM (type_names), "T", (int) type);
}

char *
varitos (vty)
Variety vty;
{
    return getname (variety_names, NELEM (variety_names), "V", (int) vty);
}



/*  getname(a,n,s,i) -- get name from table a of size n, prefix s, element i  */

static char *
getname (a, n, s, i)
char **a;
int n;
char *s;
int i;
{
#define NAMBUFS 5
    static char buf[NAMBUFS][20];	/* 5 buffers used in rotation */
    static int b = 0;			/* next buffer to use */

    if (i >= 0 && i < n)		/* if n is in range */
	return a[i];			/* return symbolic name */

    b = (b + 1) % NAMBUFS;		/* if not, select buffer */
    sprintf (buf[b], "%s_?%d?", s, n);	/* format string repr of n */
    return buf[b];			/* return pointer to buffer */
}
