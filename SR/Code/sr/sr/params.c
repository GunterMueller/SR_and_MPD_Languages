/*  params.c -- parameter list handling  */

#include "compiler.h"


static List prolist;			/* list of prototypes */
static int serial;			/* serial number */



/*  initprotos () -- initialize list of prototypes  */

void
initprotos ()
{
    prolist = list (sizeof (Proto));
    serial = 0;
}



/*  prototype (e) -- make a prototype given the parameter list
 *
 *  If e is a LIST node this is a resource parameter list.
 *  If e is a PROTO node this is an operation parameter list.
 */

Proptr
prototype (e)
Nodeptr e;
{
    Nodeptr d;
    Parptr m, *pm;
    Proptr p;
    int n;
    char buf[20];

    if (e == NULL || e->e_opr != O_PROTO)  {
	/* turn resource param list into op prototype form */
	e = bnode (O_PROTO, bnode (O_LIST, NULLNODE, e), newnode (O_RESOURCE));
    }

    p = (Proptr) lput (prolist);
    p->p_rstr = e->e_r->e_opr;
    sprintf (buf, "pb_%d", ++serial);
    p->p_def = rsalloc (buf);

    pm = &p->p_param;
    n = 0;
    for (e = e->e_l; e != NULL; e = e->e_r) {
	ASSERT (e->e_opr == O_LIST);
	m = *pm = NEW (Param);
	m->m_seq = n++;
	d = e->e_l;
	if (d == NULL) {
	    m->m_pass = O_RES;
	    m->m_sig = void_sig;
	} else {
	    ASSERT (d->e_opr == O_PARDCL);
	    m->m_pass = d->e_r->e_r->e_opr;
	    m->m_sig = d->e_sig;
	    if (d->e_l != NULL && d->e_l->e_l != NULL)
		m->m_name = d->e_l->e_l->e_sym->s_gname;
	}
	pm = &m->m_next;
    }

    return p;
}



/*  protoname (p, s) -- set the name of prototype p from symbol s  */

void
protoname (p, s)
Proptr p;
Symptr s;
{
    char buf[100];
    sprintf (buf, "pb_%s", genref (s->s_name));
    p->p_def = rsalloc (buf);
}



/*  genprotos () -- generate prototypes in the form of C structures  */

void
genprotos ()
{
    Proptr p;
    Parptr m;
    char *e;

    FOREACH (e, prolist) {
	p = (Proptr) e;
	cprintf ("%4typedef struct {\n");
	if (p->p_rstr == O_RESOURCE)
	    cprintf ("%4struct crb_st ih;\n");
	else
	    cprintf ("%4struct invb_st ih;\n");
	for (m = p->p_param; m != NULL; m = m->m_next) {
	    if (m->m_pass == O_REF) {
		/* passed by reference; allocate pointer */
		if (DESCRIBED (m->m_sig))
		    cprintf ("%4%g _%d;\n", m->m_sig, m->m_seq);
		else
		    cprintf ("%4%g *_%d;\n", m->m_sig, m->m_seq);
	    } else {
		/* passed val/var/res; need room for value */
		if (DESCRIBED (m->m_sig))		/* if size varies */
		    cprintf ("%4int o_%d;\n", m->m_seq);   /* alc offset */
		else if (m->m_sig->g_type != T_VOID)	/* else if not void */
		    cprintf ("%4%g _%d;\n", m->m_sig, m->m_seq); /* alc type */
	    }
	}
	cprintf ("%4} %s;\n\n", p->p_def);
    }
}



/*  eproto (e) -- find a prototype from a CALL/SEND/FORWARD/SYM/CAP node  */

Proptr
eproto (e)
Nodeptr e;
{
    Sigptr g;

    if (e->e_opr == O_CALL || e->e_opr == O_SEND || e->e_opr == O_FORWARD)
	e = e->e_l;
    g = e->e_sig;
    while (g->g_type == T_ARRAY)
	g = g->g_usig;
    if (g->g_type == T_OCAP || g->g_type == T_RCAP)
	g = g->g_usig;
    ASSERT (g->g_type == T_OP || g->g_type == T_RES)
    return g->g_proto;
}



/*  dumpprotos () -- print prototype list  */

void
dumpprotos ()
{
    Proptr p;
    Parptr m;
    char *e;

    printf ("\nprototypes:\n");
    FOREACH (e, prolist) {
	p = (Proptr) e;
	printf ("  %s  {%s}\n", p->p_def, oprtos (p->p_rstr));
	for (m = p->p_param; m != NULL; m = m->m_next) {
	    printf ("    %s %s: ",
		oprtos (m->m_pass), m->m_name ? m->m_name : "[anon]");
	    psig (m->m_sig);
	    printf ("\n");
	}
    }
}
