/*  errors.c -- error processing  */

#include <signal.h>
#include <ctype.h>
#include "compiler.h"

#define YYSTYPE Nodeptr
#include "tokens.h"

static int errcmp ();



typedef struct error {	/* struct for remembering an error */
    int locn;		/* line number, file index, E_WARN & E_IMPORT flags */
    int seqn;		/* sequence number for ordering errs on one line */
    char *text;		/* basic error text, possibly with %s */
    char *param;	/* string parameter to insert */
} Error, *Errptr;

static Error elist[MAX_ERRORS];
static int nerrors = 0;

static int srcmax[MAX_SRC];	/* highest line referenced in src file */



/*  checkid (e1, kwd, e2) -- check for correct id after keyword  */

void
checkid (e1, kwd, e2)
Nodeptr e1;
char *kwd;
Nodeptr e2;
{
    char buf[25];

    ASSERT (e1->e_opr == O_ID);
    if (e2 == NULL)
	return;				/* optional id was omitted */
    ASSERT (e2->e_opr == O_ID);
    if (e1->e_name != e2->e_name) {
	sprintf (buf, "expected `%s %%s'", kwd);
	err (e2->e_locn, buf, e1->e_name);
    }
}



/*  yyerror(s) -- handle syntax error detected by Yacc parser  */

/*ARGSUSED*/
int
yyerror (s)
char *s;
{
    char buf[100];
    char *yytext;
    extern int yychar;				/* defined by yacc */
    extern Nodeptr yylval;			/* defined by yacc */
    extern int tkpend;				/* defined by gettok() */

    if (tkpend != 0)  {
	err (srclocn, "unexpected newline", NULLSTR);
	return 0;
    }
    yytext = yyref ();
    if (yytext[0] == '"')
	sprintf (buf, "parse error at literal \"%.20s\"", DATA(yylval->e_sptr));
    else if (yytext[0] == '\'')
	sprintf (buf, "parse error at literal '%c'", yylval->e_int);
    else if (yytext[0] == '\0')
	sprintf (buf, "parse error at EOF");
    else if (yytext[0] == '%')
	sprintf (buf, "parse error at token `%%%%'");
    else if (!isalpha (yytext[0]))
	sprintf (buf, "parse error at token `%s'", yytext);
    else if (yychar == TK_ID)
	sprintf (buf, "parse error at identifier `%.30s'", yytext);
    else
	sprintf (buf, "parse error at keyword `%s'", yytext);
    FATAL (buf);
    return 0;
}



/*  handler -- called on fatal signals  */

void
handler (sgnl)
int sgnl;
{
    char *s;

    switch (sgnl) {
#ifdef SIGBUS
	case SIGBUS:	s = "bus error";		break;
#endif
	case SIGSEGV:	s = "segmentation violation";	break;
	default:	s = "fatal signal";		break;
    }
    kboom (s, NULLSTR, "?", 0);
}



/*  assfail -- abort due to assertion failure -- called by ASSERT macro  */

void
assfail (f, l)
char *f;
int l;
{
    kboom ("assertion failure", NULLSTR, f, l);
}



/*  kboom -- malfunction abort -- called by "BOOM" macro  */

void
kboom (s1, s2, f, l)
char *s1, *s2, *f;
int l;
{
    static int nboom = 0;

    if (nboom++ > 0) {
	fprintf (stderr, "recursive malfunction -- giving up\n");
	exit (1);
    }

    fflush (stdout);
    errflush ();
    fprintf (stderr, "COMPILER MALFUNCTION (%s/%d)", f, l);
    if (s1 != NULL && *s1 != '\0')
	fprintf (stderr, ": %s", s1);
    if (s2 != NULL && *s2 != '\0')
	fprintf (stderr, ": %s", s2);
    fprintf (stderr, "\n");
    cflush ();
    exit (1);
}



/*  err(locn,text,param) -- record error message.
 *
 *  locn is line number, possibly also with import file index and/or flags.
 *  text is basic message text.
 *  param is string to insert at %s in text, or NULL if not needed.
 */

void
err (locn, text, param)
int locn;
char *text, *param;
{
    Errptr e;

    if (option_w && (locn & E_WARN))
	return;				/* ignore warnings if "sr -w" */

    e = & elist[nerrors++];
    e->locn = locn;
    e->seqn = nerrors;
    e->text = rsalloc (text);
    e->param = rsalloc (param);

    if (locn & E_WARN)  {
	nwarnings++;
    } else {
	nfatals++;
	tfatals++;
	abort_echo ();
    }

    if (nfatals >= MAX_FATALS || (nwarnings + nfatals) >= MAX_ERRORS) {
	errflush ();
	mexit ("too many errors.  aborting.");
    }

    if (dbflags['e'])
	errflush ();
}



/*  errflush() -- flush accumulated errors  */
 
void
errflush ()
{
    Errptr e;
    int i;

    qsort ((char *) elist, nerrors, sizeof (Error), errcmp);
    for (e = elist; e < elist + nerrors; e++) {
	i = (e->locn & E_FILE) >> E_FSHIFT;
	fprintf (stderr, "\"%s\", line %2d", srcname[i], e->locn & E_LINE);
	if (e->locn & E_EXTEND)
	    fprintf (stderr, " (extended by %s)", resname);
	else if (e->locn & E_IMPORT)
	    fprintf (stderr, " (imported by %s)", resname);
	fprintf (stderr, ": %s: ", (e->locn & E_WARN) ? "warning" : "fatal");
	fprintf (stderr, e->text, e->param);
	fprintf (stderr, "\n");
    }
    nerrors = 0;
}

static int
errcmp (p1, p2)
char *p1, *p2;
{
    Errptr e1, e2;
    int line1, line2;

    e1 = (Errptr) p1;
    e2 = (Errptr) p2;
    line1 = e1->locn & (E_FILE | E_LINE);
    line2 = e2->locn & (E_FILE | E_LINE);
    if (line1 != line2)
	return line1 - line2;
    else
	return e1->seqn - e2->seqn;
}



/*  traceback (e) -- generate traceback information for parse node e  */

void
traceback (e)
Nodeptr e;
{
    int f, n;

    if (e == NULL) {
	cprintf ("(char*)0");
	return;
    }
    f = (e->e_locn & E_FILE) >> E_FSHIFT;
    n = e->e_locn & E_LINE;
    cprintf ("SRC%d+%d", f, n);
    if (srcmax[f] == 0) {	
	/* first time from this source file; generate portable forward ref */
	cprintf ("%0#define SRC%d SRC%d_%s\n", f, f, curres->r_sym->s_name);
	cprintf ("%0extern char SRC%d[];\n\n", f);
    }
    if (srcmax[f] < n) 
	srcmax[f] = n;
}



/*  tracedump () -- dump traceback definitions  */

void
tracedump ()
{
    int f, n;

    cprintf ("\n\n");
    for (f = 0; f < MAX_SRC; f++)
	if (srcmax[f] != 0) {
	    n = strlen (srcname[f]) + 2;
	    if (n < srcmax[f])
		n = srcmax[f];
	    cprintf ("char SRC%d[%d]=\"\\002%s\";\n", f, n, srcname[f]);
	    srcmax[f] = 0;
	}
}
