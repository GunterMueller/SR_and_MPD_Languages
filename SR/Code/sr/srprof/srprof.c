/*  srprof.c -- summarize SR_TRACE info to create program profile  */

#include <ctype.h>
#include <stdio.h>
#include "../gen.h"
#include "../util.h"

#define MAXLINES 10000		/* max lines in a source file */

struct evcount {		/* struct for counting events on a line */
    struct evcount *next;
    char *event;
    long count;
};

struct srfile {			/* struct for each source file */
    struct srfile *next;
    char *name;
    struct evcount *ev[MAXLINES+1];
};

struct srfile *getfile ();
struct evcount *getevent ();

char *tname;			/* trace file name, if any */
FILE *tfile;			/* trace file */
int aflag;			/* -a (annotate listing) flag */
struct srfile *flist;		/* list of mentioned files */



/*  main program  */

main (argc, argv)
int argc;
char *argv[];
{
    char buf[200], fname[200], proc[200], event[200];
    int lno;
    struct srfile *f;
    struct evcount *e;

    options (argc, argv);			/* process options */
    xprefix = argv[0];				/* save program name */
    if (tname)
	tfile = mustopen (tname, "r");		/* open input file */
    else
	tfile = stdin;

    while (fgets (buf, sizeof (buf), tfile))	/* for each line: */
	if (sscanf (buf, "%s %d %s %s", fname, &lno, proc, event) == 4) {
	    if (lno < 0 || lno > MAXLINES)
		continue;
	    /* this is a valid trace event */
	    fname [strlen (fname) - 1] = '\0';	/* chop comma */
	    f = getfile (fname);		/* get list for src file */
	    e = getevent (f, lno, event);	/* get event counter */
	    e->count++;				/* incr count */
	}

    for (f = flist; f != NULL; f = f->next)
	report (f);				/* report each source file */

    exit (0);
    /*NOTREACHED*/
}



/*  options (argc, argv) -- process command options  */

options (argc, argv)
int argc;
char *argv[];
{
    int c;
    extern int optind;
    extern char *optarg;

    /* parse command line options. */
#define USAGE "usage: srprof [-a] [file]"
    while ((c = getopt (argc, argv, "a")) != EOF)
	switch (c)  {
	    case 'a':
		aflag++;
		break;
	    default:
		mexit (USAGE);
	}
    if (argc - optind > 1)
	mexit (USAGE);
    if (optind < argc)
	tname = argv[optind];
}



/*  getfile (filename) -- find srfile struct for the given file  */

struct srfile *getfile (fname)
char *fname;
{
    struct srfile *f, **p;

    p = &flist;
    while ((f = *p) != NULL) {
	if (strcmp (f->name, fname) == 0)
	    return f;
	p = &f->next;
	}
    /*
     * Need to create a new struct.
     * Install at end of list to preserve order.
     */
    f = (struct srfile *) alloc (sizeof (struct srfile));
    memset ((char *) f, 0, sizeof (struct srfile));
    f->name = salloc (fname);
    *p = f;
    return f;
}



/*  getevent (f, lno, event) -- find evcount struct for the given event */

struct evcount *
getevent (f, lno, event)
struct srfile *f;
int lno;
char *event;
{
    struct evcount *e, **p;

    p = &f->ev[lno];
    while ((e = *p) != NULL) {
	if (strcmp (e->event, event) == 0)
	    return e;
	p = &e->next;
	}
    /*
     * Need to create a new struct.
     * Install at end of list to preserve order.
     */
    e = (struct evcount *) alloc (sizeof (struct evcount));
    e->event = salloc (event);
    e->count = 0;
    e->next = 0;
    *p = e;
    return e;
}



/*  report(f) -- produce report for file f  */

report (f)
struct srfile *f;
{
    FILE *fp;
    int lno;
    struct evcount *e;

    if (aflag) {
	if (fp = fopen (f->name, "r")) {
	    annotate (fp, f);
	    return;
	} else
	    perror (f->name);
	    /* and fall through to non-annotated report */
    }

    /* report summary */
    for (lno = 1; lno <= MAXLINES; lno++)
	if ((e = f->ev[lno]) != NULL) {
	    printf ("%s,%4d ", f->name, lno);
	    putcounts (e);
	}
    if (f->next != NULL)
	putchar ('\n');
}



/*  annotate (fp, f) -- produce annotated listing for file f  */

annotate (fp, f)
FILE *fp;
struct srfile *f;
{
    int lno, c;
    char buf[500], *p;
    struct evcount *e;

    printf ("#====================\n");
    printf ("#  %s\n", f->name);
    printf ("#====================\n");
    for (lno = 1; lno <= MAXLINES; lno++) {
	if (!fgets (buf, sizeof (buf), fp))
	    break;
	fputs (buf, stdout);
	e = f->ev[lno];
	if (e != NULL) {
	    for (p = buf; isspace (c = *p++) && c != '\n'; )
		putchar (c);
	    putchar ('#');
	    putcounts (e);
	    }
	}
    for (; lno <= MAXLINES; lno++)
	if ((e = f->ev[lno]) != NULL) {
	    printf ("(beyond EOF) %s,%4d ", f->name, lno);
	    putcounts (e);
	}
    if (f->next != NULL)
	putchar ('\n');
}



/*  putcounts (e) -- output counts beginning with e, and terminate with \n  */

putcounts (e)
struct evcount *e;
{
    while (e != NULL) {
	printf ("  %s:%ld", e->event, e->count);
	e = e->next;
	}
    putchar ('\n');
}
