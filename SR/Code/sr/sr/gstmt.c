/*  gstmt.c -- generate code for statements  */

#include "compiler.h"
#include <ctype.h>

static void gco		PARAMS ((Nodeptr e));
static void cosend	PARAMS ((Nodeptr e, int n, char *tname));
static void ifdo	PARAMS ((Nodeptr e));

static List rscdecl;	/* resources whose one-time decls have been generated */



/*  gstmt (e) -- generate code for statement e  */

void
gstmt (e)
Nodeptr e;
{
    Nodeptr l, r;
    int n;
    Symptr s, o;
    Bool needgen;
    char *tname, *xtern, *p;

    if (!e)
	return;
    l = LNODE (e);
    r = RNODE (e);

    switch (e->e_opr) {

	case O_COMPONENT:
	    rscdecl = list (sizeof (char *));
	    gstmt (r);
	    break;

	case O_IMPORT:
	    if (e->e_sig->g_type == T_GLB)	/* if global, create it */
		cprintf ("sr_create_global(%t,N_%s);\n", e, l->e_sym->s_gname);
	    gstmt (r);
	    break;

	case O_RESOURCE:
	case O_GLOBAL:
	    s = e->e_sig->g_sym;			/* symtab entry */
	    tname = s->s_gname;				/* name */
	    xtern = (s->s_res == curres) ? "" : "extern ";

	    /*
	     * Check to see if we've already generated declarations.
	     */
	    needgen = TRUE;			/* assume no */
	    FOREACH (p, rscdecl)		/* scan list */
		if (* (char **) p == tname) {
		    needgen = FALSE;		/* yes, already on list */
		    break;
		}

	    /*
	     * Generate declaration of resource capability structure.
	     */
	    if (needgen) {
		* (char **) lpush (rscdecl) = tname;	/* add to list */
		if (s->s_res != curres)
		    cprintf ("%1extern int C_%s[];\n", tname);
		cprintf ("%1extern int N_%s;\n", tname);
		cprintf ("%1typedef struct {\n");
		cprintf ("%1Rcap r;\n");
		for (o = s; o != NULLSYM; o = o->s_next)
		    if (o->s_imp == s && o->s_kind == K_OP && o->s_op != NULL
		    && o->s_op->o_impl != I_DCL && isalpha (o->s_name[0]))
			cprintf ("%1%O", o->s_op);
		cprintf ("%1} Rcap_%s;\n", tname);
		if (e->e_opr == O_GLOBAL)
		    cprintf ("%1%sRcap_%s *G_%s;\n", xtern, tname, tname);
		cprintf ("%1\n");
	    }

	    /*
	     * Generate variables.
	     */
	    if (e->e_opr == O_GLOBAL) {
		cdivert (3);
		if (needgen)
		    cprintf ("%3%sstruct {\n", xtern);
		else
		    cprintf ("%3#if 0\n");	/* hide unwanted duplicates */
	    }

	    gstmt (l);
	    gstmt (r);

	    if (e->e_opr == O_GLOBAL) {
		if (needgen) {
		    cprintf ("%3int _;\n");	/* ensure nonempty */
		    cprintf ("%3} gv_%s;\n", tname);
		    cprintf ("%3%sPtr cl_%s;\n\n", xtern, tname);
		} else {
		    cprintf ("%3#endif\n\n");
		}
		undivert (3);
	    }
	    break;

	case O_SPEC:
	    gstmt (l);
	    fixtypes (r);
	    break;

	case O_BODY:
	case O_SEQ:
	case O_LIST:
	    gstmt (l);
	    gstmt (r);
	    break;

	case O_TYPE:
	case O_OPTYPE:
	case O_EXTERNAL:
	    fixtypes (e);
	    break;

	case O_OP:
	    fixtypes (e);
	    s = e->e_l->e_l->e_sym;
	    if (s->s_depth > 2 && !IMPORTED (s)) {
		cprintf ("%8%O", s->s_op);	/* define a local op */
		makeop (s->s_op);
		notepst (s);
	    } else if (s->s_imp == NULL) {	/* private, resource level op */
		if (s->s_op->o_impl == I_IN || s->s_op->o_impl == I_CAP
		|| s->s_op->o_impl == I_UNIMPL)
		    makeop (s->s_op);		/* in-op */
		else if (s->s_op->o_impl == I_SEM)
		    makesemop (s->s_op, e);	/* sem op */
	    }
	    break;

	case O_PROC:
	case O_FINAL:
	case O_SEPARATE:
	    /* nothing to do at this time */
	    break;

	case O_BLOCK:
	    cprintf ("{\n");
	    n = npst ();
	    gstmt (r);
	    freepst (n, TRUE);
	    cprintf ("}\n");
	    break;

	case O_VARDCL:
	case O_CONDCL:
	    genvar (e);
	    break;

	case O_SEM:
	    gstmt (l);
	    s = l->e_l->e_l->e_sym;
	    if (!IMPORTED (s) && r != NULL) {
		ASSERT (s->s_kind == K_OP);
		if (s->s_op->o_impl == I_SEM) {
		    ASSERT (r->e_sig->g_type == T_INT ||
			    r->e_sig->g_type == T_ARRAY);
		    if (r->e_sig->g_type == T_INT) {
		        cprintf ("*(Int*)%n=%e;\n", s, r);
			cprintf ("if (sr_trc_flag) ");
			cprintf ("sr_trace(\"INITS\",%t,%e);\n", e, r);
		    }
		    else { /* (r->e_sig->g_type == T_ARRAY) */
			ASSERT (usig (r->e_sig)->g_type == T_INT);
			cprintf ("sr_init_arraysem(%t,(Ptr)%e,(Ptr)%a,%d);\n",
			    r, l->e_l->e_l, r, ndim (r->e_sig));
			cprintf ("if (sr_trc_flag) ");
			cprintf ("sr_trace(\"INITSARR\",%t);\n", e);
		    }
		} else if (r->e_sig->g_type != T_ARRAY) {
		    ASSERT (r->e_sig->g_type == T_INT);
		    tname = alctemp (T_INT);
		    cprintf ("%s=%e;\n", tname, r);
		    cprintf ("sr_init_semop(%t,(Ptr)&%e,(Ptr)&%s,0);\n",
			r, l->e_l->e_l, tname);
		    freetemp (T_INT, tname);
		} else {
		    ASSERT (usig (r->e_sig)->g_type == T_INT);
		    cprintf ("sr_init_semop(%t,(Ptr)%e,(Ptr)%a,%d);\n",
			r, l->e_l->e_l, r, ndim (r->e_sig));
		}
	    }
	    break;

	case O_PROCESS:
	    gstmt (l);	/* OP and PROC */
	    s = l->e_l->e_l->e_l->e_sym;
	    tname = alctemp (T_PTR);
	    if (r)
		qbegin (r);
	    cprintf ("%s=sr_alc(sizeof(%P),1);\n", tname, s);
	    for (e = r, n = 1; e != NULL; e = e->e_r, n++)
		cprintf ("((%P*)%s)->_%d=_%s;\n",
		    s, tname, n, e->e_l->e_r->e_l->e_l->e_sym->s_gname);
	    cprintf ("((Pach)%s)->size=sizeof(%P);\n", tname, s);
	    cprintf ("((Invb)%s)->type=SEND_IN;\n", tname);
	    cprintf ("((Invb)%s)->opc=rv->_%s;\n", tname, s->s_gname);
	    cprintf ("sr_invoke(%t,%s);\n", l, tname);
	    if (r)
		qend (r);
	    freetemp (T_PTR, tname);
	    break;

	case O_CO:
	    gco (e);
	    break;
	case O_SKIP:
	    /* nothing to generate */
	    break;
	case O_IF:
	case O_DO:
	    ifdo (e);
	    break;
	case O_FA:
	    bgnloop ();
	    qbegin (l);
	    gstmt (r);
	    nextloop ();
	    qend (l);
	    endloop ();
	    break;
	case O_NEXT:
	case O_EXIT:
	    goloop (e);
	    break;
	case O_RETURN:
	    if (indepth > 0) {
		freepst (intmp[indepth], FALSE);
		cprintf ("goto %L;\n", inret[indepth]);
	    } else {
		freepst (0, FALSE);
		cprintf ("if (sr_trc_flag) sr_trace(\"RETURN\",%t,", e);
		if (curproto && curproto->p_rstr != O_FINAL) 
		    cprintf ("(pb->ih).invoker);\n");
		else
		    cprintf ("0);\n");
		cprintf ("goto end;\n");
	    }
	    break;
	case O_STOP:
	    cprintf ("sr_stop(%e,0);\n", l);  
	    break;
	case O_RECEIVE:
	    greceive (e);
	    break;
	case O_IN:
	    ginput (e);
	    break;
	case O_EQ:
	    err (E_WARN + e->e_locn,
		"suspicious comparison -- was `:=' intended?", NULLSTR);
	    cprintf ("%e;\n", e);
	    break;
	default:
	    fixtypes (e);	/* may be type decl inside "new" or "low" */
	    cprintf ("%e;\n", e);
	    break;
    }
    retemp ();			/* recycle any temps used by statement */
    freetrans (0, ';');		/* free any transient mem allocated by stmt */
}



/*  gco (e) -- generate code for a "co" statement  */

static void
gco (e)
Nodeptr e;
{
    int n, qn;
    char *tname;
    Nodeptr l, sel, q, a;
    Proptr p;

    tname = alctemp (T_PTR);
    cprintf ("sr_co_start(%t);\n", e);
    /*
     * dispatch invocations
     */
    for (l = e->e_l, n = 1; l != NULL; l = l->e_r, n++) {
	sel = l->e_l->e_l;
	ASSERT (sel->e_opr == O_COSEL);
	if (sel->e_r->e_opr == O_LIBCALL) {
	    EFATAL (sel->e_r->e_l, "cannot co-call a builtin function");
	    freetemp (T_PTR, tname);
	    return;
	}
	if (sel->e_l != NULL)
	    qbegin (l->e_l->e_l->e_l);
	cosend (sel, n, tname);
	if (sel->e_l != NULL)
	    qend (l->e_l->e_l->e_l);
    }
    /*
     * wait for results
     */
    bgnloop ();
    cprintf ("while(%s=sr_co_wait(%t))switch(((Invb)%s)->co.arm_num){\n",
	tname, e, tname);
    for (l = e->e_l, n = 1; l != NULL; l = l->e_r, n++) {
	sel = l->e_l->e_l;
	a = sel->e_r;
	if (a->e_opr == O_ASSIGN)
	    a = a->e_r;
	p = eproto (a);

	cprintf ("case %d:{\n", n);

	/* get quantifier values */
	for (q = sel->e_l, qn = 0; q != NULL; q = q->e_r, qn++) {
	    cprintf ("int %e=*(int *)(%s+SRALIGN(sizeof(%s))+%d);\n",
		q->e_l->e_r->e_l->e_l, tname,
		p->p_def, qn * SRALIGN (sizeof (int)));
	}

	/* copy back var and res params (if call, not send) */
	if (a->e_opr == O_CALL) {
	    cprintf ("0");
	    gparback (eproto (a), a->e_r, tname);
	    cprintf (";\n");
	}

	/* store result of assignment */
	if (sel->e_r->e_opr == O_ASSIGN) {
	    q = bnode (O_ASSIGN, sel->e_r->e_l,
		parmnode (p->p_def, tname, 0, sel->e_r->e_r->e_sig));
	    q->e_sig = q->e_r->e_sig;
	    q->e_locn = q->e_r->e_locn;
	    gstmt (q);
	}

	/* finish explicit & implicit postprocessing */
	ASSERT (l->e_l->e_opr == O_COINV);
	gstmt (l->e_l->e_r);	/* postprocessing block */
	cprintf ("sr_free(%s);\n", tname);
	cprintf ("break;}\n");
    }
    nextloop ();
    cprintf ("}");
    endloop ();
    cprintf ("sr_co_end(%t);\n", e);
    freetemp (T_PTR, tname);
}



/*  cosend (e, n, t) -- gen invoks for COSEL node e of arm n using temp t  */

static void
cosend (e, n, tname)
Nodeptr e;
char *tname;
{
    Nodeptr a;

    ASSERT (e->e_opr == O_COSEL);
    a = e->e_r;
    if (a->e_opr == O_ASSIGN)
	a = a->e_r;
    ASSERT (a->e_opr == O_CALL || a->e_opr == O_SEND);

    cprintf ("%s=", tname);
    gparblk (NULLSTR, eproto (a), a->e_r, e->e_l, a->e_opr);
    cprintf (";\n");
    if (a->e_opr == O_CALL)
	cprintf ("((Invb)%s)->type=COCALL_IN;\n", tname);
    else /* O_SEND */
	cprintf ("((Invb)%s)->type=COSEND_IN;\n", tname);
    cprintf ("((Invb)%s)->co.arm_num=%d;\n", tname, n);
    cprintf ("((Invb)%s)->opc=%e;\n", tname, a->e_l);
    cprintf ("%s=sr_invoke(%t,%s);\n", tname, e, tname);
}



/*  ifdo (e) -- generate code for an "if" or "do" statement  */

static void
ifdo (e)
Nodeptr e;
{
    Operator op;
    Nodeptr l;
    char *eword = "";

    op = e->e_opr;
    ASSERT (op == O_IF || op == O_DO);

    /* for DO only, wrap in an infinite loop */
    if (op == O_DO) {
	bgnloop ();
	cprintf ("for (;;) {\n");
	nextloop ();
	if (!option_T) {
	    cprintf ("if (--sr_private[MY_JS_ID].rem_loops<=0)\n");
	    cprintf ("   sr_loop_resched(%t);\n", e);
	}
    }

    /* these tests are supposed to be nondeterministic, but that's not easy. */
    /* no sense in shuffling here, need RUNTIME nondeterminism. */
    for (l = e->e_l; l != NULL; l = l->e_r) {
	ASSERT (l->e_l->e_opr == O_GUARD);
	cprintf ("%sif (%f)", eword, l->e_l->e_l);
	gstmt (l->e_l->e_r);
	eword = "else ";
    }

    if (op == O_DO) {

	/* a DO always has an else; implicit "else exit" if not explicit */
	cprintf ("else ");
	if (e->e_r != NULL)
	    gstmt (e->e_r);
	else
	    cprintf ("break;\n");
	cprintf ("}\n");			/* terminate DO loop */
	endloop ();

    } else /* op == O_IF */ {

	/* an IF has an else clause only if explicitly given */
	if (e->e_r != NULL) {
	    cprintf ("else ");
	    gstmt (e->e_r);
	}
    }
}
