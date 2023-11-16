/*  symtab.c -- symbol table management  */

#include "compiler.h"
#include <ctype.h>



typedef struct {	/* structure for noting a forward type reference */
    Nodeptr f_ref;	/* pointer to node making reference */
    Symptr f_loc;	/* current symtab locn at time of reference */
} FwdRef, *Refptr; 



static void resspec	PARAMS ((Nodeptr e));
static Symptr pushblock	PARAMS ((char *name));
static void popblock	PARAMS ((NOARGS));
static Symptr tailsym	PARAMS ((Kind kind, char *name));
static Symptr newsym	PARAMS ((Kind kind, char *name, Symptr prev));
static Symptr lookup	PARAMS ((Nodeptr e, int givemsg));
static Symptr define	PARAMS ((Nodeptr e, Kind k));
static Bool samectx	PARAMS ((Symptr s, Symptr t));
static void deflist	PARAMS ((Nodeptr e, Kind k));
static void opdefs	PARAMS ((NOARGS));
static void enumdefs	PARAMS ((NOARGS));
static Sigptr enumlist	PARAMS ((char *typename, char *names[]));
static void fixfwd	PARAMS ((Refptr f));



static Symptr defctx;			/* definition context */
static Symptr refctx;			/* reference context */

static Symptr tail;			/* tail pointer for current scope */
static Symptr blktail[MAX_NESTING];	/* tail pointers for nested blocks */
static Symptr blkctx[MAX_NESTING];	/* import context for each level */

static int depth;			/* current nesting depth */

static int capnum;			/* anonymous cap number */

List fwdlist;				/* list of forward references */



/*  initsym() -- initialize the symbol table, including the predefs  */

void
initsym ()
{
    Symptr s;
    Varptr v;

    depth = 0;
    capnum = 0;
    defctx = NULL;
    tail = newsym (K_BLOCK, NULLSTR, NULLSYM);
    blktail[0] = tail;			/* start with root entry */
    pushblock ("[root]");		/* descend one level for the predefs */
    opdefs ();				/* predefined operations */
    enumdefs ();			/* predefined enums */

    s = tailsym (K_VAR, unique ("EOF"));/* the "EOF" constant */
    v = newvar (s, V_GLOBAL, O_CONDCL, int_sig);
    v->v_value = intnode (EOF);
    v->v_set = v->v_used = TRUE;
}



/*  dumpsyms(p) -- dump symbol table;  p=0 for all, p=1 to exclude predefs  */

void
dumpsyms (p)
int p;
{
    Symptr s;

    printf ("\nsymbol table:\n");
    s = blktail[p];
    pstab (s);
}



/*  identify(e) -- resolve identifiers in tree e
 *
 *  Walk the tree and change each O_ID node into an O_SYM node.
 */

void
identify (e) 
Nodeptr e;
{
    Symptr s, t, rs, dx, rx;
    char *buf, *name;
    Nodeptr l, r;
    Kind k;
    Refptr f;
    int i;

    if (!e)
	return;
    l = LNODE (e);
    r = RNODE (e);
    switch (e->e_opr) {

	case O_COMPONENT:
	    ASSERT (r->e_l->e_opr == O_SPEC);
	    ASSERT (r->e_r->e_opr == O_BODY);
	    fwdlist = list (sizeof (FwdRef));	/* make list for fwd refs */
	    s = pushblock ("[resource]");
	    rs = define (l, K_RES);		/* define component name */
	    defctx = refctx = rs;		/* set context */
	    resspec (e);			/* process spec */
	    for (t = rs->s_res->r_parm->s_child; t != NULL; t = t->s_next) {
		k = t->s_kind;
		/* check all param kinds; errors are caught later */
		if (k == K_FVAL || k == K_FVAR || k == K_FRES || k == K_FREF)
		    tailsym (K_VAR, t->s_name);	/* declare var for each parm */
		}
	    defctx = NULL;			/* clear definition context */
	    identify (r->e_r);			/* process body */
	    popblock ();
	    while ((f = (Refptr) lpop (fwdlist)) != NULL)
		fixfwd (f);			/* fix forward ref */
	    break;

	case O_IMPORT:
	    dx = defctx;			/* save symtab context */
	    rx = refctx;
	    if (r) {
		/* not a duplicate import; need to do some work */
		ASSERT (r->e_l->e_opr == O_SPEC);
		pushblock ("[import]");
		defctx = NULL;			/* clear for defining rsc id */
		rs = define (l, K_RES);		/* define resource */
		defctx = refctx = rs;		/* set context */
		resspec (e);			/* process spec */
		popblock ();
		/* now export the symbols from one level deeper (only) */
		for (s = rs; s != NULL; s = s->s_next) {
		    if (isalpha (s->s_name[0]) && s->s_kind != K_IMP) {
			t = tailsym (K_IMP, s->s_name);
			t->s_child = s;
			t->s_imp = s->s_imp;
		    }
		}
	    } else {
		/* duplicate import; look up previous symtab entry */
		identify (l);
	    }
	    defctx = dx;			/* restore context */
	    refctx = rx;
	    break;

	case O_OP:
	case O_EXTERNAL:
	    define (l->e_l, K_OP);
	    identify (l->e_r);
	    identify (r);
	    break;
	case O_CAP:
	    if (l->e_opr == O_PROTO && l->e_l->e_l->e_l->e_l->e_opr == O_ID) {
		buf = ralloc (8);
		sprintf (buf, "%d", ++capnum);
		define (idnode (buf), K_OP);
	    }
	    identify (l);
	    break;
	case O_PROC:
	    ASSERT (r->e_opr == O_BLOCK);
	    name = r->e_l->e_name;
	    identify (r->e_l);			/* identifier */
	    buf = ralloc (strlen (name) + 3);
	    sprintf (buf, "(%s)", name);	/* create different name */
	    pushblock (buf);			/* (don't displace op entry) */
	    deflist (l, K_VAR);			/* params */
	    identify (r->e_r);			/* statements */
	    popblock ();
	    break;
	case O_INPARM:
	    identify (l);			/* idsubs */
	    deflist (r, K_VAR);			/* define params */
	    break;
	case O_PROCESS:
	    identify (l);
	    if (r) {				/* if any process quantifiers */
		pushblock ("[pquant]");		/* don't want them global */
		identify (r);
		popblock ();
	    };
	    break;
	case O_FINAL:
	    pushblock ("[final]");
	    identify (l);
	    popblock ();
	    break;
	case O_BLOCK:
	    if (l != NULL && e->e_l->e_opr == O_ID)
		pushblock (l->e_name);
	    else
		pushblock ("[block]");
	    identify (r);
	    popblock ();
	    break;
	case O_PROTO:
	    pushblock (NULLSTR);
	    identify (l);			/* return val & parameters */
	    popblock ();
	    break;
	case O_PARDCL:
	    switch (r->e_r->e_opr) {
		case O_VAL:	k = K_FVAL;	break;
		case O_VAR:	k = K_FVAR;	break;
		case O_RES:	k = K_FRES;	break;
		case O_REF:	k = K_FREF;	break;
		default:	BOOM ("bad param kind", "");
	    }
	    define (l->e_l, k);
	    identify (l->e_r);
	    identify (r);
	    break;
	case O_VARDCL:
	case O_CONDCL:
	case O_FLDDCL:
	    identify (l->e_r);
	    identify (r);
	    define (l->e_l, K_VAR);
	    break;

	/* In order to handle forward type definitions, we allocate the space
	 * for the signature here.  Then in attest(), the signature can be
	 * referenced even before it's filled in.
	 */
	case O_TYPE:
	case O_OPTYPE:
	    define (l, K_TYPE);			/* define type name */
	    ASSERT (l->e_opr == O_SYM);		/* no subscripts etc. allowed */
	    if (e->e_opr == O_OPTYPE) {
		ASSERT (r->e_opr == O_PROTO);
		buf = ralloc (8);
		sprintf (buf, "%d", ++capnum);
		define (idnode (buf), K_OP);
	    }
	    l->e_sym->s_sig = NEW (Signat);	/* alloc space for signature */
	    identify (r);			/* process type definition */
	    break;

	case O_PTR:
	    if (l->e_opr == O_TYPENAME && l->e_l->e_opr == O_ID
		    && lookup (l->e_l, FALSE) == NULL) {
		/* we have an undefined ID, presumably a forward reference. */
		f = (Refptr) lput (fwdlist);	/* append new list entry */
		f->f_ref = l;			/* save node pointer */
		f->f_loc = tail;		/* save current context */
	    } else
		identify (l);
	    break;

	case O_REC:
	    pushblock ("[record]");
	    identify (l);
	    popblock ();
	    break;

	case O_ENUM:
	    if (l->e_l->e_opr != O_SYM) {	/* if not already processed */
		i = 0;
		while (l) {
		    ASSERT (l->e_opr == O_LIST);
		    ASSERT (l->e_l->e_opr == O_ID);
		    if (lookup (l->e_l, FALSE))
			err (l->e_locn, "enum literal duplicates id `%s'",
			    l->e_l->e_name); 
		    define (l->e_l, K_ELIT);
		    l->e_l->e_sym->s_seq = i++;
		    l = l->e_r;
		}
	    }
	    break;
	case O_FA:
	    pushblock ("[fa]");
	    identify (l);
	    ASSERT (r->e_opr == O_BLOCK);	/* child is a BLOCK node */
	    identify (r->e_r);			/* skip it, nest less deeply */
	    popblock ();
	    break;
	case O_QUANT:
	    ASSERT (l->e_opr == O_QSTEP);
	    ASSERT (r->e_opr == O_QTEST);
	    ASSERT (r->e_l->e_l->e_opr == O_ID);
	    /* important: follow this same sequence in attest(): */
	    identify (l->e_l);			/* initial */
	    define (r->e_l->e_l, K_VAR);	/* NOW define quant variable */
	    identify (r->e_l->e_l);
	    identify (r->e_l->e_r);		/* limit */
	    identify (l->e_r);			/* increment */
	    identify (r->e_r);			/* suchthat */
	    break;
	case O_COINV:
	    pushblock ("[//]");
	    identify (l);
	    identify (r);
	    popblock ();
	    break;
	case O_ARM:
	    pushblock ("[in]");
	    identify (l);
	    identify (r);
	    popblock ();
	    break;
	case O_CALL:
	case O_SEND:
	case O_FORWARD:
	    identify (l);
	    identify (r);
	    if (e->e_opr == O_CALL && l->e_opr == O_SYM)
		if (l->e_sym->s_kind == K_PREDEF)
		    e->e_opr = O_LIBCALL;
		else if (l->e_sym->s_kind == K_TYPE)
		    e->e_opr = O_CAST;
	    break;
	case O_FIELD:
	    identify (l);
	    /* defer resolving field name until later, when have signatures */
	    break;
	case O_ID:
	    if ((s = lookup (e, TRUE)) != NULL) {
		if (s->s_kind == K_IMP)		/* if imported */
		    s = s->s_child;		/* follow link to real entry */
		e->e_opr = O_SYM;
		e->e_sym = s;
		switch (s->s_kind) {
		    case K_FVAL:
		    case K_FVAR:
		    case K_FRES:
		    case K_FREF:
			err (e->e_locn,
			    "illegal reference to formal parameter %s",
			    s->s_name);
		}
	    } else {
		e->e_opr = O_ILIT;
		e->e_sig = int_sig;
		e->e_int = 1;
	    }
	    break;
	default:
	    if (l)	
		identify (l);			/* process left child, if any */
	    if (r)
		identify (r);			/* process right child, if any*/
	    break;
    }
}



/*************************  INTERNAL ROUTINES  *************************/



/*  resspec(e) - process spec below COMPONENT/IMPORT/EXTEND node e  */

static void
resspec (e)
Nodeptr e;
{
    Resptr r;
    Symptr s, t;

    s = e->e_l->e_sym;			/* symtab entry for resource */
    r = NEW (Res);			/* make Res struct */
    r->r_opr = e->e_r->e_opr;		/* save O_RESOURCE or O_GLOBAL */
    r->r_sym = s;
    s->s_res = r;
    if (curres == NULL)
	curres = r;

    e = e->e_r->e_l;			/* move to SPEC node */
    ASSERT (e->e_opr == O_SPEC);

    r->r_parm = pushblock("[rparams]");	/* make param block now, for extends */
    popblock ();

    identify (e->e_l);			/* do spec contents */

    /* move the param block down to the end of the table */
    s = r->r_parm;
    if ((t = s->s_next) != NULL) {
	t->s_prev = s->s_prev;
	s->s_prev->s_next = t;
	while (t->s_next != NULL)
	    t = t->s_next;
	t->s_next = s;
	s->s_prev = t;
	s->s_next = NULL;
	blktail[depth] = s;
    }

    /* re-enter the param block and define the params */
    t = s->s_child;
    if (t != NULL)
	while (t->s_next != NULL)	/* find last entry in param block */
	    t = t->s_next;
    blkctx[depth] = defctx;		/* save context */
    defctx = NULL;
    depth++;				/* descend */
    tail = blktail[depth] = t;
    identify (e->e_r);			/* process params */
    popblock ();
}



/*  pushblock(name) -- descend into a deeper nesting level
 *
 *  But first, install name as a symbol, if present.
 */

static Symptr
pushblock (name)
char *name;
{
    Symptr s;

    if (name)
	s = tailsym (K_BLOCK, name);
    else if (blktail[depth] == NULL || blktail[depth]->s_child != NULL)
	s = tailsym (K_BLOCK, NULLSTR);
    else
	s = NULL;
    blkctx[depth] = defctx;
    if (++depth >= MAX_NESTING)
	mexit ("block nesting too deep");
    blktail[depth] = NULL;
    defctx = NULL;	/* don't need or want annotations at deeper level */
    return s;
}



/*  popblock() -- ascend one nesting level, discarding the old one  */

static void
popblock ()
{
    ASSERT (depth > 1);
    depth--;
    tail = blktail[depth];
    defctx = blkctx[depth];
}



/*  tailsym(kind,name) -- add new symbol struct at end of current block  */

static Symptr
tailsym (kind, name)
Kind kind;
char *name;
{
    Symptr s;

    if (blktail[depth]) {
	s = newsym (kind, name, blktail[depth]);
	blktail[depth]->s_next = s;
    } else {
	s = newsym (kind, name, blktail[depth-1]);
	ASSERT (blktail[depth-1]);
	blktail[depth-1]->s_child = s;
    }
    tail = blktail[depth] = s;
    return s;
}



/*  newsym(kind,name,prev) -- make new symbol struct in current context  */

static Symptr
newsym (kind, name, prev)
Kind kind;
char *name;
Symptr prev;
{
    Symptr s;
    
    s = NEW (Symbol);
    s->s_kind = kind;
    s->s_name = name;
    s->s_prev = prev;
    s->s_depth = depth;
    s->s_imp = defctx;

    if (depth <= 2 || kind == K_RES ||
	(IMPORTED (s) && (s->s_kind==K_OP || defctx->s_res->r_opr==O_GLOBAL)))
	    s->s_gname = name;		/* use declared name when possible */
    else 
	    s->s_gname = genref (name); /* use altered version for local syms */
    return s;
}



/*  newvar(sym,vty,dcl,sig) -- allocate and initialize a new Var structure */

Varptr
newvar (s, vty, d, g)
Symptr s;
Variety vty;
Operator d;
Sigptr g;
{
    Varptr v;

    ASSERT (s->s_kind == K_VAR);
    s->s_sig = g;
    s->s_var = v = NEW (Var);		/* allocate */
    v->v_vty = vty;			/* set variety */
    v->v_dcl = d;			/* set declaration type */
    v->v_sig = g;			/* set signature */
    v->v_sym = s;
    return v;				/* return new struct */
}



/*  lookup (e, givemsg) -- find the name from an ID node in the symbol table.
 *
 *  Return an imported symbol iff it is the only matching symbol that is
 *  currently in scope.
 *
 *  If not found, issue an error message if givemsg is true.
 */

static Symptr
lookup (e, givemsg)
Nodeptr e;
int givemsg;
{
    Symptr s, imps;
    int nimp;
    char *name;

    if (e->e_opr == O_SYM)
	return e->e_sym;		/* name was already resolved */
    ASSERT (e->e_opr == O_ID);
    name = e->e_name;
    nimp = 0;				/* count imported symbols seen */

    for (s = tail; s; s = s->s_prev)
	if (s->s_name == name) {	/* if name matches */
	    if (s->s_kind != K_IMP && (s->s_imp == NULL || s->s_imp == refctx))
		break;			/* if in our current resource */
	    else {
		imps = s;		/* otherwise, remember it */
		nimp ++;
	    }
	}

    if (s == NULL && nimp == 1)
	s = imps;			/* use imported sym if unique */

    if (s == NULL && givemsg) {
	if (nimp > 1)
	    err (e->e_locn, "ambiguous identifier: %s", name); 
	else
	    err (e->e_locn, "undefined identifier: %s", name); 
    }

    return s;
}



/*  deflist (e, kind) -- define a list of (possibly subscripted) IDs  */

static void
deflist (e, k)
Nodeptr e;
Kind k;
{
    while (e) {
	ASSERT (e->e_opr == O_LIST);
	define (e->e_l, k);
	e = e->e_r;
    }
}



/*  define (e, kind) -- define a symbol, changing an ID node into a SYM node.
 *
 *  e may be an ID node or SUBS node, or null.  The defined symbol is returned.
 */

static Symptr
define (e, k)
Nodeptr e;
Kind k;
{
    Symptr s;
    char *name;

    if (e == NULL)			/* if null node */
	return tailsym (k, NULLSTR);	/* just create symbol, don't chg tree*/

    while (e->e_opr == O_SUBS) {
	identify (e->e_r);		/* resolve IDs in subscripts */
	e = e->e_l;			/* move down */
    }
    if (e->e_opr == O_SYM)
	return e->e_sym;		/* we have already processed this node*/

    ASSERT (e->e_opr == O_ID);		/* now we are at the ID node */
    name = e->e_name;
    if (name != NULL && (s = lookup (e, FALSE)) != NULL) {
	if (s->s_depth == depth && samectx (s->s_imp, defctx))
	    err (e->e_locn, "duplicate identifier: %s", name);
	else if (s->s_kind == K_ELIT)
	    err (e->e_locn, "id `%s' duplicates enum literal", name);
    }
    s = tailsym (k, name);		/* add symbol table entry */
    e->e_opr = O_SYM;			/* change node to point to it */
    e->e_sym = s;
    return s;				/* return Symptr */
}



/*  samectx (s, t) -- check if two Symptrs represent the same context  */

static Bool
samectx (s, t)
Symptr s, t;
{
    if (s == NULL)
	s = curres->r_sym;
    if (t == NULL)
	t = curres->r_sym;
    return s == t;
}



/*  opdefs() -- add the predefined operations to the symbol table  */

static void
opdefs ()
{
    static struct pre_op_struct {	/* table of predefined operations */
	char *name;
	Predef which;
    } prelist[] = {
#define premac(name,minn,maxn) { LITERAL(name), PASTE(PRE_,name) },
#include "predefs.h"
#undef premac
	{ 0, PRE_BOGUS }
    }, *p;

    for (p = prelist; p->name; p++)
	(tailsym (K_PREDEF, unique (p->name))) -> s_pre = p->which;
}



/*  enumdefs() -- add the predefined enums to the symbol table  */

static void
enumdefs ()
{
    static char *modelist[] = { "READ", "WRITE", "READWRITE", 0 };
    static char *seeklist[] = { "ABSOLUTE", "RELATIVE", "EXTEND", 0 };

    mode_sig = enumlist ("accessmode", modelist);
    seek_sig = enumlist ("seektype", seeklist);
}



/*  enumlist (typename, list) -- install a list of enums, returning type  */

static Sigptr
enumlist (typename, list)
char *typename;
char *list[];
{
    Symptr tsym, s;
    Sigptr g;
    int i;

    tsym = tailsym (K_TYPE, unique (typename));
    tsym->s_sig = g = newsig (T_ENUM, NULLSIG);
    i = 0;
    while (*list) {
	s = tailsym (K_ELIT, unique (*list++));
	s->s_sig = g;
	s->s_seq = i++;
    }
    g->g_sym = tsym->s_next;
    return g;
}



/*  fixfwd (f) -- fix up forward type reference f  */

static void
fixfwd (f)
Refptr f;
{
    Nodeptr e;
    Symptr s, t;
    char *name;
    int d;

    e = f->f_ref;
    ASSERT (e->e_opr == O_TYPENAME);
    e = e->e_l;
    if (e->e_opr == O_SYM)
	return;			/* was ref'd twice, has already been fixed */
    ASSERT (e->e_opr == O_ID);
    name = e->e_name;

    s = f->f_loc;
    while (s->s_depth >= 2) {

	/* first, check later definitions in current scope */
	for (t = s->s_next; t != NULL; t = t->s_next)
	    if (t->s_name == name) {
		e->e_opr = O_SYM;	/* change node to SYM node */
		e->e_sym = t;		/* point to correct symtab entry */
		return;
	    }

	/* if that didn't work, move up to outer level */
	d = s->s_depth;
	while (s->s_depth == d)
	    s = s->s_prev;
    }

    err (e->e_locn, "%s: type never defined", e->e_name); 
    e->e_opr = O_INT;			/* make it "ptr int" */
    e->e_name = NULL;
}
