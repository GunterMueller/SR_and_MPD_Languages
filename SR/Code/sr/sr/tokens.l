/*  lex spec for SR  */


/*  configuration options  */

%e 1200
%p 3600
%n 500
%k 125
%a 1200
%o 1200		/* HP-UX needs large number */


/*  definitions to aid in specifying real constants  */

D	[0-9]
E	([eE][+-]?{D}+)



%{



#ifdef __CLCC__
#pragma Warning_level(0)
#endif


#include "compiler.h"
#include <ctype.h>

#define YYSTYPE Nodeptr
#include "tokens.h"

/*  hex to integer mapping (use low 5 bits of ascii char as subscript)  */
static unsigned xci[] = 
    {0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9};



static int doint	PARAMS ((int base));
static int literal	PARAMS ((int tk, Operator o, Sigptr g, int value));
static int doid		PARAMS ((NOARGS));
static int doreal	PARAMS ((NOARGS));
static int chrlit	PARAMS ((NOARGS));
static int strlit	PARAMS ((NOARGS));
static int escape	PARAMS ((NOARGS));
static void comment	PARAMS ((NOARGS));
static void toolong	PARAMS ((NOARGS));



/********************************  NOTE  *******************************/

/*	when adding or changing keywords, also update
	../srgrind/srgrind.sh and ../srtex/scan.l	*/

/***********************************************************************/

%}

%%

P		return TK_P;
V		return TK_V;
af		return TK_AF;
and		return TK_AND;
any		return TK_ANY;
begin		return TK_BEGIN;
body		return TK_BODY;
bool		return TK_BOOL;
by		return TK_BY;
call		return TK_CALL;
cap		return TK_CAP;
char		return TK_CHAR;
co		return TK_CO;
const		return TK_CONST;
create		return TK_CREATE;
destroy		return TK_DESTROY;
do		return TK_DO;
downto		return TK_DOWNTO;
else		return TK_ELSE;
end		return TK_END;
enum		return TK_ENUM;
exit		return TK_EXIT;
extend		return TK_EXTEND;
external	return TK_EXTERNAL;
fa		return TK_FA;
false		return literal (TK_BLIT, O_BLIT, bool_sig, FALSE);
fi		return TK_FI;
file		return TK_FILE;
final		return TK_FINAL;
forward		return TK_FORWARD;
global		return TK_GLOBAL;
high		return TK_HIGH;
if		return TK_IF;
import		return TK_IMPORT;
in		return TK_IN;
initial		return TK_INITIAL;
int		return TK_INT;
low		return TK_LOW;
mod		return TK_MOD;
new		return TK_NEW;
next		return TK_NEXT;
ni		return TK_NI;
noop		return literal (TK_NLIT, O_NLIT, nlit_sig, NOOP_SEQN);
not		return TK_NOT;
null		return literal (TK_NLIT, O_NLIT, nlit_sig, NULL_SEQN);
oc		return TK_OC;
od		return TK_OD;
on		return TK_ON;
op		return TK_OP;
optype		return TK_OPTYPE;
or		return TK_OR;
proc		return TK_PROC;
procedure	return TK_PROCEDURE;
process		return TK_PROCESS;
ptr		return TK_PTR;
real		return TK_REAL;
rec		return TK_REC;
receive		return TK_RECEIVE;
ref		return TK_REF;
reply		return TK_REPLY;
res		return TK_RES;
resource	return TK_RESOURCE;
return		return TK_RETURN;
returns		return TK_RETURNS;
sem		return TK_SEM;
send		return TK_SEND;
separate	return TK_SEPARATE;
skip		return TK_SKIP;
st		return TK_SUCHTHAT;
stderr		return literal (TK_FLIT, O_FLIT, file_sig, STDERR);
stdin		return literal (TK_FLIT, O_FLIT, file_sig, STDIN);
stdout		return literal (TK_FLIT, O_FLIT, file_sig, STDOUT);
stop		return TK_STOP;	
string		return TK_STRING;	
to		return TK_TO;
true		return literal (TK_BLIT, O_BLIT, bool_sig, TRUE);
type		return TK_TYPE;
union		return TK_UNION;
val		return TK_VAL;
var		return TK_VAR;
vm		return TK_VM;
xor		return TK_XOR;

","		return TK_COMMA;
":"		return TK_COLON;
"="		return TK_EQ;
"++"		return TK_INCR;
"+"		return TK_PLUS;
"--"		return TK_DECR;
"-"		return TK_MINUS;
"*"		return TK_ASTER;
"**"		return TK_EXPON;
"/"		return TK_DIV;
"%"		return TK_REMDR;
"("		return TK_LPAREN;
")"		return TK_RPAREN;
"->"		return TK_ARROW;
"[]"		return TK_SQUARE;
":="		return TK_ASSIGN;
":=:"		return TK_SWAP;
"["		return TK_LBRACKET;
"]"		return TK_RBRACKET;
">="		return TK_GE;
"<="		return TK_LE;
">"		return TK_GT;
"<"		return TK_LT;
"!="		return TK_NE;
"~="		return TK_NE;
"|"		return TK_OR;
"&"		return TK_AND;
"."		return TK_PERIOD;
"~"		return TK_NOT;
"@"		return TK_ADDR;
"^"		return TK_HAT;
"||"		return TK_CONCAT;
"{"		return TK_LBRACE;
"}"		return TK_RBRACE;
"?"		return TK_QMARK;
"//"		return TK_PARALLEL;
">>"		return TK_RSHIFT;
"<<"		return TK_LSHIFT;

"+:="		return TK_AUG_PLUS;
"-:="		return TK_AUG_MINUS;
"*:="		return TK_AUG_ASTER;
"**:="		return TK_AUG_EXPON;
"/:="		return TK_AUG_DIV;
"%:="		return TK_AUG_REMDR;
"|:="		return TK_AUG_OR;
"&:="		return TK_AUG_AND;
"||:="		return TK_AUG_CONCAT;
">>:="		return TK_AUG_RSHIFT;
"<<:="		return TK_AUG_LSHIFT;

[A-Za-z][A-Za-z0-9_]*   return doid();		/* identifier */
{D}+"."{D}*{E}?	|
"."{D}+{E}?	|
{D}+{E}			return doreal();	/* real literal */
[0-7]+[qQ]		return doint(8);	/* base 8 integer literal */
[0-9]+			return doint(10);	/* base 10 integer literal */
[0-9][0-9a-fA-F]*[Xx]	return doint(16);	/* base 16 integer literal */
\'			return chrlit();	/* character literal */
\"			return strlit();	/* string literal */

"/*"			comment();		/* block comment */
"*/"			FATAL ("comment terminator outside comment");
#.*			  ;			/* line comment */
[ \t\f\v]		  ;			/* whitespace */
\\[ \t]*\n		newlines++;		/* line continuation */

;			return TK_SEPARATOR;
\n			{ newlines++;	return TK_NEWLINE; }

.			err (srclocn, "illegal character `%s'", (char*)yytext);


%%



#ifndef YYLMAX
#define YYLMAX 500
#endif

extern int yydebug;		/* defined by yacc */
extern Nodeptr yylval;		/* defined by yacc */



/*  doid() -- process identifier  */

static int
doid ()
{
#ifdef YYDEBUG
    if (yydebug)
	fprintf (stderr, "`%s' ", yytext);
#endif /*YYDEBUG*/

    if (yyleng >= YYLMAX)
	toolong ();
    yylval = idnode ((char *) yytext);
    return TK_ID;
}



/*  doreal() -- interpret real literal  */

static int
doreal ()
{
    Real v;

    if (yyleng >= YYLMAX)
	toolong ();
    sscanf ((char *) yytext, "%lf", &v);
    yylval = realnode (v);
    return TK_RLIT;
}



/*  doint (base) -- interpret integer literal  */

static int
doint (base)
int base;
{
    char *p, *q;
    int n;

    if (yyleng >= YYLMAX)
	toolong ();
    p = (char *) yytext;		/* string pointer */
    q = (char *) (yytext + yyleng);	/* end+1 of string */
    if (base != 10)			/* chop "q" or "x" if not base 10 */
	q--;
    n = 0;
    while (p < q)			/* process each digit in turn */
	n = base * n + xci [*p++ & 037];
    return literal (TK_ILIT, O_ILIT, int_sig, n);
}



/*  literal (token, opr, sig, value) -- construct token for literal value  */

static int
literal (token, opr, sig, value)
int token;
Operator opr;
Sigptr sig;
int value;
{
    yylval = intnode (value);		/* value = integer value */
    yylval->e_opr = opr;		/* opr = node identifier */
    yylval->e_sig = sig;		/* sig = literal's signature */
    return token;			/* token = lex token */
}



/*  chrlit() -- process a character ('c') literal  */

static int
chrlit ()
{
    int c, d;

    c = input();
    if (c == '\n') {
	unput (c);
	FATAL ("newline in character literal");
	return literal (TK_CLIT, O_CLIT, char_sig, 0);
    }
    if (c == '\'') {
	FATAL ("empty character literal");
	return literal (TK_CLIT, O_CLIT, char_sig, 0);
    }

    if (c == '\\')
	c = escape ();

    d = input();			/* find trailing delimiter */
    if (d != '\'') {
	FATAL ("character literal contains multiple characters");
	while (d != '\'' && d != '\n' && d != 0)
	    d = input();
	if (d != 0)
	    unput (d);
    }

    return literal (TK_CLIT, O_CLIT, char_sig, c);
}



/*  strlit() -- process a string ("abc") literal  */

static int
strlit ()
{
    char buf [YYLMAX + 1];
    char *p, *q;
    int c, n, len;
    String *s;

    p = buf;
    q = p + YYLMAX;
    len = 0;
    for (c = input(); c != '"'; c = input()) {
	if (c == '\n' || c == '\0') {
	    if (c != '\0')
		unput (c);
	    FATAL ("unterminated string");
	    break;
	}
	if (c == '\\') 
	    c = escape ();
	if (p < q)
	    *p++ = c;
	len++;
    }
    *p++ = '\0';

    if (len > YYLMAX) {
	FATAL ("string too long");
	len = YYLMAX;
    }

    n = len + STRING_OVH;
    s = (String *) ralloc (n);
    s->size = n;
    s->length = len;
    memcpy (DATA (s), buf, len + 1);

    yylval = newnode (O_SLIT);
    yylval->e_sptr = s;
    yylval->e_sig = newsig (T_STRING, NULLSIG);
    yylval->e_sig->g_lb = intnode (1);
    yylval->e_sig->g_ub = intnode (len);
    return TK_SLIT;
}



/*  escape() -- interpret sequence following '\' and return resulting char  */

static int
escape ()
{
    int ntok;
    unsigned bitp;

    switch (ntok = input ()) {
    case 'a'  :     return '\007';		/* alert */
    case 'b'  :     return '\b';		/* backspace */
    case 'e'  :     return '\033';		/* escape */
    case 'f'  :     return '\f';		/* form feed */
    case 'n'  :     return '\n';		/* newline */
    case 'r'  :     return '\r';		/* return */
    case 't'  :     return '\t';		/* tab */
    case 'v'  :     return '\013';		/* vertical tab */
    case '\n' :	    newlines++; return '\n';	/* newline */
    case 'x':
	ntok = input ();
	if (isdigit(ntok) || (ntok>='a'&&ntok<='f') || (ntok>='A'&&ntok<='F')) {
	    bitp = xci [ntok & 037];
	    ntok = input ();
	    if (isdigit(ntok)||(ntok>='a'&&ntok<='f')||(ntok>='A'&&ntok<='F')) {
		return (bitp << 4) | xci [ntok & 037];
	    } else {
		unput (ntok);
		return bitp;
	    }
	} else {
	    unput (ntok);
	    return 0;
	}
    default:
	if ('0' <= ntok && ntok <= '7') {
	    bitp = ntok - '0';
	    if ((ntok = input ()) >= '0' && ntok <= '7') {
		bitp = (bitp << 3) | (ntok - '0');
		if ((ntok = input ()) >= '0' && ntok <= '7')
		    bitp = (bitp << 3) | (ntok - '0');
		else
		    unput (ntok);
	    } else
		unput (ntok);
	    return (int) bitp;
	} else
	    return ntok;
    }
}



/*  comment() -- process a block comment, /+* through *+/   */

static void
comment ()
{
    int c;

    c = input ();
    for (;;) {
	switch (c)  {
	    case 0:
	    case EOF:
		FATAL ("unterminated comment at EOF");
		return;
	    case '\n':
		newlines++;
		c = input ();
		continue;
	    case '/':
		c = input ();
		if (c == '*') {
		    comment ();
		    c = input ();
		}
		continue;
	    case '*':
		c = input ();
		if (c == '/')
		    return;
		continue;
	    default:
		c = input ();
		continue;
	}
    }
}



/*  toolong () -- diagnose a token that is too long.
 *
 *  We don't try to recover; lex has already overwritten memory.
 *  If we're lucky we can issue a diagnostic on the way out.
 */
static void
toolong ()
{
    write (2, "token too long: ", 16);
    write (2, yytext, 30);
    write (2, "...\n", 4);
    _exit (1);
}



/*  yyref () -- return pointer to yytext buffer.
 *
 *  This function allows other files to be ignorant of whether
 *  yytext is defined as char* or char[].
 */
char *
yyref ()
{
    return (char *) yytext;	/* must cast; might be unsigned */
}



/*  yywrap () -- handle EOF.
 *
 *  Supplying yywrap eliminates the need to link with the lex library.
 */
int
yywrap ()
{
    return 1;		/* indicates no additional file; return EOF token */
}



#ifdef FLEX_SCANNER

/*************  input switching for flex  ******************/

void *
lexfrom (fp)
FILE *fp;
{
    void *oldbuf;

    oldbuf = YY_CURRENT_BUFFER;
    yy_switch_to_buffer (yy_create_buffer (fp, YY_BUF_SIZE));
    return oldbuf;
}

void
lexrevert (old)
void *old;
{
    yy_delete_buffer (YY_CURRENT_BUFFER);
    yy_switch_to_buffer ((YY_BUFFER_STATE) old);
}



# else	/* not FLEX_SCANNER */

/*************  input switching for lex  ******************/

void *
lexfrom (fp)
FILE *fp;
{
    void *old;

    old = yyin;
    yyin = fp;
    return old;
}

void
lexrevert (old)
void *old;
{
    yyin = (FILE *) old;
}


#endif  /* FLEX_SCANNER */
