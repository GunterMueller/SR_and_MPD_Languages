/*  input.c -- input processing and spec file echoing  */

#include "compiler.h"

#define YYSTYPE Nodeptr
#include "tokens.h"

static void echo	PARAMS ((NOARGS));

static FILE *specfile;			/* file pointer */
static char specname[MAX_PATH];		/* file name */

int tkpend;				/* pending token from last call */
					/* if found NZ in yyerror, */
					/* indicates unexpected newline */

static int tkprev;			/* previously returned token */
static int tkolder;			/* the token before that one */



/*
 *  btable[] -- line boundary table, indexed by token code.
 *
 *  Most of the data comes from comments in grammar.y.
 *  btable[tk] & BEG is nonzero if token tk can begin a line (follow ';') in SR.
 *  btable[tk] & END is nonzero if token tk can end a line (precede ';') in SR.
 */

static char btable [TK_BOGUS + 1];	/* assumes TK_BOGUS is max value */
#define BEG 1
#define END 2



/*  gettok() -- get a token, handle newlines, and echo to spec file.
 *
 *  This routine is interposed between yacc and lex via a #define of yylex.
 *  Its two purposes are to decide which newlines are significant and to
 *  copy the tokens of a spec portion to a .spec file.
 *
 *  A TK_NEWLINE token from lex either discarded or returned as TK_SEPARATOR
 *  depending on the surrounding context.  If the previous token can validly
 *  end a line, and the next token can begin one, then the newline is a
 *  separator.  This technique comes from the Y and Icon languages.
 *
 *  Echoing is controlled from outside by calls to start_echo and stop_echo.
 */

#define ECHO()		{if (specfile != NULL) echo ();}
#define RETURN(token,image) { \
    if (dbflags['k']) { \
	if (tkprev == TK_SEPARATOR) \
	    printf ("%2d", srclocn & E_LINE); \
	printf (" %s", image); \
    } \
    if (yylval) yylval->e_locn = srclocn; \
    tkolder = tkprev; \
    return tkprev = token; \
}

int
gettok ()
{
    int tk;				/* new token */
    extern Nodeptr yylval;		/* defined by yacc */

    /* process any newlines that are pending from the previous call */
    updtloc ();

    /* if there is a token pending from the last call, return it */
    if (tkpend != 0) {
	tk = tkpend;
	tkpend = 0;
	ECHO ();
	RETURN (tk, yyref ());
    }

    /* get the next token; may increment the global "newlines" */
    yylval = 0;
    tk = yylex ();

    /* if EOF, and previous was not a separator, insert newline */
    if (tk == 0 && tkprev != TK_SEPARATOR)
	tk = TK_NEWLINE;

    /* if token is not a newline, just return it */
    if (tk != TK_NEWLINE) {
	updtloc ();
	ECHO ();
	RETURN (tk, yyref ());
    }

    /* special check to keep from reading beyond end of global or resource */
    if (tkprev == TK_END || (tkolder == TK_END && tkprev == TK_ID)) {
	RETURN (TK_SEPARATOR, ";;\n");	/* `;;' trace flags special case */
    }

    /* read past newline tokens to find something significant */
    yylval = 0;
    while (tk == TK_NEWLINE)
	tk = yylex ();		/* may further increment "newlines" global */

    /* if the newline token was significant, return a separator */
    if ((btable[tkprev] & END) && (btable[tk] & BEG)) {
	tkpend = tk;			/* save current token for next call */
	RETURN (TK_SEPARATOR, ";\n");	/* \n distinguishes from explicit ; */
    }

    /* newlines were not significant; account for them and return new token */
    updtloc ();
    ECHO ();
    RETURN (tk, yyref ());
}

#undef ECHO
#undef RETURN



/*  updtloc () -- update source code location and zero newline counter  */

void
updtloc ()
{
    srclocn += newlines;
    if (specfile != NULL)
	while (--newlines >= 0)	
	    putc ('\n', specfile);
    newlines = 0;
}



/*  start_echo (e) -- start echoing tokens on Interfaces/resname.spec  */

void
start_echo (e)
Nodeptr e;
{
    char *s;

    ASSERT (specfile == NULL);
    if (!allow_echo)
	return;

    srclocn += newlines;		/* get locn counter in sync */
    newlines = 0;

    switch (e->e_r->e_opr) {
	case O_GLOBAL:    s = "global";    break;
	case O_RESOURCE:  s = "resource";  break;
	default:          BOOM ("unknown component type",oprtos(e->e_r->e_opr));
    }

    sprintf (specname, "%s/%s%s", ifdir, e->e_l->e_name, SPEC_SUF);
    mkinter ();
    specfile = mustopen (specname, "w");
    fprintf (specfile, "# %% ");
    if (srcname[0][0] != '/')
	fprintf (specfile, "%s/", cwd);
    fprintf (specfile, "%s %d+\n\n", srcname[0], srclocn & E_LINE);
    fprintf (specfile, "%s %s", s, e->e_l->e_name);
}



/*  echo () - echo current token to specfile.  */

static void
echo ()
{
    String *s;
    char c;
    char *yytext;
    extern Nodeptr yylval;		/* defined by yacc */

    ASSERT (specfile != NULL);
    putc (' ', specfile);
    yytext = yyref ();
    switch (yytext[0]) {
	case '\'':
	    c = yylval->e_int;
	    wescape (specfile, &c, 1, '\'');
	    break;
	case '\"':
	    s = yylval->e_sptr;
	    wescape (specfile, DATA (s), s->length, '"');
	    break;
	default:
	    fputs (yytext, specfile);
	    break;
    }
}



/*  stop_echo (e) -- write parameter count and close specfile
 *
 *  e is the parse tree for the parameter list, if any.
 *  Insert the number of parameters into the header line and close the file.
 *
 *  We may have already written one too many tokens out, due to Yacc lookahead.
 *  That is no problem due to the way the spec file is read.
 *
 *  But for cases where we haven't, we add enough legal termination tokens --
 *  without a newline -- to ensure that the final newline is not consumed
 *  by lookahead when the spec file is read. This keeps the line numbers
 *  of the main file from being thrown off.
 */

void
stop_echo (e)
Nodeptr e;
{
    int n;

    if (specfile == NULL)
	return;
    fprintf (specfile, " ; end ;\n");	/* terminate legally before newline */
    fflush (specfile);

    for (n = 0; e != NULL; e = e->e_r)
	n++;
    rewind (specfile);
    fprintf (specfile, "#%2d", n);
    fflush (specfile);

    fclose (specfile);
    specfile = NULL;
}



/*  abort_echo () -- close and remove specfile, if open  */

void
abort_echo ()
{
    if (specfile == NULL)
	return;
    fclose (specfile);
    specfile = NULL;
    unlink (specname);
}



/*  setinput (fp) -- set input source to file fp, returning handle for reset */

void *
setinput (fp)
FILE *fp;
{
    FILE *old;
    extern FILE *yyin;

    /* on first call, initialize table of flags via macro calls in tkflags.h */
    if (btable[0] == 0) {
	btable[0] = BEG;	 /* EOF can be preceded by a ';' */
#define TKFLAGS(tk,f) btable[tk] = f;
#include "tkflags.h"
    }

    /* clear lookahead status */
    tkpend = 0;
    tkprev = tkolder = TK_SEPARATOR;

    /* now, set the input file and return handle for restoring */
    newlines = 0;
    return lexfrom (fp);
}



/*  resetinput (handle) -- reset to earlier input file  */

void
resetinput (handle)
void *handle;
{
    newlines = 0;
    lexrevert (handle);
}
