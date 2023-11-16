/*  output.c -- output routines for code generation
 *
 *  Output accumulates in up to ten "streams" of internal buffers.
 *  These buffers are selected by %3 (e.g.) in a cprintf call.
 *  When cflush() is called, the streams are all written out, in order.
 */

#include "compiler.h"
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

#define NSTREAMS 10		/* number of output streams */
#define DEFSTREAM 9		/* default stream number */

#define INDENT 3		/* number of spaces to indent each level */

#define NEWLINE_TIME 40		/* after this column insert \n at first %, */



typedef struct obuf {		/* output buffer structure */
    int cnt;			/* chars left in this buffer */
    int linefull;		/* value of cnt when line gets full */
    char *sto;			/* store pointer */
    struct obuf *next;		/* pointer to following buffer */
    struct obuf *divto;		/* pointer to diversion buffer */
    struct obuf *backto;	/* pointer to buff that diverted to this one */
    short parens;		/* parenthesis count */
    char pad[1];		/* padding for better alignment */
    char buff[OBUFSIZE+1];	/* output buffer (first char not written) */
} Obuf;



static void putconst	PARAMS ((int n));
static int  full	PARAMS ((int c));
static Obuf *newbuf	PARAMS ((NOARGS));


static char *outname;		/* output file name */
static FILE *outfile;		/* output file pointer */

static int outstream = -1;	/* current stream number */
static Obuf *outbuf;		/* pointer to current output buffer */

static int  ndivert[NSTREAMS];	/* number of diversions per stream */
static Obuf *curbuf[NSTREAMS];	/* current buffer for each stream */
static Obuf *firstbuf[NSTREAMS];/* first buffer for each stream */
static Obuf *stdbuf[NSTREAMS];	/* last buffer used before diversion */



/*  put one character c in the current output buffer  */
#define PUT(c) ((--outbuf->cnt >= 0) ? (*outbuf->sto++=(c)) : full(c))

/*  check the previously output character  */
#define LASTPUT() (outbuf->sto[-1])

/*  buffers may survive, empty, across different resources */
/*  -- so make sure nobody in here uses ralloc() */
#define ralloc do_not_use_ralloc_in_here
#undef NEW



/*  mkinter () -- make Interfaces directory  */

void
mkinter ()
{
    static int beenhere;
    extern int errno;			/* not defined in all errno.h files */

    if (!beenhere++)			/* once only */
	if (mkdir (ifdir, 0777) != 0 && errno != EEXIST)
	    pexit (ifdir);
}



/*  copen (fname) -- open output file  */

void
copen (fname)
char *fname;
{
    ASSERT (!outfile);
    outname = fname;
    outfile = mustopen (fname, "w");
    setstream (DEFSTREAM);
}



/*  cdivert (n) -- divert stream n.
 *
 *  Create a new buffer set for stream n; when later undiverted, link it at the
 *  end of the main stream for buffer n.  Diversions can nest in stack fashion.
 */

void
cdivert (n)
int n;
{
    Obuf *b;

    ASSERT (n >= 0 && n < NSTREAMS);
    if (!curbuf[n])
	curbuf[n] = firstbuf[n] = newbuf ();
    if (ndivert[n]++ == 0)		/* if first diversion */
	stdbuf[n] = curbuf[n];		/* save base of chain */
    b = curbuf[n]->divto = newbuf ();	/* create new buffer, link in */
    b->backto = curbuf[n];		/* link back to previous level */
    curbuf[n] = b;			/* put in place */
}



/*  undivert (n) -- undivert stream n  */

void
undivert (n)
int n;
{
    ASSERT (n >= 0 && n < NSTREAMS);
    ASSERT (ndivert[n] > 0);
    ASSERT (stdbuf[n]->next == NULL);

    stdbuf[n]->next = curbuf[n]->backto->divto;	/* link head of diverted chain*/
    stdbuf[n] = curbuf[n];			/* move to tail */
    if (--ndivert[n] == 0)
	curbuf[n] = stdbuf[n];
    else
	curbuf[n] = curbuf[n]->backto;
}



/*  cclose () -- flush and close output file  */

void
cclose ()
{
    cflush ();
    if (fclose (outfile) != 0)
	pexit (outname);
    outname = NULL;
    outfile = NULL;
}



/*  cflush () -- write all buffers, then free them  */

void
cflush ()
{
    Obuf *p, *q;
    int i, n, s;

    if (outfile == NULL)
	return;
    for (i = 0; i < NSTREAMS; i++) {
	while (ndivert[i] > 0) {	/* should only happen flushing a kboom*/
	    setstream (i);
	    cprintf ("\n********** diversion %d closed **********\n", i);
	    undivert (i);
	}
	p = firstbuf[i];
	if (dbflags['b'] && p != NULL)
	    fprintf (outfile, "\n/*---------- %%%d */\n\n", i);
	while (p)  {			/* write buffers in sequence */
	    q = p->next;
	    n = p->sto - (p->buff + 1);
	    ASSERT (n >= 0 && n <= OBUFSIZE);
	    if (n > 0) {
		s = fwrite (p->buff + 1, 1, n, outfile);
		if (s <= 0)
		    pexit (outname);
	    }
	    free ((char *) p);		/* free after writing */
	    p = q;
	}
	firstbuf[i] = curbuf[i] = 0;
    }
    setstream (outstream);		/* restart current stream */
}



/*  cprintf (template,p1,p2,...) -- accumulate formatted output
 *
 *  The template is similar in flavor to printf(3S).
 *  "argtype" gives the expected type of the corresponding "pn" argument.
 *  "result" gives the type of the generated C expression, if applicable.
 *
 *   field   argtype	result	meaning
 *   -----   -------	------	---------------------------------------------
 *	%0			switch to stream 0 (similarly for %1...%9)
 *	%a   Nodeptr	char*	address of addressable expression
 *	%c   int		char argument
 *	%C   Classptr	Ptr	class structure pointer
 *	%d   int	int	integer argument
 *	%e   Nodeptr	any	value of expression 
 *	%E   Nodeptr	any	value of expression; save to gen again later
 *	%f   Nodeptr	any	value of expression, freeing allocated temps
 *	%g   Sigptr		C type implementing the signature
 *	%l   Nodeptr	int	value of the lower bound of an array
 *	%L   int		label reference
 *	%n   Symptr		full name of a symbol for access from C
 *	%N   Symptr		partial form of name  (sans "rv->" or whatever)
 *	%o   int		print integer argument as three octal digits
 *	%O   Opptr		generate opcap decl with array header if any
 *	%P   Symptr		typedef name for paramlist of op
 *	%s   char *		string argument
 *	%S   Nodeptr    String*	pass string argument; convert if char array
 *	%t   Nodeptr	int	traceback information (e->e_locn)
 *	%u   Nodeptr	int	value of the upper bound of an array
 *	%%			percent sign
 *	%,			comma followed by space (or newline if too long)
 *	%other			error
 *
 *   Some special cases simplify code generation:
 *   (1)  An initial ',' following '(' is suppressed for easy list generation.
 *   (2)  When a negative "%d" follows '=' or '-', a space is inserted between.
 *   (3)  Spaces are inserted after \n according to the current nesting level.
 *   (4)  A space is inserted before any of +-* if preceded immediately by =.
 */


void
cprintf (char *fmt, ...)
{
    va_list ap;
    char *s;
    char c;
    int n, oldstream;
    Nodeptr e, f;
    Classptr cl;

    oldstream = outstream;		/* save stream number in case changed */

    va_start (ap, fmt);

    if (*fmt == ',' && LASTPUT () == '(')
	fmt++;				/* skip initial comma inside parens */

    while (c = *fmt++)  {
	switch (c) {
	    case '+':
	    case '-':
	    case '*':
		if (LASTPUT () == '=')	
		    PUT (' ');
		PUT (c);
		break;
	    case '(': 
	    case '{':
		outbuf->parens++;
		PUT (c);
		break;
	    case ')': 
	    case '}':
		outbuf->parens--;
		PUT (c);
		break;
	    case '\n':
		PUT ('\n');
		outbuf->linefull = outbuf->cnt - NEWLINE_TIME;
		for (n = INDENT * outbuf->parens; n > 0; n--)
		    PUT (' ');
		break;
	    case '%':
		switch (c = *fmt++)  {
		    case '0':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
		    case '8':
		    case '9':
			setstream (c - '0');
			break;
		    case 'a':
			gaddr (va_arg (ap, Nodeptr));
			break;
		    case 'c':
			PUT (va_arg (ap, int));
			break;
		    case 'C':
			cl = va_arg (ap, Classptr);
			if (cl->c_seg == SG_PROC)	/* if a local op */
			    cprintf ("c%d", cl->c_num);
			else if (cl->c_num > 0)		/* if not a global op */
			    cprintf ("rv->c%d", cl->c_num);
			else
			    cprintf ("cl_%s",		/* use global class */
				cl->c_first->o_sym->s_imp->s_gname);
			break;
		    case 'd':
			putconst (va_arg (ap, int));
			break;
		    case 'e':
			gexpr (va_arg (ap, Nodeptr));
			break;
		    case 'E':
			once (va_arg (ap, Nodeptr), 0);
			break;
		    case 'f':
			gfexpr (va_arg (ap, Nodeptr));
			break;
		    case 'g':
			cprintf ("%s", csig (va_arg (ap, Sigptr)));
			break;
		    case 'l':
			e = va_arg (ap, Nodeptr);
			if ((e->e_opr != O_SYM || e->e_sym->s_kind != K_VAR || 
					e->e_sym->s_var->v_vty != V_REFNCE)
			        && e->e_sig->g_lb != NULL
				&& e->e_sig->g_lb->e_opr != O_ASTER)
			    cprintf ("%e", e->e_sig->g_lb);
			else {
			    f = e;
			    while (f != NULL && f->e_opr == O_INDEX)
				f = LNODE (f);
			    ASSERT (f);
			    cprintf ("LB(%e,%d)", f, ndim (e->e_sig) - 1);
			}
			break;
		    case 'u':
			e = va_arg (ap, Nodeptr);
			if ((e->e_opr != O_SYM || e->e_sym->s_kind != K_VAR || 
					e->e_sym->s_var->v_vty != V_REFNCE)
				&& e->e_sig->g_ub != NULL
				&& e->e_sig->g_ub->e_opr != O_ASTER)
			    cprintf ("%e", e->e_sig->g_ub);
			else {
			    Nodeptr f = e;
			    while (f != NULL && f->e_opr == O_INDEX)
				f = LNODE (f);
			    ASSERT (f);
			    cprintf ("UB(%e,%d)", f, ndim (e->e_sig) - 1);
			}
			break;
		    case 'L':
			cprintf ("L%d", va_arg (ap, int));
			break;
		    case 'n':
			cprintf ("%s", sname (va_arg (ap, Symptr), 1));
			break;
		    case 'N':
			cprintf ("%s", sname (va_arg (ap, Symptr), 0));
			break;
		    case 'o':
			goctal (va_arg (ap, int));
			break;
		    case 'O':
			opdecl (va_arg (ap, Opptr));
			break;
		    case 'P':
			cprintf (pbdef (va_arg (ap, Symptr)));
			break;
		    case 's':
			for (s = va_arg (ap, char *); c = *s++; )
			    PUT (c);
			break;
		    case 'S':
			e = va_arg (ap, Nodeptr);
			if (e->e_sig->g_type == T_STRING)
			    cprintf ("%e", e);
			else {
			    ASSERT (e->e_sig->g_type == T_ARRAY);
			    ASSERT (e->e_sig->g_usig->g_type == T_CHAR);
			    cprintf ("((String*)(%s=sr_astring(%e)))",
				alctrans (), e);
			}
			break;
		    case 't':
			traceback (va_arg (ap, Nodeptr));
			break;
		    case '%':
			PUT ('%');
			break;
		    case ',':
			PUT (',');
			if (outbuf->cnt < outbuf->linefull)
			    cprintf ("\n");
			else
			    PUT (' ');
			break;
		    default:  
			BOOM ("bad cprintf() format", fmt - 2);
		}
		break;
	    default:
		PUT (c);
		break;
	    }
	}
    va_end (ap);

    if (oldstream != outstream)
	setstream (oldstream);		/* restore previous output stream */
}



/*  setstream (n) -- set stream n for further output  */

void
setstream (n)
int n;
{
    ASSERT (n >= 0 && n < NSTREAMS);
    if (!curbuf[n])
	curbuf[n] = firstbuf[n] = newbuf ();
    outbuf = curbuf[n];
    outstream = n;
}



/*  wescape(fp,s,n,d) -- write n chars of s to fp, delim by d, with escapes  */

void
wescape (fp, s, n, d)
FILE *fp;
char *s;
int n;
int d;	/* char */
{
    int c;

    putc (d, fp);
    while (n--) {
	c = *s++ & 0xFF;
	if (!isascii (c))
	    fprintf (fp, "\\%03o", c);
	else if (c == d || c == '\\')
	    fprintf (fp, "\\%c", c);
	else if (isprint (c))
	    putc (c, fp);
	else if (isspace (c))
	    fprintf (fp, "\\%c", "tnvfr"[c-9]);
	else
	    fprintf (fp, "\\%03o", c);
    }
    putc (d, fp);
}



/*************************  INTERNAL ROUTINES  *************************/



/*  putconst (n) -- output a constant, special casing n<0 following '[=-]'  */

static void
putconst (n)
int n;
{
    char buf[20], c, *s;

    if (n < 0 && (LASTPUT () == '=' || LASTPUT () == '-'))
	PUT (' ');			/* avoid false "=-" or "--" */
    sprintf (buf, "%d", n);		/* format the integer */
    for (s = buf; c = *s++; )		/* write the characters */
	PUT (c);
}



/*  full (c) -- add new output buffer (current is full) and put character c  */

static int
full (c)
int c;
{
    Obuf *b;

    b = newbuf ();			/* allocate & initialize new buffer */
    b->parens = outbuf->parens;		/* set paren count */
    b->backto = outbuf->backto;		/* preserve backpointer for diversion */
    b->linefull = b->cnt + outbuf->linefull;	/* set newline threshold */
    b->buff[0] = LASTPUT ();		/* set lookback char */
    outbuf->next = b;			/* link in buffer */
    outbuf = curbuf[outstream] = b;	/* update buffer pointers */
    PUT (c);				/* write the stalled character */
    return c;
}



/*  newbuf () -- create a new output buffer  */

static Obuf *
newbuf ()
{
    Obuf *b;

    b = (Obuf *) alloc (sizeof (Obuf));	/* allocate buffer */
    b->cnt = OBUFSIZE;			/* init buffer capacity */
    b->linefull = b->cnt-NEWLINE_TIME;	/* set threshold for inserting newline*/
    b->buff[0] = ' ';			/* init lookback character */
    b->sto = b->buff + 1;		/* init buffer pointer */
    b->next = 0;			/* no next buffer */
    b->divto = 0;			/* no diversion info */
    b->backto = 0;
    b->parens = 0;			/* no parens that we know of */
    return b;
}
