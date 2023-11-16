/*  op.c -- operation handling  */

#include "compiler.h"


static List classlist;		/* list of classes */
static List oplist;		/* list of ops */
static int serial;		/* serial number counter */

static Classptr cmerge PARAMS ((Classptr o, Classptr p));
static void capmerge PARAMS ((NOARGS));



/*  initops () -- initialize op and class lists  */

void
initops ()
{
    oplist = list (sizeof (Op));
    classlist = list (sizeof (Class));
    serial = 0;
}



/*  newop(s,seg) -- make new Op struct for symbol s
 *
 *  newop allocates and initializes an Op struct and associated Class struct.
 *
 *  newop allocates a minimal Signat struct for o_asig and o_usig
 *  to allow recursive refs within the parameter list.
 *
 */

Opptr
newop (s, seg)
Symptr s;
Segment seg;
{
    Opptr o;
    Classptr c;

    o = (Opptr) lput (oplist);
    c = (Classptr) lput (classlist);

    o->o_sym = s;
    o->o_class = c;
    o->o_seg = seg;
    o->o_asig = newsig (T_OP, NULLSIG);
    o->o_usig = newsig (T_OP, NULLSIG);
    o->o_impl = I_UNIMPL;

    if (seg == SG_GLOBAL)
	c->c_num = 0;
    else
	c->c_num = ++serial;
    c->c_members = 1;
    c->c_first = o;
    c->c_seg = seg;
    c->c_geninit = FALSE;

    s->s_op = o;
    s->s_sig = o->o_asig;
    return o;
}



/*  digop (g) -- find op entry based on signature  */

Opptr
digop (g)
Sigptr g;
{
    Symptr s;

    while (g->g_type == T_ARRAY)
	g = g->g_usig;
    if (g->g_type == T_OCAP)
	g = g->g_usig;
    ASSERT (g->g_type == T_OP);
    s = g->g_sym->s_prev;
    ASSERT (s->s_kind == K_OP);
    return s->s_op;
}



/*  instmt (e) -- combine classes for *in* statement  */

void
instmt (e)
Nodeptr e;
{
    Nodeptr l, el, inop, id;
    Opptr o;
    Classptr c;
    Sigptr g;

    c = NULL;					/* init empty class list */
    el = e->e_r;				/* `else' block */
    l = e->e_l;					/* head of arm list */
    ASSERT (l->e_opr == O_LIST);

    while (l != NULL) {				/* for each arm */
	ASSERT (l->e_l->e_opr == O_ARM);
	ASSERT (l->e_l->e_l->e_opr == O_SCHED);
	ASSERT (l->e_l->e_l->e_l->e_opr == O_SYNC);
	ASSERT (l->e_l->e_l->e_l->e_l->e_opr == O_SELECT);
	ASSERT (l->e_l->e_l->e_l->e_l->e_r->e_opr == O_INPARM);
	inop = l->e_l->e_l->e_l->e_l->e_r->e_l;
	ASSERT (inop->e_opr == O_INOP);

	if (inop->e_l != NULL) {

	    /* input by op name */
	    id = inop->e_l;
	    if (id == NULL || id->e_opr != O_SYM || id->e_sym->s_kind != K_OP)
		return;				/* if earlier errors */
	    o = id->e_sym->s_op;		/* find op */
	    c = cmerge (c, o->o_class);		/* merge classes */

	    /* disallow sem impl if else, sched, sync, quant, or multi arms */
	    if (el || l->e_r || l->e_l->e_l->e_r || l->e_l->e_l->e_l->e_r
		    || l->e_l->e_l->e_l->e_l->e_l)
		o->o_nosema = TRUE;

	} else {

	    g = inop->e_r->e_sig;
	    if (g->g_type == T_OCAP) {		/* if no earlier errors */
		o = digop (g);			/* find op entry */
		o->o_incap = TRUE;		/* mark for later handling */
		c = cmerge (c, o->o_class);	/* merge */
	    }
	}

	l = l->e_r;				/* move to next arm */
    }
}


/*  capmerge () -- merge classes based on cap usage.
 *
 *  Merge any op prototype whose cap was used in an input statement
 *  with any other compatible op whose cap was constructed -- because
 *  the cap might find its way eventually to the input statement.
 */

static void
capmerge ()
{
    Classptr c;
    Opptr x, y;
    char *p, *q;

    FOREACH (p, oplist) {
	x = (Opptr) p;
	if (x->o_incap) {
	    c = x->o_class;
	    FOREACH (q, oplist) {
		y = (Opptr) q;
		if ((y->o_exported || y->o_madecap)
			&& compat (x->o_usig, y->o_usig, C_EXACT))
		    c = cmerge (c, y->o_class);
	    }
	}
    }
}
 


/*  cmerge (c, d) -- merge the classes associated with two operations.
 *
 *  Either class may be NULL.  The merged class is returned.
 */

static Classptr
cmerge (c, d)
Classptr c, d;
{
    Opptr o;

    if (!c)
	return d;
    if (!d)
	return c;
    if (c == d)
	return c;			/* already merged earlier */

    c->c_members += d->c_members;	/* combine membership count */
    if (c->c_seg == SG_PROC)
	c->c_seg = d->c_seg;		/* possibly promote to resource level */

    for (o = c->c_first; o->o_classmate != NULL; o = o->o_classmate)
	;				/* find end of class c */
    o->o_classmate = d->c_first;	/* append members of d */

    while ((o = o->o_classmate) != NULL)
	o->o_class = c;			/* move members of d to class c */
    ldel (classlist, (char *) d);	/* destroy class d */
    return c;
}



/*  opscan () -- global op checking after entire resource has been parsed
 *
 *  Merge classes due to use of capabilities in input stmts. 
 *  Check that all ops have been implemented (except in globals).
 *  Remove classes corresponding to ops implemented by procs.
 *  Turn ops into semaphores when possible for faster implementation.
 *
 *  An op is turned into a semaphore if it meets several tests.
 *  Checks made elsewhere (setting o_nosema if they fail) are:
 *	-- must be declared at resource level, not inside a proc or block
 *	-- must have no parameters and no result
 *	-- must not be exported (i.e. not declared in the spec)
 *	-- must have no else, sync, or sched on any "in" statement
 *	-- must be invoked only by send, not call
 *	-- must neither forward within its body nor be target of a forward
 *	-- must not be sent to within a co stmt (call w/i co handled above)
 *	-- must not be otherwise referenced (e.g. as a parameter or capability)
 *  Final checks made below are:
 *	-- must be implemented by "in" (this includes "P" and "receive")
 *	-- must be the only op in its class
 *	-- the -P (pessimize) flag did not appear on the command line
 *
 *  Even ops explicitly declared as semaphores must meet those tests.  A "sem"
 *  declared in a spec portion, e.g., cannot truly be implemented as one.
 */

void
opscan ()
{
    Classptr c;
    Opptr o;
    char *p, *name;

    capmerge ();			/* merge classes based on cap usage */
    FOREACH (p, classlist) {
	c = (Classptr) p;
	for (o = c->c_first; o != NULL; o = o->o_classmate) {
	    name = o->o_sym->s_name;
	    if (o->o_seg == SG_RESOURCE || o->o_seg == SG_PROC) {
		switch (o->o_impl) {
		    case I_UNIMPL:
			if (!o->o_exported) {
			    if (o->o_dclsema)
				err (srclocn,
				    "semaphore %s never implemented by P()",
				    name);
			    else
				err (srclocn,
				    "op %s declared but not implemented", name);
			}
			o->o_impl = I_IN;
			break;
		    case I_PROC:
		    case I_EXTERNAL:
			ldel (classlist, (char *) o->o_class);
			o->o_class = NULLCLASS;
			break;
		    case I_IN:
			if (o->o_class->c_members == 1
			&& !o->o_nosema
			&& !option_P)
			    o->o_impl = I_SEM;
			break;
		    case I_CAP:
		    case I_DCL:
			/* nothing */
			break;
		}
	    }
	}
    }
}



/*  genclass () -- generate code to initialize classes  */

void
genclass ()
{
    Classptr c;
    char *p;

    FOREACH (p, classlist) {
	c = (Classptr) p;
	if (c->c_num > 0 && c->c_seg != SG_PROC && c->c_first->o_impl != I_SEM){
	    /* if not global, not local, not a semaphore */
	    cprintf ("%6Ptr c%d;\n", c->c_num);
	    cprintf ("%C=sr_make_class();\n", c);
	    c->c_geninit = TRUE;
	}
    }
}



/*  genops () -- generate code to declare ops  */

void
genops ()
{
    Opptr o;
    Symptr s;
    char *p;

    cprintf ("%1int C_%s[] = {", curres->r_sym->s_gname);
    FOREACH (p, oplist) {
	o = (Opptr) p;
	s = o->o_sym;
	ASSERT (s->s_kind == K_OP);

	if (!IMPORTED (s)) {

	    /* declare the opcap, if a real op (not cap) at resource level */
	    if (o->o_impl != I_DCL && s->s_depth == 2)
		cprintf ("%5%O", o);

	    if (o->o_impl==I_IN || o->o_impl==I_UNIMPL || o->o_impl==I_CAP) {

		/* create op, if exported */
		if (o->o_exported) {
		    if (o->o_seg != SG_GLOBAL)
			cprintf ("%1%d%,", ndim (o->o_asig));	/* dimensions */
		    makeop (o);
		}

	    } else if (o->o_impl == I_PROC || o->o_impl == I_EXTERNAL) {

		/* declare the C function that implements a proc */
		if (!o->o_exported || curres->r_opr != O_GLOBAL)
		    cprintf ("%0static ");
		cprintf ("%0void P_%s();\n", s->s_gname);

		/* add to list of dimensions (always 0 for a proc) */
		if (o->o_exported)
		    cprintf ("%10%,");

		/* initialize the opcap */
		cprintf ("sr_make_proc(&rv->_%s,%d,P_%s);\n", s->s_gname,
		    o->o_sepctx ? PROC_SEP_OP : PROC_OP, s->s_gname);
	    }
	}
    }

    /* terminate declarations */
    cprintf ("%0\n");
    cprintf ("%1-1};\n\n");
}



/*  makeop (o) -- generate code to create an input op  */

void
makeop (o)
Opptr o;
{
    Symptr s;
    Sigptr g;
    Classptr c;

    c = o->o_class;
    /* declare classptr for locals, but only once (at head of class chain) */
    if (c->c_seg == SG_PROC && !c->c_geninit) {
	cprintf ("%8Ptr c%d;\n", c->c_num);
	cprintf ("%C=sr_make_class();\n", c);
	c->c_geninit = TRUE;
    }
    s = o->o_sym;
    g = o->o_asig;
    if (g->g_type == T_ARRAY) {
	if (s->s_imp)			/* if exported */
	    cprintf ("sr_init_array(%t,%n,", g->g_ub, s);
	else				/* if private */
	    cprintf ("%n=sr_init_array(%t,(Array*)0,", s, g->g_ub);
	cprintf ("%d,NULL,%d", sizeof (Ocap), ndim (o->o_asig));
	while (g->g_type == T_ARRAY) {
	    cprintf (",%e,%e", g->g_lb, g->g_ub);
	    if (o->o_exported)
		cprintf ("%1%e, %e%,", g->g_lb, g->g_ub);
	    g = g->g_usig;
	}
	cprintf (");\n");
    }
    cprintf ("sr_make_inops((Ptr)");
    if (o->o_asig->g_type != T_ARRAY)
	cprintf ("&");
    cprintf ("%n,%C,%d,%d);\n", s, o->o_class, ndim (o->o_asig), INPUT_OP);
}


/*  makesemop (o, e)
 *
 *  Generate code to create a single sem or an array of sem op for o
 *  e is used only for generating traceback info.
 *  Code is similar to makeop code.
 */

void
makesemop (o, e)
Opptr o;
Nodeptr e;
{
    Symptr s;
    Sigptr g;
  
    ASSERT (!o->o_exported);
    s = o->o_sym;
    g = o->o_asig;
    ASSERT (g->g_type == T_OP || g->g_type == T_ARRAY);
    if (g->g_type == T_OP) {
	cprintf ("%n=sr_make_semop(%t);\n", s, e);
	return;
    }

    /* know (g->g_type == T_ARRAY) */
    ASSERT (!s->s_imp);			/* not exported */
    cprintf ("%n=sr_init_array(%t,(Array*)0,", s, g->g_ub);
    cprintf ("%d,NULL,%d", sizeof (Sem), ndim (o->o_asig));
    while (g->g_type == T_ARRAY) {
	cprintf (",%e,%e", g->g_lb, g->g_ub);
	g = g->g_usig;
    }
    cprintf (");\n");

    cprintf ("sr_make_arraysem(%t, (Ptr)", e);
    if (o->o_asig->g_type != T_ARRAY)
	cprintf ("&");
    cprintf ("%n,%d);\n", s, ndim (o->o_asig));
}



/*  wimpl (globalname) -- write .impl file giving info about global ops  */

#define SRC_PREFIX "#  from   "

void
wimpl (gname)
char *gname;
{
    FILE *f;
    Opptr o;
    Symptr s;
    char c, *p;
    char fname[MAX_PATH];

    sprintf (fname, "%s/%s%s", ifdir, gname, IMPL_SUF);
    f = fopen (fname, "w");
    if (!f)
	pexit (fname);
    fprintf (f, "#  global %s\n%s", gname, SRC_PREFIX);
    if (srcname[0][0] != '/')
	fprintf (f, "%s/", cwd);
    fprintf (f, "%s\n\n", srcname[0]);
    FOREACH (p, oplist) {
	o = (Opptr) p;
	s = o->o_sym;
	if (!IMPORTED (s) && o->o_seg == SG_GLOBAL) {
	    switch (o->o_impl) {
		case I_DCL:			continue;  /* not an op */
		case I_EXTERNAL:    c = 'x';	break;
		case I_PROC:	    c = 'p';	break;
		case I_IN:	    c = 'i';	break;
		case I_CAP:	    c = 'i';	break;	  /* must be I_IN */
		case I_UNIMPL:	    c = 'i';	break;	  /* must be I_IN */
		default:  BOOM ("unexpected o_impl value", s->s_name);
	    }
	    fprintf (f, "%c%c %s\n", c, o->o_sepctx ? '*' : ' ', s->s_name);
	}
    }
    fclose (f);
}



/*  rimpl (e) -- read .impl file for global identified by node e  */

void
rimpl (e)
Nodeptr e;
{
    Symptr s, t;
    FILE *f;
    char *gname;
    char line[MAX_LINE];
    char implname[MAX_PATH], *srcname, *opname;

    /*
     *  Get the global name and open its .impl file.
     */
    ASSERT (e->e_opr == O_SYM);
    s = e->e_sym;
    gname = s->s_name;
    f = pathopen (gname, IMPL_SUF, implname);
    if (!f) {
	err (e->e_locn,
	    "cannot import global %s until its body has been compiled", gname);
	return;
    }

    /*
     *  Read the header; extract the source file name and check its timestamp.
     */
    fgets (line, sizeof (line), f);		/* skip first header line */
    line[0] = '\0';				/* initialize to nothing */
    fgets (line, sizeof (line), f);		/* read second header line */
    if (strncmp (line, SRC_PREFIX, strlen (SRC_PREFIX)) != 0)
	err (e->e_locn, "bad .impl file for global %s", gname);
    line [strlen (line) - 1] = '\0';		/* remove newline */
    srcname = line + strlen (SRC_PREFIX);	/* find source file name */

    if (modtime (srcname) > modtime (implname))	/* check that .impl is current*/
	err (E_WARN + e->e_locn,
	    "source for global %s is newer than .impl", gname);

    /*
     *  For each op in the file, set flags in the corresponding op structure.
     */
    fgets (line, sizeof (line), f);		/* skip blank line */
    while (fgets (line, sizeof (line), f)) {
	line [strlen (line) - 1] = '\0';	/* remove newline */
	opname = line + 3;			/* find op name */
	for (t = s->s_next; t != NULL; t = t->s_next)
	    if (t->s_imp == s && strcmp (t->s_name, opname) == 0)
		break;
	if (t == NULL)
	    err (e->e_locn,
		"found op %s in .impl but it was not imported", opname);
	else {
	    switch (line[0]) {
		case 'i':  t->s_op->o_impl = I_IN;		break;
		case 'x':  t->s_op->o_impl = I_EXTERNAL;	break;
		case 'p':  t->s_op->o_impl = I_PROC;		break;
	    }
	    if (line[1] == '*')
		t->s_op->o_sepctx = TRUE;
	}
    }
    fclose (f);

    /*
     *  Check to make sure that all ops were accounted for.
     */
    for (t = s->s_next; t != NULL; t = t->s_next)
	if (t->s_imp == s && t->s_kind == K_OP
	&& t->s_op != NULL && t->s_op->o_impl == I_UNIMPL)
	    err (e->e_locn,
		"op %s not found in .impl file of global", t->s_name);
}



/*  dumpops () -- print op list  */

void
dumpops ()
{
    Opptr o;
    char *p;

    printf ("\nops:\n");
    FOREACH (p, oplist) {
	o = (Opptr) p;
	printf ("  ");
	putchar (o->o_exported ? 'x' : ' ');
	switch (o->o_seg) {
	    case SG_RESOURCE:	putchar ('R');  break;
	    case SG_IMPORT:	putchar ('I');  break;
	    case SG_GLOBAL:	putchar ('G');  break;
	    case SG_PROC:	putchar ('P');  break;
	    default:		putchar ('?');  break;
	}
	switch (o->o_impl) {
	    case I_UNIMPL:	putchar ('u');  break;
	    case I_EXTERNAL:	putchar ('x');  break;
	    case I_PROC:	putchar ('p');  break;
	    case I_IN:		putchar ('i');  break;
	    case I_SEM:		putchar ('s');  break;
	    case I_DCL:		putchar ('d');  break;
	    case I_CAP:		putchar ('c');  break;
	    default:		putchar ('?');  break;
	}
	putchar (o->o_sepctx ? '*' : ' ');
	putchar (' ');
	psym (o->o_sym);
	printf ("\n");
    }
}



/*  dumpclass () -- print class list entries  */

void
dumpclass ()
{
    char *p;
    Classptr c;
    Opptr o;

    printf ("\nop classes:\n");
    FOREACH (p, classlist) {
	c = (Classptr) p;
	if (c->c_members > 0) {
	    printf ("%3d.", c->c_num);
	    switch (c->c_seg) {
		case SG_RESOURCE:	putchar ('R');  break;
		case SG_IMPORT:		putchar ('I');  break;
		case SG_GLOBAL:		putchar ('G');  break;
		case SG_PROC:		putchar ('P');  break;
		default:		putchar ('?');  break;
	    }
	    for (o = c->c_first; o != NULL; o = o->o_classmate)
		printf (" %s", o->o_sym->s_gname);
	    printf ("\n");
	}
    }
}
