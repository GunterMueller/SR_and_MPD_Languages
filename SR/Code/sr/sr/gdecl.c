/*  gdecl.c -- generate code for declarations  */

#include "compiler.h"

static void initarray	PARAMS ((Nodeptr e));



/*  genvar (e) -- generate variable declaration  */

void
genvar (e)
Nodeptr e;
{
    Nodeptr a;
    Symptr s;
    Sigptr g;
    Varptr v;
    Variety vty;
    Operator o;

    ASSERT (e != NULL && (e->e_opr == O_VARDCL || e->e_opr == O_CONDCL));
    ASSERT (e->e_l->e_opr == O_SUBS && e->e_r->e_opr == O_VARATT);

    fixtypes (e->e_l->e_r);	
    fixtypes (e->e_r);

    g = e->e_sig;			/* signature */
    s = e->e_l->e_l->e_sym;		/* symtab entry */
    ASSERT (s->s_kind == K_VAR);
    v = s->s_var;
    vty = v->v_vty;

    switch (vty) {
	case V_GLOBAL:
	    cprintf ("%3%g %N;\n", g, s);
	    break;
	case V_RVAR:
	    cprintf ("%6%g %N;\n", g, s);
	    break;
	case V_LOCAL:
	    cprintf ("%8%g %N;\n", g, s);
	    break;
	default:
	    BOOM ("unexpected variety in genvar", varitos (vty));
    }

    /* initialize, if not an imported global */
    if (vty != V_GLOBAL || !IMPORTED (s)) {

	/* some types need to be allocated and/or initialized */
	switch (g->g_type) {
	    case T_ARRAY:
		if (e->e_r->e_l == NULL
		&& e->e_r->e_r->e_sig->g_type == T_ARRAY
		&& g->g_lb == NULL) {
		    initarray (e);
		    freetrans (0, ';');
		    return;
		}
		notepst (s);
		cprintf ("%n=sr_init_array(%t,(Array*)0,", s, e);
		garray (e, e->e_sig);
		cprintf (");\n");
		break;
	    case T_STRING:
		notepst (s);
		if (g->g_ub != NULL)		/* if maxlength specified */
		    cprintf ("%n=sr_alc_string(%e,1);\n", s, g->g_ub);
		else				/* else get from initializer */
		    cprintf ("%n=sr_alc_string((%E)->length,1);\n",
			s, e->e_r->e_r);
		break;
	    case T_REC:
		if (g->g_rec->k_init)
		    cprintf ("%n=%s;\n", s, g->g_rec->k_init);
		break;
	    default:
		break;
	}

	/* assign initial value, if there is one */
	if (e->e_r->e_r != NULL) {
	    a = bnode (O_ASSIGN, e->e_l->e_l, e->e_r->e_r);
	    attest (a);			/* affix signatures */
	    if (a->e_opr == O_ASSIGN) {	/* if no errors */
		if (a->e_l->e_sig == NULL)
		    a->e_l->e_sig = a->e_r->e_sig;
		o = v->v_dcl;		/* save operator */
		v->v_dcl = O_VARDCL;	/* make constants variable */
		cprintf ("%e;\n", a);	/* generate assignment */
		v->v_dcl = o;		/* restore operator */
	    }
	}

    }
    freetrans (0, ';');
}



/*  initarray (e) -- generate code to allocate initialized array  */

/*  this is a special case for when we don't know {ub,lb} at compile time  */

static void
initarray (e)
Nodeptr e;
{
    Nodeptr r;
    Symptr s;
    char *tname;

    ASSERT (e != NULL && e->e_l != NULL && e->e_l->e_opr == O_SUBS);
    r = e->e_r->e_r;			/* initial value */
    s = e->e_l->e_l->e_sym;		/* symtab entry */
    notepst (s);

    tname = alctemp (T_ARRAY);
    cprintf ("%s=%a;\n", tname, r);
    cprintf ("%n=(Array*)memcpy(sr_alc(-%s->size,1),(Ptr)%s,%s->size);\n",
	s, tname, tname, tname);
    freetemp (T_ARRAY, tname);
}
