/*  ginput.c -- generate code for "in" statement  */

#include "compiler.h"



static char *namepb	PARAMS ((Nodeptr e, int depth));



/*  ginput (e) -- generate code for "in" statement  */

void
ginput (e)
Nodeptr e;
{
    Nodeptr l, blk, sched, sync, qlist, inparm, inop, id;
    char *pbvar, *invvar, *etyvar, *schvar, *newsch;
    int retlabel, donelabel;
    Bool complex = FALSE;
    Bool haselse = FALSE;
    Proptr pro, oldpro;
    Classptr cp;
    Type schtype;

    /*
     * First, check to see how complicated this input statement is.
     */
    ASSERT (e->e_opr == O_IN);
    if (e->e_r != NULL)		/* if has else clause */
	haselse = TRUE;
    if (e->e_l->e_r != NULL)	/* if multiple arms */
	complex = TRUE;
    for (l = e->e_l; l != NULL; l = l->e_r) {	/* check every arm */
	ASSERT (l->e_opr == O_LIST);
	ASSERT (l->e_l->e_opr == O_ARM);
	blk = l->e_l->e_r;
	ASSERT (l->e_l->e_l->e_opr == O_SCHED);
	sched = l->e_l->e_l->e_r;
	ASSERT (l->e_l->e_l->e_l->e_opr == O_SYNC);
	sync = l->e_l->e_l->e_l->e_r;
	ASSERT (l->e_l->e_l->e_l->e_l->e_opr == O_SELECT);
	qlist = l->e_l->e_l->e_l->e_l->e_l;
	inparm = l->e_l->e_l->e_l->e_l->e_r;
	ASSERT (inparm->e_opr == O_INPARM);
	inop = inparm->e_l;
	ASSERT (inop->e_opr == O_INOP);
	if (sched != NULL)		/* if has scheduling expression */
	    complex = TRUE;
	if (sync != NULL)		/* if has synchronization expression */
	    complex = TRUE;
	if (qlist != NULL)		/* if has quantifier list */
	    complex = TRUE;
	if (inop->e_l != NULL && inop->e_r->e_opr != O_SYM) /* if subscripted */
	    complex = TRUE;
    }

    /*
     * The simplest case is an op that's optimized into a semaphore.
     */
    id = inop->e_l;
    if (id != NULL && id->e_sym->s_op->o_impl == I_SEM) {	/* if sem */
	ASSERT (inparm->e_r->e_r == NULL);
	oldpro = curproto;
	curproto = eproto (inop->e_r);
	++indepth;
	intmp[indepth] = npst ();
	retlabel = inret[indepth] = nextlabel++;
	invptr[indepth] = NULL;
	ASSERT (inop->e_r->e_opr == O_SYM || inop->e_r->e_opr == O_INDEX);
	cprintf ("sr_P(%t,%e);\n", e, inop->e_r);
	gstmt (blk);
	cprintf ("%s%L:;\n", "\n", retlabel);
	indepth--;
	curproto = oldpro;
	return;
    }

    /*
     * Next simplest is a real unsubscripted op with one arm,
     * no sched, no sync, no quant.  An else clause is allowed.
     *
     * Note that this case *must* be optimized if input is from a cap;
     * the general-case RTS entry points disallow a cap that is remote.
     */
    if (!complex && (inop->e_r->e_sig->g_type == T_OCAP || !option_P)) {
	pbvar = namepb (inparm, indepth + 1);
	pro = eproto (inop->e_r);
	cprintf ("{\n");
	cprintf ("%s *%s;\n", pro->p_def, pbvar);
	cprintf ("%s=(%s*)sr_receive(%t,%e,%d);\n",
	    pbvar, pro->p_def, inop, inop->e_r, haselse);
	if (haselse) {
	    cprintf ("if(!%s){", pbvar);
	    gstmt (e->e_r);
	    cprintf ("if (sr_trc_flag)\n");
	    cprintf (
		"   sr_trace(\"NI\",%t,sr_private[MY_JS_ID].cur_proc);\n", e);
	    cprintf ("} else {");
	}
	oldpro = curproto;
	curproto = pro;
	++indepth;
	intmp[indepth] = npst ();
	retlabel = inret[indepth] = nextlabel++;
	invptr[indepth] = pbvar;
	gstmt (blk);
	cprintf ("%s%L:sr_finished_input(%t,(Ptr)%s);\n","\n",retlabel,e,pbvar);
	cprintf ("}\n");
	if (haselse) 
	    cprintf ("}\n");
	indepth--;
	curproto = oldpro;
	return;
    }

    /*
     * Handle the general case.  The generated code is:
     *
     *	    access class
     *	    loop:
     *		get potential invocation
     *		if (null)
     *		    do `else' clause
     *		else
     *		    for (each arm)
     *			for (quantlist)
     *			    if (inv is ours and synch expr is true)
     *				if (scheduled)
     *				    scan queue for better one
     *				process invocation
     *				go to done
     *		go to loop
     *	    done:
     *
     *  First, generate prologue and `else' code.
     */
    donelabel = nextlabel++;
    invvar = alctemp (T_PTR);
    etyvar = alctemp (T_PTR);

    if (inop->e_l)
	cp = inop->e_l->e_sym->s_op->o_class;
    else
	cp = digop (inop->e_r->e_sig) -> o_class;
    cprintf ("sr_iaccess(%C,%d);\n", cp, haselse);
    cprintf ("for (;;) {\n");
    cprintf ("%s=sr_get_anyinv(%t,%C);\n", invvar, e, cp);
    if (haselse) {
	cprintf ("if(!%s){\n", invvar);
	cprintf ("sr_rm_iop(%t,%C,%s);\n", e->e_r, cp, invvar);
	gstmt (e->e_r);
	cprintf ("if (sr_trc_flag)\n");
	cprintf ("   sr_trace(\"NI\",%t,sr_private[MY_JS_ID].cur_proc);\n", e);
	cprintf ("goto %L;\n", donelabel);
	cprintf ("}\n");
    }
    cprintf ("%s=((Invb)%s)->opc.oper_entry;\n", etyvar, invvar);
    /*
     *  Generate code for each arm.
     */
    oldpro = curproto;
    for (l = e->e_l; l != NULL; l = l->e_r) {
	blk = l->e_l->e_r;
	sched = l->e_l->e_l->e_r;
	sync = l->e_l->e_l->e_l->e_r;
	qlist = l->e_l->e_l->e_l->e_l->e_l;
	inparm = l->e_l->e_l->e_l->e_l->e_r;
	inop = inparm->e_l;
	curproto = eproto (inop->e_r);
	pbvar = namepb (inparm, ++indepth);
	invptr[indepth] = pbvar;
	intmp[indepth] = npst ();
	retlabel = inret[indepth] = nextlabel++;
	if (qlist)
	    qbegin (qlist);
	cprintf ("{\n");
	cprintf ("%s *%s=(%s*)%s;\n",
	    curproto->p_def, pbvar, curproto->p_def, invvar);
	cprintf ("if (");
	if (inop->e_l)			/* if named op */
	    cprintf ("%s==(%e).oper_entry", etyvar, inop->e_r);
	else {
	    if (inop->e_r->e_opr == O_FIELD
		&& inop->e_r->e_l->e_sig->g_type == T_RCAP)
		EFATAL (inop->e_r, "illegal context for non-local capability");
	    cprintf ("sr_cap_ck(%t,%s,%e)", inop->e_r, etyvar, inop->e_r);
	    }
	if (sync)
	    cprintf ("\n&& %f", sync);
	cprintf (") {\n");
	if (sched) {
	    schtype = sched->e_sig->g_type;
	    if (schtype != T_REAL)
		schtype = T_INT;
	    schvar = alctemp (schtype);
	    newsch = alctemp (schtype);
	    cprintf ("%s=%e;\n", schvar, sched);
	    freetrans (0, ';');
	    cprintf ("while (%s=(%s*)sr_chk_myinv(((Invb)%s)->opc))\n",
		pbvar, curproto->p_def, invvar);
	    cprintf ("   if ((%s=%f)<%s", newsch, sched, schvar);
	    if (sync)
		cprintf ("\n   && %f", sync);
	    cprintf (")\n");
	    cprintf ("      %s=%s, %s=(Ptr)%s;\n", schvar,newsch,invvar,pbvar);
	    cprintf ("%s=(%s*)%s;\n", pbvar, curproto->p_def, invvar);
	    freetemp (schtype, newsch);
	    freetemp (schtype, schvar);
	}
	cprintf ("sr_rm_iop(%t,%C,%s);\n", blk, cp, invvar);
	gstmt (blk);
	indepth--;

	cprintf ("%s%L:sr_finished_input(%t,(Ptr)%s);\n", "\n", 
		retlabel, e, pbvar);
	cprintf ("goto %L;\n", donelabel);
	cprintf ("}\n");
	if (qlist)
	    qend (qlist);
	cprintf ("}\n");
    }
    curproto = oldpro;
    /*
     *  Generate epilogue.
     */
    cprintf ("}%s%L:;\n", "\n", donelabel);
    freetemp (T_PTR, etyvar);
    freetemp (T_PTR, invvar);
}



/*  greceive (e) -- generate code for "receive" statement  */

void
greceive (e)
Nodeptr e;
{
    Nodeptr l, r, a, p;
    Proptr pro;
    Parptr m;
    Symptr s;
    static char *pbvar = "pb0";		/* pb name for all receives */

    l = e->e_l;
    r = e->e_r;
    ASSERT (l->e_opr == O_INOP)

    if (l->e_l != NULL) {
	/* input is by name, not capability */
	ASSERT (l->e_l->e_opr == O_SYM);
	s = l->e_l->e_sym;
	ASSERT (s->s_kind == K_OP);
	if (s->s_op->o_impl == I_SEM) {		/* if semaphore, it's simple */
	    ASSERT (r == NULL);
	    ASSERT (l->e_r->e_opr == O_SYM || l->e_r->e_opr == O_INDEX);
	    cprintf ("sr_P(%t,%e);\n", e, l->e_r);
	    return;
	}
    }

    /* not a semaphore; handle a general receive */
    pro = eproto (l->e_r);
    cprintf ("{\n");
    cprintf ("%s *%s;\n", pro->p_def, pbvar);
    cprintf ("%s=(%s*)sr_receive(%t,%e,0);\n",
	pbvar, eproto (l->e_r) -> p_def, l, l->e_r);
    m = pro->p_param->m_next;
    while (r != NULL) {
	ASSERT (r->e_opr == O_LIST);
	ASSERT (m != NULL);
	if (m->m_pass == O_REF)
	    p = unode (O_PTRDREF, 
		parmnode (NULLSTR, pbvar, m->m_seq, newsig (T_PTR, m->m_sig)));
	else
	    p = parmnode (NULLSTR, pbvar, m->m_seq, m->m_sig);
	a = bnode (O_ASSIGN, r->e_l, p);
	a->e_locn = r->e_locn;
	attest (a);
	cprintf ("%e;\n", a);
	freetrans (0, ';');
	r = r->e_r;
	m = m->m_next;
    }
    ASSERT (m == NULL);
    cprintf ("sr_finished_input(%t,(Ptr)%s);\n", e, pbvar);
    cprintf ("}\n");
}



/*  namepb (e,d) -- create parameter block name for INPARM node e at depth d  */

static char *
namepb (e, d)
Nodeptr e;
int d;
{
    char *pbvar, buf[20];
    Symptr s;

    ASSERT (e->e_opr == O_INPARM);
    sprintf (buf, "pb%d", d);		/* generate name for param block */
    pbvar = rsalloc (buf);
    if (e->e_r != NULL) {		/* if any parameters */
	ASSERT (e->e_r->e_opr == O_LIST && e->e_r->e_l->e_opr == O_SYM);
	s = e->e_r->e_l->e_sym;
	while (s->s_kind != K_BLOCK)
	    s = s->s_prev;
	s->s_pb = pbvar;		/* set name in BLOCK symbol entry */
    }
    return pbvar;			/* return name */
}
