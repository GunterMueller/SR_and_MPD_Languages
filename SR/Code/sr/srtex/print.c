/*   SR program TeX formatter
 *   Written by Mike Coffin, U. of Arizona, 1 November 1987
 */

#include <stdio.h>
#include "../config.h"
char *yyref ();

#define CSIZE (1/1.8)		/* character size */
#define BUFSIZE 8192		/* size of output buffer */
#define DEF_TAB TAB_WIDTH
#define DEF_ALIGN 3
#define DEF_COMMENT_FONT "it"    
#define DEF_KEYWORD_FONT "bf"
#define DEF_STRING_FONT  "tt"
#define DEF_ID_FONT      "rm"
    
static char buf[BUFSIZE];	/* output buffer */
static char *lp;		/* current insert pos in buffer */
				/* invariant: always points at \0 */
static int chcnt;		/* chars read so far on this line */
static int prcnt;		/* chars printed so far on this line */

/* options */
static int tab = DEF_TAB;
static int latex = 0;		/* output LaTeX ? */
static int align_spaces = DEF_ALIGN;	/* this many spaces cause tabbing */
static int print;		/* is this a solo print ? */
static char *comment_font = DEF_COMMENT_FONT;
static char *keyword_font = DEF_KEYWORD_FONT;
static char *string_font  = DEF_STRING_FONT;
static char *id_font      = DEF_ID_FONT;

static int res_break = 0;	/* break resource at end of this line? */
static int proc_break = 0;	/* same for proc break */


main (argc, argv)
int argc;
char *argv[];
{
    int c;
    extern char *optarg;	/* for use with getopt() */
    extern int optind;		/* ditto */
    
    while ((c = getopt (argc, argv, "t:pla:C:K:S:I:")) != EOF) {
	switch (c) {
	case 't':
	    tab = atoi (optarg);
	    if (tab <= 0)
		tab = DEF_TAB;
	    break;
	case 'p':
	    print = 1;
	    break;
	case 'l':
	    latex = 1;
	    break;
	case 'a':
	    align_spaces = atoi (optarg);
	    break;
	case 'C':
	    comment_font = optarg;
	    break;
	case 'K':
	    keyword_font = optarg;
	    break;
	case 'S':
	    string_font = optarg;
	    break;
	case 'I':
	    id_font = optarg;
	    break;
	case '?':
	default:
	    fprintf (stderr,
		"usage: srtex [-lp] [-ta n] [-CIKS font] file ...\n");
	    exit (1);
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
    if (latex) {
	if (print) {
	    printf ("\\documentstyle{article}\n");
	    printf ("\\begin{document}\n");
	    printf ("\\raggedbottom\n");
	}
	printf ("\\begin{flushleft}\n");
    } else {
	if (print) {
	    printf ("\\raggedbottom\n");
	}
    }
}


/* finish up */
postquel ()
{
    if (latex) {
	printf ("\\end{flushleft}\n");
	if (print)
	    printf ("\\end{document}\n");
    } else {
	if (print) 
	    printf ("\\bye\n");
    }
}

/* process one file */
one_file ()
{
    lp = buf; *lp = 0;
    startline ();
    yylex ();
    endline ();
    printf ("\\filbreak\n");
    printf ("\\eject\n");
}

startline ()
{
    chcnt = prcnt = 0;
    if (latex)
	printf ("$");
    else
	printf ("\\line{$");
}

endline ()
{
    if (lp - buf > BUFSIZE) {
	fprintf (stderr,"Srtex internal error: Output buffer size exceeded.\n");
	exit (1);
    }
    printf ("\\hbox{$%s$}$\\hfill", buf);
    if (latex)
	printf (" \\\\ \n");
    else
	printf ("}\n");
	    
    lp = buf;
    *lp = '\0';
    
    if (res_break) {
	printf ("\\filbreak\n");
	printf ("\\goodbreak\n");
	res_break = 0;
    }
    if (proc_break) {
	printf ("\\filbreak\n");
	proc_break = 0;
    }
    
}

blankline ()
{
    char *p;
    int cnt = 0;
    for (p = yyref(); *p; p++)
	if (*p == '\n')
	    cnt++;
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

/* append string to output buffer, but escape special TeX characters */
tex (s)
char *s;
{
    chcnt += strlen (s);
    for (; *s; s++) {
	switch (*s) {
	case '\\':
	    out ("{\\tt\\char92}");
	    break;
	case ' ':
	    out ("\\ ");
	    break;
	case '{':
	    out ("\\lbrace ");
	    break;
	case '}':
	    out ("\\rbrace ");
	    break;
	case '~':
	    out ("\\sim ");
	    break;
	case '*':
	    out ("*");
	    break;
	case '>':
	    out (">");
	    break;
	case '<':
	    out ("<");
	    break;
	case '\"':
	    out ("\\char34 ");
	    break;
	case '^':
	    out ("\\char94 ");
	    break;
	case '$':
	case '&':
	case '#':
	case '%':
	case '_':
	    *lp++ = '\\';
	    /* fall through */
	default:
	    *lp++ = *s;
	    break;
	}
    }
    *lp = '\0';
}

keyword ()
{
    out ("{\\");
    out (keyword_font);
    out (" ");
    tex (yyref());
    out ("}");
}

punct ()
{
    tex (yyref());
}

arrow ()
{
    chcnt += 2;
    out ("\\rightarrow ");
}

box ()
{
    chcnt += 2;
    out ("\\hbox to 1.25em{\\kern.2em\\lower.5ex\\hbox{\\vrule\\kern -.4pt\\vbox{\\hrule\\kern 2.3ex\\hbox to 0.45em{}\\hrule}\\kern -.4pt\\vrule}\\hfill}");
}


gteq ()
{
    chcnt += 2;
    out ("\\geq ");
}

leeq ()
{
    chcnt += 2;
    out ("\\leq ");
}

neq ()
{
    chcnt += 2;
    out ("\\not= ");
}

colon ()
{
    chcnt += 1;
    out ("\\,\\colon ");
}

parallel ()
{
    chcnt += 2;
    out ("\\hbox to 1.25em{/\\kern -.1em/\\hfill}");
}

number ()
{
    tex (yyref());
}

name ()
{
    out ("{\\");
    out (id_font);
    out (" ");
    tex (yyref());
    out ("}");
}


string ()
{
    out ("\\hbox{\\");
    out (string_font);
    out (" ");
    tex (yyref());
    out ("}");
}

comment ()
{
    out ("\\hbox{\\");
    out (comment_font);
    out (" ");
    tex (yyref());
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
    if (has_tab || (cols - chcnt) >= align_spaces) {
	printf ("\\hbox to %1fem{$%s$\\hfill}",(double)(cols-prcnt)*CSIZE,buf);
	lp = buf;
	*lp = 0;
	chcnt = prcnt = cols;
    } else {
	tex (yyref());
    }
}

nest_out (level)
int level;
{
    if (level == 0)
	res_break = 1;
    if (level == 1)
	proc_break = 1;
}

/* maybe this should print a warning...*/
bad_char ()
{
    tex (yyref());
}
