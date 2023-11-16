/*  main.c -- sr compiler main program, file handling  */

#include <ctype.h>
#include <signal.h>
#include "compiler.h"
#include "../paths.h"
#include "../arch.h"

static void options	PARAMS ((int argc, char *argv[]));
static void dofile	PARAMS ((char *filename, int reason));
static int  concrete	PARAMS ((Nodeptr e));
static void genmake	PARAMS ((Nodeptr e));
static void gmake	PARAMS ((Nodeptr e, char *rtype, char *half));
static void link	PARAMS ((char *files[]));



/*  keep first, so it will be first in the binary  */

static char version[] = VERSION;



/*
 *  Globals are *declared* in "globals.h".
 *  They are now *defined* (instantiated) here.
 */
#undef GLOBAL
#undef INIT
#define GLOBAL
#define INIT(v) = v
#include "globals.h"



/*  code generation file info  */

static char *ifhome;			/* Interfaces directory location */
static char *oname;			/* output file name */



/*  main program  */

main (argc, argv)
int  argc;
char *argv[];
{
    int i, oldw;
    extern int optind;			/* next option index (getopt) */

    xprefix = argv[0];			/* set exit message prefix */
    options (argc, argv);		/* process options */

    if (option_v) {
#ifdef MULTI_SR
	fprintf (stderr, "Multi%s\n", version);
#else
	fprintf (stderr, "%s\n", version);
#endif
    }

    if (optind >= argc)
	exit (0);			/* if no files, just quit */

    if (getcwd (cwd, MAX_PATH) == 0)	/* get current working directory */
	mexit ("can't get current directory");

#ifdef SIGBUS
    signal (SIGBUS, handler);		/* catch aborts to flush errors */
#endif
    signal (SIGSEGV, handler);

    if (option_M) {			/* if -M, generate Makefile info */
	for (i = optind; i < argc; i++)
	    if (strtail (argv[i], SOURCE_SUF))
		dofile (argv[i], 'M');
	exit (tfatals);
    }

    if (option_s || !option_b) {	/* if spec files are to be generated */
	oldw = option_w;
	if (option_b || !option_s)	/* if body pass is to follow */
	    option_w = TRUE;		/* disable warnings on this pass */
	for (i = optind; i < argc; i++)
	    if (strtail (argv[i], SOURCE_SUF))
		dofile (argv[i], 's');	/* process each file in turn */
	if (tfatals)
	    exit (tfatals);		/* quit now if any fatal errors seen */
	option_w = oldw;
    }

    if (option_b || !option_s)		/* if bodies (object files) wanted */
	for (i = optind; i < argc; i++)
	    if (strtail (argv[i], SOURCE_SUF))
		dofile (argv[i], 'b');

    if (tfatals || option_c) {		/* if can't, or don't want, link */
	exit (tfatals);
	/*NOTREACHED*/
    } else {
	if (!option_q)
	    fprintf (stderr, "linking:\n");
	link (argv + optind);		/* exec srl to build executable file */
	/*NOTREACHED*/
    }
}



/*  options(argc,argv) -- process command options  */

static void
options (argc, argv)
int argc;
char *argv[];
{
    int c;
    char *p, *q;
    char buf[MAX_PATH];
    extern int optind;
    extern char *optarg;

/* the "+" getopt arg works around gratuitous GNU getopt incompatibility */
#define USAGE \
"usage:  sr [-sbcqwegvCFOPMT] [-I dir] [-o file] [-d flags] file.sr ..."
    while ((c = getopt (argc, argv, "+sbcqwegvCFOPMTI:o:d:")) != EOF)
	switch (c)  {
	    case 'b':	option_b = option_c = TRUE;	break;
	    case 's':	option_s = option_c = TRUE;	break;
	    case 'C':	option_C = option_c = TRUE;	break;
	    case 'c':	option_c = TRUE;		break;
	    case 'M':	option_M = option_q = TRUE;	break;
	    case 'e':	option_e = TRUE;		break;
	    case 'g':	option_g = TRUE;		break;
	    case 'q':	option_q = TRUE;		break;
	    case 'F':	option_F = TRUE;		break;
	    case 'O':	option_O = TRUE;		break;
	    case 'P':	option_P = TRUE;		break;
	    case 'T':	option_T = TRUE;		break;
	    case 'v':	option_v = TRUE;  trcexec = 1;	break;
	    case 'w':	option_w = TRUE;		break;

	    case 'I':
		if (ifhome)
		    mexit ("duplicate -I option");
		ifhome = optarg;
		break;

	    case 'o':
		if (oname)
		    mexit ("duplicate -o option");
		oname = optarg;
		break;

	    case 'd':
		option_d = TRUE;
		for (p = optarg; c = *p; p++)
		    if (c == 'a')
			for (q = DEBUG_DEFAULT; *q; q++)
			    dbflags[*q] = 1;
		    else if (c == 'A')
			memset (dbflags, 1, sizeof (dbflags));
		    else
			dbflags[c] ^= 1;
		break;

	    default:
		mexit (USAGE);
	}

    if (oname && option_c)		/* msg if -o useless */
	fprintf (stderr, "%s: -o ignored\n", argv[0]);

    if (optind + 1 == argc && !option_v)  /* if just one file, set -q option */
	option_q = TRUE;

    sprintf (buf, "%s/%s", ifhome ? ifhome : ".", INTER_DIR);
    ifdir = salloc (buf);

    setpath (ifdir, (int) option_e);
}



/*  dofile(filename, reason) -- Compile one file
 *
 *  reason is one of:
 *	'M' : generate Makefile information to stdout
 *	's' : generate *.spec files in Interfaces directory
 *	'b' : generate .o (body) files in Interfaces directory
 */

static void
dofile (filename, reason)
char *filename;
int reason;
{
    Nodeptr e;
    FILE *fp;
    void *old;

#ifdef YYDEBUG
    extern int yydebug;			/* yacc debugging flag */
    yydebug = dbflags['y'];
#endif
    
    fp = mustopen (filename, "r");
    srcname[0] = filename;
    if (!option_q)
	fprintf (stderr, "%s:\n", srcname[0]);
    srclocn = 1;
    old = setinput (fp);		/* set input file */

    initmem ();				/* initialize memory */
    allow_echo = (reason == 's');	/* enable .spec iff this is the time */

    while ((e = parse ()) != NULL) {	/* for each resource */
	resname = salloc (e->e_l->e_name);  /* save resource name permanantly */
	switch (reason) {
	    case 'M':
		genmake (e);
		break;
	    case 's':
		/* nothing to do, was echoed during parsing */
		break;
	    case 'b':
		if (concrete (e->e_r))	/* if this resource has a body */
		    resource (e);	/* generate it */
		break;
	}
	errflush ();			/* flush errors */
	initmem ();			/* reset globals */
    }

    errflush ();			/* flush any errors outside resource */
    fclose (fp);			/* close source file */
    resetinput (old);
}



/*  concrete (e) -- is e a concrete resource (or global) with body present?  */

static int
concrete (e)
Nodeptr (e);
{
    if (! (e = e->e_r))
	return FALSE;			/* no BODY node */
    if (! (e = e->e_r))
	return TRUE;			/* yes, empty body */
    if (e->e_opr == O_SEPARATE)
	return FALSE;			/* no, body is separate */
    else
	return TRUE;			/* yes, nonempty body */
}



/*  genmake (e) -- generate Makefile dependencies for resource  */

static void
genmake (e)
Nodeptr e;
{
    Bool hasspec, hasbody;
    char *rtype;

    e = e->e_r;
    if (e->e_opr == O_GLOBAL)
	rtype = "global";
    else {
	ASSERT (e->e_opr == O_RESOURCE);
	rtype = "resource";
    }
    hasspec = (e->e_l != NULL);
    hasbody = concrete (e);
	
    if (hasspec)
	printf ("%s %s spec source %s\n", rtype, resname, srcname[0]);
    if (hasbody)
	printf ("%s %s body source %s\n", rtype, resname, srcname[0]);
    if (hasspec)
	gmake (e->e_l->e_l, rtype, "spec");
    if (hasbody)
	gmake (e->e_r->e_r, rtype, "body");
}

static void
gmake (e, rtype, half)
Nodeptr e;
char *rtype, *half;
{
    if (e) switch (e->e_opr) {
	case O_IMPORT:
	case O_EXTEND:
	    for (e = e->e_l; e != NULL; e = e->e_r)
		printf ("%s %s %s imports %s\n",rtype,resname,half,e->e_name);
	    break;
	default:
	    if (LNODE (e))
		gmake (e->e_l, rtype, half);
	    if (RNODE (e))
		gmake (e->e_r, rtype, half);
	    break;
    }
}



/*  cc (dir, fname) -- compile a C file in the specified directory  */

void
cc (dir, fname)
char *dir, *fname;
{
    char *argv[20];
    char **argp = argv;

    if (option_C)
	return;
    if (tfatals == 0)  {
	*argp++ = CCPATH;
#ifndef __PARAGON__         /* Paragon doesn't understand -w! */
	*argp++ = "-w";
#endif
	if (option_g)
	    *argp++ = "-g";
	if (option_O)
	    *argp++ = "-O";
	*argp++ = "-c";

#ifdef BIGCC		/* if arch needs cc arg for bigger tree space */
	*argp++ = BIGCC;
#endif

#ifdef MULTI_CC_OPT	/* if need special option for MultiSR */
	*argp++ = MULTI_CC_OPT;
#endif

	*argp++ = fname;
	*argp++ = 0;
	if (spawn (argv, dir) != 0)
	    BOOM ("compiler generated bad code", fname);
    }
}



/*  link (files) -- run srl to link the components we have compiled
 *
 *  "files" is portion of orig argv containing file names (including SR files).
 */

static void
link (files)
char *files[];
{
    char path [MAX_PATH];
    char **p, *s;
    char *argv [MAX_RES_DEF+20];	/* argument vector */
    char **argp = argv;

    /* build argument vector */

    if (option_e)
	sprintf (path, "%s/srl/srl", SRSRC);
    else
	sprintf (path, "%s/srl", SRCMD);
    *argp++ = path;
    if (option_e)
	*argp++ = "-e";
    if (option_g)
	*argp++ = "-g";
    if (option_v)
	*argp++ = "-v";
    if (ifhome)  {
	*argp++ = "-I";
	*argp++ = ifhome;
    }
    if (oname)  {
	*argp++ = "-o";
	*argp++ = oname;
    }

    /* first add any imported resources we didn't compile ourselves  */
    ilp = ilist;
    while (s = *ilp++) {
	for (p = rlist; p < rlp; p++) 
	    if (strcmp (*p, s) == 0)	/* if import matches compiled res */
		break;
	if (p == rlp)			/* if was not duplicate */
	    *argp++ = s;		/* add to arg vector */
    }

    /* now add compiled resource list (last one becomes main resource) */
    rlp = rlist;
    if (!*rlp)
	mexit ("no resource bodies; linking suppressed");
    while (*argp++ = *rlp++)		/* copy list */
	;

    /* finally, add any non-.sr files from command line */
    argp--;				/* back up past terminator */
    while ((s = *files++) != NULL)
	if (!strtail (s, SOURCE_SUF))
	    *argp++ = s;
    *argp++ = NULL;

    /* exec srl */

#if defined(__SABER__) || defined(__CENTERLINE__)
    mexit ("can't link while running under Saber C");
#endif

    doexec (argv);
}
