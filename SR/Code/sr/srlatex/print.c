/*  srlatex -- SR program grinder
 *
 *  Written by Mike Coffin, U. of Arizona, 1 November 1987
 *  Seriously hacked by David Mosberger, UofA, 24 September 1992
 */

#include <stdio.h>
#include "../gen.h"
#include "srlatex.h"

char *yyref ();

#define BUFSIZE 8192		/* size of output buffer */
#define DEF_TAB 8
#define DEF_ALIGN 3

static char *pgm_name;
static char buf[BUFSIZE];	/* output buffer */
static char *lp;		/* current insert pos in buffer */
				/* invariant: always points at \0 */
static int chcnt;		/* chars read so far on this line */
static int prcnt;		/* chars printed so far on this line */



/* options */

Language lang;
int literal_escape = 0;		/* recognize escape sequence? */
static int tab = DEF_TAB;
static int align_spaces = DEF_ALIGN;	/* this many spaces cause tabbing */
static int print;		/* is this a solo print ? */

static int res_break = 0;	/* resource break at end of this line? */
static int proc_break = 0;	/* same for proc break */
static int math_mode = 0;	/* are we in math mode? */


static void
usage ()
{
    fprintf (stderr, "usage: %s [-ep] [-l lang] [-a n] [-t n] file ...\n",
	pgm_name);
    exit (1);
}



int
main (argc, argv)
int argc;
char *argv[];
{
    int c;
    char *p;
    extern char *optarg;	/* for use with getopt() */
    extern int optind;		/* ditto */

    pgm_name = argv[0];
    p = strrchr (pgm_name, '/');
    if (p)
	++p;
    else
	p = pgm_name;
    if (*p == 's')
	lang = SR;		/* name starts with ``sr'' */
    else if (*p == 'f')
	lang = FTSR;		/* name starts with ``ftsr'' */
    else
	lang = PL;		/* assume name starts with ``pl'' */

    while ((c = getopt (argc, argv, "el:t:pa:")) != EOF) {
	switch (c) {
	    case 'e':
		literal_escape ^= 1;
		break;
	    case 'l':
		switch (optarg[0]) {
		    case 's':
			lang = SR;
			break;
		    case 'f':
			lang = FTSR;
			break;
		    case 'p':
			lang = PL;
			break;
		    default:
			fprintf (stderr,
			    "%s: language should be one of sr, ftsr, or pl\n",
			    pgm_name);
			usage ();
		}
		break;
	    case 't':
		tab = atoi (optarg);
		if (tab <= 0)
		    tab = DEF_TAB;
		break;
	    case 'p':
		print ^= 1;
		break;
	    case 'a':
		align_spaces = atoi (optarg);
		break;
	    case '?':
	    default:
		usage ();
	}
    }
    
    prequel ();
    
    if (optind == argc) {	/* process stdin */
	one_file ();
    } else {			/* remaining command-line args */
	for (; optind < argc; optind++) {
	    FILE *f;
	    if ((f = freopen (argv[optind], "r", stdin)) == NULL) {
		perror (argv[optind]);
		exit (1);
	    }
	    one_file ();
	    fclose (f);
	}
    }
    postquel ();
    exit (0);
    /*NOTREACHED*/
}

/* output any initial stuff that's needed */
prequel ()
{
    if (print) {
	printf ("\\documentclass{article}\n");
	printf ("\\usepackage{srlatex}\n");
	printf ("\\begin{document}\n");
	printf ("\\raggedbottom\n");
    }
    printf ("\\begin{flushleft}\n");
}


/* finish up */
postquel ()
{
    printf ("\\end{flushleft}\n");
    if (print) {
	printf ("\\vfill\n");
	printf ("\\end{document}\n");
    }
}

/* process one file */
one_file ()
{
    lp = buf; *lp = 0;
    printf ("\\begin{srTeXprogram}\n");
    startline ();
    yylex ();
    endline ();
    printf ("\\end{srTeXprogram}\n");
}

tmode ()
{
    if (math_mode) {
	out ("$");
	math_mode = 0;
    }
}

mmode ()
{
    if (!math_mode) {
	out ("$");
	math_mode = 1;
    }
}

startline ()
{
    chcnt = prcnt = 0;
    math_mode = 0;
}

endline ()
{
    if (lp - buf > BUFSIZE) {
	fprintf (stderr, "%s: malfunction: output buffer size exceeded.\n",
		pgm_name);
	exit (1);
    }
    tmode ();
    if (buf[0])
	printf ("%s\\\\\n", buf);
    else
	printf ("\\hspace{0pt}\\\\\n");
	    
    lp = buf;
    *lp = '\0';
    
    if (res_break) {
	printf ("\\goodbreak\n");
	res_break = 0;
    }
    if (proc_break) {
	proc_break = 0;
    }
    
}

blankline ()
{
    char *p;
    int cnt = 0;

    for (p = yyref(); *p; p++) {
	if (*p == '\n') {
	    ++cnt;
	}
    }

    if (cnt > 1)
	printf ("\\vskip%d\\baselineskip\n", cnt - 1);
}

newpage ()
{
    printf ("\\eject");
}

/* append string to output buffer */
out (s)
char *s;
{
    while (*s)
	*lp++ = *s++;
    *lp = '\0';
}

/* append string to output buffer, but escape any special characters  */
tex (s)
char *s;
{
    char str[32];
    int blank_cnt;

    chcnt += strlen (s);
    for (; *s; s++) {
	switch (*s) {
	    case ' ':
		for (blank_cnt = 1; *(++s) == ' '; ++blank_cnt)
		    ;				/* skip */
		--s;
		sprintf (str, "\\srTeXsp{%d}", blank_cnt);
		out (str);
		break;
	    case '\\':
		mmode ();
		out ("\\backslash ");
		break;
	    case '{':
		mmode ();
		out ("\\lbrace ");
		break;
	    case '}':
		mmode ();
		out ("\\rbrace ");
		break;
	    case '~':
		mmode ();
		out ("\\sim ");
		break;
	    case '@':
		mmode ();
		out ("\\char`\\@ ");
		break;
	    case '^':
		mmode ();
		out ("\\char`\\^ ");
		break;
	    case '*': case '>': case '<': case '=': case '(': case ')':
	    case ',': case ';': case '+': case '-': case '/': case ':':
	    case '[': case ']': case '!': case '|': case '?':
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		mmode ();
		*lp++ = *s;
		break;
	    case '$': case '&': case '#':
	    case '%': case '_':
		tmode ();
		*lp++ = '\\';
		/* fall through */
	    default:
		tmode ();
		*lp++ = *s;
		break;
	}
    }
    *lp = '\0';
}

keyword ()
{
    tmode ();
    out ("\\srTeXkw{");
    tex (yyref());
    tmode ();
    out ("}");
}

punct ()
{
    tex (yyref());
}

arrow ()
{
    chcnt += 2;
    mmode ();
    out ("\\rightarrow ");
}

box ()
{
    chcnt += 2;
    mmode ();
    out ("\\srTeXbx ");
}


gteq ()
{
    chcnt += 2;
    mmode ();
    out ("\\geq ");
}

leeq ()
{
    chcnt += 2;
    mmode ();
    out ("\\leq ");
}

neq ()
{
    chcnt += 2;
    mmode ();
    out ("\\neq ");
}

colon ()
{
    chcnt += 1;
    tmode ();
    out ("\\,:\\,");
}

parallel ()
{
    chcnt += 2;
    mmode ();
    out ("\\srTeXpl ");
}

number ()
{
    mmode ();
    tex (yyref());
}

name ()
{
    tmode ();
    out ("\\srTeXid{");
    tex (yyref());
    tmode ();
    out ("}");
}


string ()
{
    tmode ();
    out ("\\srTeXst{");
    tex (yyref());
    tmode ();
    out ("}");
}

literal ()
{
    tmode ();
    out (yyref());
}

comment ()
{
    tmode ();
    out ("\\srTeXcm{");
    tex (yyref());
    tmode ();
    out ("}");
}

whitespace ()
{
    char *p;
    int has_tab = 0,
	cols = chcnt;
    
    for (p = yyref(); *p; p++) {
	switch (*p) {
	    case '\t':
		has_tab++;
		cols = (cols / tab + 1) * tab - 1;
	    case ' ':
		cols++;
		break;
	    default:
		cols++;
		break;
	}
    }
    tmode ();
    if (has_tab || (cols - chcnt) >= align_spaces) {
	if (strlen (buf))
	    printf ("\\srTeXmb{%d}{%s}", cols - prcnt, buf);
	else
	    printf ("\\srTeXsp{%d}", cols - prcnt);
	lp = buf; *lp = 0;
	chcnt = prcnt = cols;
    } else
	tex (yyref());
}

nest_out (level)
int level;
{
    if (level == 0)
	res_break = 1;
    if (level == 1)
	proc_break = 1;
}

bad_char ()
{
    fprintf (stderr, "%s: unknown character `%s'\n", pgm_name, yyref());
    tex (yyref());
}
