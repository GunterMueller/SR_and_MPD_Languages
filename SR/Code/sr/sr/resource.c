/*  resource.c -- compile one resource or global  */

#include "compiler.h"
#include "../paths.h"

static void body	PARAMS ((Nodeptr root));
static void genresource	PARAMS ((Nodeptr e));
static void genprocs	PARAMS ((Nodeptr e));
static void genext	PARAMS ((Nodeptr e));
static void genfinal	PARAMS ((Nodeptr e));
static void bugtree	PARAMS ((int flag, Nodeptr e, char *label));

static int hasfinal;		/* was explicit final generated? */



/*  resource (root) -- compile a resource, given the parse tree  */

void
resource (root)
Nodeptr root;
{
    resname = salloc (root->e_l->e_name);/* save name permanently */
    curres = NULL;			/* curres will be set later */

    if (option_d)			/* print res name if any debugging */
	printf ("\n===============  %s  ===============\n", resname);
    bugtree ('t', root, "parse tree");	/* print parse tree if requested */

    body (root);			/* compile body */

    if (option_d)
	fflush (stdout);
}



/*  body (root) -- compile the body of a resource  */

static void
body (root)
Nodeptr root;
{
    Nodeptr e, i;
    char cfile[MAX_PATH], cpath[MAX_PATH];
    
    if (rlp >= rlist + MAX_RES_DEF)
	mexit ("too many resources");
    *rlp++ = resname;			/* save name to pass to srl */

    e = root->e_r;			/* get RESOURCE node */
    initimp (resname);			/* init imports to prevent rereading */
    if (e->e_l == NULL) {		/* if we have no spec */
	i = readspec (root->e_l, 0);	/* then import it */
	if (i) {
	    e->e_l = i->e_l;		/* add spec to tree */
	    e->e_opr = i->e_opr;	/* change RESOURCE to GLOBAL if gbl */
	}
    }
    if (nfatals > 0)
	return;

    import (root);			/* expand imports into trees */
    bugtree ('T', root, "parse tree with imports");	

    identify (root);			/* resolve identifiers */

    bugtree ('i', root, "parse tree with symbols resolved");
    if (dbflags['$'])
	dumpsyms (0);			/* sr -d$: print whole symbol table */
    else if (dbflags['S'])
	dumpsyms (1);			/* sr -dS: print without predefs */

    if (nfatals > 0)			/* quit here if there were errors */
	return;

    attest (root);			/* affix signatures */
    bugtree ('g', root, "parse tree with signatures");
    if (nfatals > 0)
	return;

    if (! option_F)
	fold (root);			/* fold constants */
    bugtree ('f', root, "parse tree after constant folding");

    if (dbflags['s'])
	dumpsyms (1);			/* sr -ds: print symtab with signats */
    if (dbflags['p'])
	dumpprotos ();			/* sr -dp: print op prototype table */

    opscan ();				/* check ops, find implicit sems */

    if (dbflags['o'])
	dumpops ();			/* sr -do: dump op list */
    if (dbflags['c'])
	dumpclass ();			/* sr -dc: dump class list */

    if (nfatals > 0)			/* quit here if there were errors */
	return;

    sprintf (cfile, "%s.c", resname);
    sprintf (cpath, "%s/%s", ifdir, cfile);
    mkinter ();
    copen (cpath);

    nextlabel = 1;
    genresource (root);			/* generate top-level resource code */
    hasfinal = 0;
    genprocs (root);			/* generate code inside procs */
    if (!hasfinal)
	genfinal (NULLNODE);		/* generate implicit final code */

    tracedump ();
    cflush ();
    errflush ();
    cclose ();

    if (nfatals == 0) {
	if (root->e_r->e_opr == O_GLOBAL)
	    wimpl (resname);		/* write .impl file for global */
	cc (ifdir, cfile);		/* compile the generated C code */
    }

    if (!(option_C || option_g))  {
	if (unlink (cpath) != 0)	/* remove C code after compilation */
	    perror (cpath);
    }
}



/*  genresource (e) -- generate top-level resource code. */

static void
genresource (e)
Nodeptr e;
{
    initdynamic ();

    /* (%0) begin file */
    cprintf ("%0/* resource %s */\n\n/* %s */\n\n", resname, VERSION);
    cprintf ("%0#define RUNTIME\n");
    cprintf ("%0#include \"%s/srmulti.h\"\n", option_e ? SRSRC : SRLIB);
    cprintf ("%0#include \"%s/sr.h\"\n\n", option_e ? SRSRC : SRLIB);

    /* (%5) begin resource variable structure */
    cprintf ("%5typedef struct {\n");	/* header of rvar struct */
    cprintf ("%5Rcap myresource;\n");

    /* (%8) begin function header for executable code */
    cprintf ("%8\nvoid R_%s(rp)\n", resname);
    cprintf ("%8register %s *rp;\n", curres->r_sig->g_proto->p_def);
    cprintf ("%8%s\n", "{");		/*  don't count this '{' */
    cprintf ("%8register rvar *rv;\n");

    /* (%9) begin executable code */
    cprintf ("(sr_private[MY_JS_ID].cur_proc)->pname = \"%s.body\";\n",
	    curres->r_sym->s_name); 
    cprintf ("rv=(rvar*)sr_alloc_rv(sizeof(*rv));\n");
    if (curres->r_sig->g_type == T_GLB) {
	cprintf ("G_%s=(Rcap_%s*)rv;\n", resname, resname);
	cprintf ("cl_%s=sr_make_class();\n", resname);
    }
    cprintf ("if (sr_trc_flag) sr_trace(\"BODY\",%t,0);\n", e);

    genclass ();			/* initialize classes */
    genprotos ();			/* declare parameter blocks */
    genops ();				/* declare ops */

    gstmt (e);				/* generate resource code */
    freepst (0, TRUE);			/* free presistent memory */

    cprintf ("end:\n");
    cprintf ("if (sr_trc_flag) sr_trace(\"ENDBODY\",%t,0);\n", e);
    cprintf ("sr_finished_init();\n");	
    cprintf ("%s\n\n\n", "}");		/*  don't count this '}' */

    dcltemps ();			/* generate accumulated temps */
    cprintf ("%6} rvar;\n\n");		/* terminate rvar struct decl */
    cflush ();
}



/*  genprocs (e) -- generate code for procs contained in tree e  */

static void
genprocs (e)
Nodeptr e;
{
    Opptr o;
    char *name;

    if (e == NULL)
	return;
    switch (e->e_opr) {
	case O_COMPONENT:
	case O_GLOBAL:
	case O_RESOURCE:
	case O_SPEC:
	case O_BODY:
	case O_LIST:
	case O_SEQ:
	case O_SEM:
	case O_PROCESS:
	    genprocs (LNODE (e));
	    genprocs (RNODE (e));
	    break;
	case O_PROC:
	    initdynamic ();
	    o = e->e_r->e_l->e_sym->s_op;
	    curproto = o->o_usig->g_proto;
	    name = o->o_sym->s_gname;
	    cprintf ("%8\n");
	    if (!o->o_exported || curres->r_opr != O_GLOBAL)
		cprintf ("%8static ");
	    cprintf ("%8void P_%s (rp,rv,pb,wc)\n", name);
	    cprintf ("%8register %s *rp;\n", curres->r_sig->g_proto->p_def);
	    cprintf ("%8register rvar *rv;\n");
	    cprintf ("%8register %s *pb;\n", curproto->p_def);
	    cprintf ("%8int wc;\n");
	    cprintf ("%8%s\n", "{");
	    cprintf ("%8char *old_name;\n");
	    if (curres->r_opr == O_GLOBAL)
		cprintf ("rv=(rvar*)G_%s;\n", resname);

	    /* put some prologue code in a nested block
	     * to ensure the vars won't be used when no longer valid */
	    cprintf ("{\n");
	    cprintf ("struct private_st *pst = &sr_private[MY_JS_ID];\n");
	    cprintf ("Proc curp = pst->cur_proc;\n");
	    cprintf ("old_name = curp->pname;\n");
	    cprintf ("if (sr_trc_flag && wc != RTS_CALL)\n");
	    cprintf ("   pb->ih.invoker = pb->ih.forwarder = curp;\n");
	    cprintf ("curp->pname = \"%s.%s\";\n", curres->r_sym->s_name, name);
	    cprintf ("curp->res = ");
	    cprintf ("pst->cur_res;\n"); 
	    cprintf (
		"if (sr_trc_flag) sr_trace(\"PROC\",%t,(pb->ih).forwarder);\n",
		e);
	    cprintf ("}\n");

	    /* generate code for proc */
	    gstmt (e->e_r);

	    /* generate epilogue */
	    cprintf ("end:\n");
	    cprintf (
		"if (sr_trc_flag) sr_trace(\"ENDPROC\",%t,(pb->ih).invoker);\n",
		e);  
	    cprintf ("(sr_private[MY_JS_ID].cur_proc)->pname = old_name;\n");
	    cprintf ("if (wc==RTS_CALL) sr_finished_proc(pb);\n");
	    cprintf ("%s\n\n\n", "}");
	    dcltemps ();
	    cflush ();
	    curproto = NULL;
	    break;
	case O_EXTERNAL:
	    genext (e);
	    break;
	case O_FINAL:
	    if (hasfinal)
		EFATAL (e, "duplicate `final' code");
	    hasfinal = 1;
	    genfinal (e->e_l);
	    break;
    }
}



/*  genext (e) -- generate interface to external code  */
static void
genext (e)
Nodeptr e;
{
    char *name, *p, *dcl;
    Sigptr g;
    Type t;
    Parptr m;
    Opptr o;
    int i;
    char buf[200];

    initdynamic ();
    o = e->e_l->e_l->e_sym->s_op;
    name = o->o_sym->s_name;
    curproto = o->o_usig->g_proto;
    g = curproto->p_param->m_sig;
    t = g->g_type;
    cprintf ("%8\n");
    if (!o->o_exported || curres->r_opr != O_GLOBAL)
	cprintf ("%8static ");
    cprintf ("%8void P_%s (rp,rv,pb,wc)\n", name);
    cprintf ("%8register %s *rp;\n", curres->r_sig->g_proto->p_def);
    cprintf ("%8register rvar *rv;\n");
    cprintf ("%8register %s *pb;\n", curproto->p_def);
    cprintf ("%8int wc;\n");
    cprintf ("%8%s\n", "{");

    /* figure out the C type of the function, and declare it */
    switch (t) {
	case T_VOID:
	    dcl = NULL;		/* best to say nothing if user didn't specify */
				/* othwise risk conflict e.g. w/ stdio.h */
	    break;
	case T_BOOL:
	case T_CHAR:
	case T_INT:
	case T_ENUM:
	    dcl = "int ";
	    break;
	case T_REAL:
	    dcl = "double ";
	    break;
	case T_PTR:
	    dcl = "char *";
	    break;
	case T_STRING:
	    cprintf ("char *result;\n");
	    dcl = "char *";
	    break;
	case T_FILE:
	    dcl = "FILE *";
	    break;
	default:
	    EFATAL (e, "illegal return type for external");
	    return;
    }
    if (dcl != NULL) {
	cprintf ("#ifndef %s\n", name);
	cprintf ("%s%s();\n", dcl, name);
	cprintf ("#endif\n");
    }

    cprintf ("if (sr_trc_flag && wc != RTS_CALL) {\n");
    cprintf ("  (pb->ih).invoker = sr_private[MY_JS_ID].cur_proc;\n");
    cprintf ("  (pb->ih).forwarder = sr_private[MY_JS_ID].cur_proc;\n");
    cprintf ("}\n");

    cprintf ("if (sr_trc_flag) sr_trace(\"PROC\",%t,(pb->ih).forwarder);\n", e);

    /* terminate any string arguments with \0 */
    for (m = curproto->p_param->m_next; m != NULL; m = m->m_next)
	if (m->m_sig->g_type == T_STRING
	&& (m->m_pass == O_VAL || m->m_pass == O_VAR)) {
	    p = alctemp (T_STRING);
	    cprintf ("%s=(String*)((Ptr)pb+pb->o_%d); ", p, m->m_seq);
	    cprintf ("*(DATA(%s)+%s->length)='\\0';\n", p, p);
	    freetemp (T_STRING, p);
	}

    /* call the function */
    if (t == T_STRING)
	cprintf ("result=");
    else if (t != T_VOID)
	cprintf ("pb->_0=");
    cprintf ("%s(", name);
    i = 0;
    for (m = curproto->p_param->m_next; m != NULL; m = m->m_next) {
	i++;
	if (m->m_pass == O_REF) 
	    /*
	     * passed by reference
	     */
	    switch (m->m_sig->g_type) {
		case T_INT:
		case T_ENUM:
		case T_REAL:
		case T_FILE:
		    cprintf (",pb->_%d", m->m_seq);
		    break;
		case T_BOOL:
		case T_CHAR:
		case T_REC:
		case T_PTR:
		    cprintf (",(char*)pb->_%d", m->m_seq);
		    break;
		case T_STRING:
		    cprintf (",(char*)DATA(pb->_%d)", m->m_seq);
		    break;
		case T_ARRAY:
		    cprintf (",(char*)ADATA(pb->_%d)", m->m_seq);
		    break;
		default:
		    EFATAL (e, "illegal parameter type for external");
		    return;
	    }
	else
	    /*
	     * passed (in and/or out) by value:  val, var, res
	     */
	    switch (m->m_sig->g_type) {
		case T_BOOL:
		case T_CHAR:
		case T_INT:
		case T_ENUM:
		case T_REAL:
		case T_PTR:
		case T_FILE:
		    cprintf (",pb->_%d", m->m_seq);
		    if (m->m_pass != O_VAL) {
			sprintf (buf, 
		        "%s(), parameter %d: type precludes returning a result",
			    name, i);
			err (E_WARN + e->e_locn, buf, NULLSTR);
		    }
		    break;
		case T_REC:
		    cprintf (",(char*)&pb->_%d", m->m_seq);  /* pass by ref */
		    break;
		case T_STRING:
		    cprintf (",(char*)DATA((%s)((Ptr)pb+pb->o_%d))",
			csig (m->m_sig), m->m_seq);
		    break;
		case T_ARRAY:
		    cprintf (",(char*)ADATA((%s)((Ptr)pb+pb->o_%d))",
			csig (m->m_sig), m->m_seq);
		    break;
		default:
		    EFATAL (e, "illegal parameter type for external");
		    return;
	    }
    }
    cprintf (");\n");

    /* if the C function returned a string pointer, copy the string to arg 0 */
    if (t == T_STRING) {
	p = alctemp (T_STRING);
	cprintf ("%s=(String*)((Ptr)pb+pb->o_0);\n", p);
	cprintf ("if (result) {\n");
	cprintf ("*(DATA(%s)+%s->size-%d)='\\0';\n", p, p, STRING_OVH);
	cprintf ("strncpy(DATA(%s),result,%s->size-%d);\n", p, p, STRING_OVH);
	cprintf ("} else\n");
	cprintf ("   *DATA(%s)='\\0';\n", p);
	freetemp (T_STRING, p);
    }

    /* set string length for any results (parameters or function value) */
    for (m = curproto->p_param; m != NULL; m = m->m_next)
	if (m->m_sig->g_type == T_STRING
	&& (m->m_pass == O_VAR || m->m_pass == O_RES)) {
	    p = alctemp (T_STRING);
	    cprintf ("%s=(String*)((Ptr)pb+pb->o_%d); ", p, m->m_seq);
	    cprintf ("%s->length=strlen(DATA(%s));\n", p, p);
	    freetemp (T_STRING, p);
	}

    /* finish up */
    cprintf ("end:\n");
    cprintf ("if (sr_trc_flag) sr_trace(\"ENDPROC\",%t,(pb->ih).invoker);\n",e);
    cprintf ("if (wc==RTS_CALL) sr_finished_proc(pb);\n");
    cprintf ("%s\n\n\n", "}");
    dcltemps ();
    cflush ();
    curproto = NULL;
}



/*  genfinal (e) -- generate final code for block e  */

static void
genfinal (e)
Nodeptr e;
{
    curproto = NEW (Proto);	/* parameterless proto indicates final */
    curproto->p_rstr = O_FINAL;
    initdynamic ();
    cprintf ("%8\n\nvoid F_%s(rp,rv)\n", resname);
    cprintf ("%8register %s *rp;\n", curres->r_sig->g_proto->p_def);
    cprintf ("%8register rvar *rv;\n");
    cprintf ("%8%s\n", "{");
    cprintf ("%8\n");
    cprintf ("(sr_private[MY_JS_ID].cur_proc)->pname = \"%s.final\";\n",
	    curres->r_sym->s_name);
    if (e && e->e_locn)
	cprintf ("if (sr_trc_flag) sr_trace(\"FINAL\",%t,0);\n", e);
    gstmt (e);				/* user code (if any) */
    cprintf ("end: \n");
    if (e && e->e_locn)
	cprintf ("if (sr_trc_flag) sr_trace(\"ENDFINAL\",%t,0);\n", e);
    cprintf ("sr_finished_final();\n");
    cprintf ("%s\n", "}");
    dcltemps ();
    cflush ();
    curproto = NULL;
}



/*  bugtree (c, root, label) -- print parse tree if debugging flag c is set  */

static void
bugtree (c, root, label)
int c;
Nodeptr root;
char *label;
{
    if (dbflags[c]) {
	errflush ();
	printf ("\n%s:\n", label);
	ptree (root);
    }
}
