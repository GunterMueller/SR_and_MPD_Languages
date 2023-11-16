/*  import.c -- code for importing specifications  */

#include "compiler.h"



static void nestin	PARAMS ((NOARGS));
static void nestout	PARAMS ((NOARGS));
static Bool enroll	PARAMS ((Nodeptr id, List l));
static char *setspec	PARAMS ((FILE *f, int flags));
static void addimp	PARAMS ((char *name));

static int nestlvl;		/* current block nesting level */



typedef struct {
    char *name;
    int lvl;
} Impname, *Imp;

static List implist;		/* list of id's already imported */
static List extlist;		/* list of id's already extended */



/*  initimp (rname) -- initialize import list to contain only rname  */

void
initimp (rname)
char *rname;
{
    Imp i;

    nestlvl = 0;
    implist = list (sizeof (Impname));
    i = (Imp) lpush (implist);
    i->name = unique (rname);
    i->lvl = nestlvl;
    extlist = list (sizeof (Impname));
}



/*  import (e) -- expand "import" (etc.) statements, recursively, in tree e  */

void
import (e)
Nodeptr e;
{
    Bool oldecho;
    Nodeptr a, b;

    oldecho = allow_echo;			/* save old echo flag */
    allow_echo = FALSE;				/* don't rewrite .spec files */
    if (e) switch (e->e_opr) {
	case O_BLOCK:
	    nestin ();
	    import (e->e_r);
	    nestout ();
	    break;
	case O_IMPORT:
	    if (enroll (e->e_l, implist)) {
		e->e_r = readspec (e->e_l, E_IMPORT);	/* read .spec file */
		nestin ();
		import (e->e_r);		/* process recursive imports */
		nestout ();
	    }
	    break;
	case O_EXTEND:
	    if (enroll (e->e_l, implist)) {	/* start out like an import */
		e->e_r = readspec (e->e_l, E_IMPORT + E_EXTEND);
		nestin ();
		import (e->e_r);		/* process recursive imports */
		nestout ();
		if (e->e_r->e_opr == O_GLOBAL)
		    EFATAL (e, "cannot extend a global");
	    }
	    b = readspec (e->e_l, E_EXTEND);	/* read again to #include */
	    b = b->e_l->e_l;			/* find stmts inside spec */
	    import (b);				/* process recursive imports */
	    /* replace e by a SEQ node: IMPORT followed by included stmts */
	    a = bnode (O_IMPORT, e->e_l, e->e_r);
	    *e = *bnode (O_SEQ, a, b);
	    break;
	default:
	    import (LNODE (e));			/* recurse, if left is a node */
	    import (RNODE (e));			/* likewise for right */
	    break;
    }
    allow_echo = oldecho;			/* restore old flag value */
}



/*  nestin () -- descend one nesting level  */

static void
nestin ()
{
    nestlvl++;
}



/*  nestout () -- pop one level and forget imports that are now out of scope */

static void
nestout ()
{
    Imp i;

    nestlvl--;
    while ((i = (Imp) lfirst (implist)) != NULL && i->lvl > nestlvl) 
	lpop (implist);
    while ((i = (Imp) lfirst (extlist)) != NULL && i->lvl > nestlvl) 
	lpop (extlist);
}



/*  enroll (id, l) -- note id on list l, or return FALSE if already seen  */

static Bool
enroll (id, l)
Nodeptr id;
List l;
{
    Imp i;
    char *rname, *p;

    rname = id->e_name;				/* get resource name */
    FOREACH (p, l) {				/* check list of past imports */
	if (((Imp) p)->name == rname)		/* if duplicate, return */
	    return FALSE;
    }
    i = (Imp) lpush (l);			/* add to list */
    i->name = rname;
    i->lvl = nestlvl;
    return TRUE;
}



/*  readspec (id, flags) -- read Interfaces/rname.spec and rtn resource tree  */

Nodeptr
readspec (id, flags)
Nodeptr id;
int flags;
{
    Nodeptr e;
    FILE *spec;
    void *main_file;
    char specname[MAX_PATH], *srcname;
    int main_locn;

    spec = pathopen (id->e_name, SPEC_SUF, specname);
    if (!spec) {
	err (id->e_locn, "can't open spec file %s", specname);
	return NULL;
    }

    updtloc ();					/* ensure srclocn up to date */

    main_file = setinput (spec);		/* redirect input to spec file*/
    main_locn = srclocn;			/* remember old line number */
    srcname = setspec (spec, flags);		/* set input file name */
    if (!srcname) {
	err (id->e_locn, "misformatted spec file %s", specname);
	return NULL;
    }

    if (modtime (srcname) > modtime (specname))
	err (E_WARN + srclocn + 2, "source file is newer than %s", specname);

    e = parse ();				/* parse the .spec file */
    if (e->e_r->e_r)				/* if not abstract resource */
	addimp (id->e_name);			/* remember name for srl */

    fclose (spec);				/* close .spec file */
    resetinput (main_file);			/* reset input */
    srclocn = main_locn;			/* restore line number */
    return e->e_r;				/* return resource tree */
}



/*  setspec (specfile, flags) -- record/return file name fm spec; set srclocn */

static char *
setspec (spec, flags)
FILE *spec;
int flags;
{
    int i, len;
    int tmp;
    int sline;
    char sname[MAX_SRC];
    char tmp_name[MAX_SRC];

    /* read the header information of spec */
    i = fscanf (spec, "# %d %s %d+", &tmp, tmp_name, &sline);
    if (i != 3)
	return NULL;

    len = strlen (tmp_name);
    for (i = len - 1; i > 0 && tmp_name[i] !='/'; i--)
	;
    strcpy (sname, tmp_name + i + 1);
    
    for (i = 0; i < MAX_SRC && srcname[i] != NULL; i++)
	if (strcmp (sname, srcname[i]) == 0)
	    break;
    if (i >= MAX_SRC) {			/* if list is full */
	errflush ();
	mexit ("too many import files");
    }
    if (srcname[i] == NULL)		/* if first time, allocate name */
	srcname[i] = salloc (sname);
    srclocn = flags | (i << E_FSHIFT);	/* set file name and flags */
    srclocn += (sline - 2);		/* set line number */
    return srcname[i];
}



/*  addimp (name) -- remember name for srl  */

static void
addimp (name)
char *name;
{
    char **p;
    for (p = ilist; p < ilp; p++)
	if (strcmp (name, *p) == 0)
	    return;				/* if already on list */
    if (ilp >= ilist + MAX_RES_DEF)
	mexit ("too many imported resources");
    *ilp++ = salloc (name);
}
