%a 1600
%o 1450
%n 500
%k 400
%p 3400
%e 1300

B	[ \t]*\n

%Start N S D LC BC LT
    /*
      N = normal
      S = single quote string, like 'abcd'
      D = double quote string, like "abcd"
      LC = line comment --- # like this
      BC = block comment like the one we're in
      LT = literal tex string
    */

%{
#ifdef __CLCC__
#pragma Warning_level(0)
#endif

#include "srlatex.h"
%}

%%

    int cn;
    int ln;
    int nest = 0;
    int ret_state = N;
    BEGIN N;
<N>P		keyword();
<N>V		keyword();
<N>af		keyword();
<N>and		keyword();
<N>any		keyword();
<N>await	{if (lang == PL) keyword(); else name();}
<N>backups	{if (lang == FTSR) keyword(); else name();}
<N>barrier	{if (lang == PL) keyword(); else name();}
<N>begin	{nest++; keyword();}
<N>body		keyword();
<N>bool		keyword();
<N>by		keyword();
<N>call		keyword();
<N>cap		keyword();
<N>chan		{if (lang == PL) keyword(); else name();}
<N>char		keyword();
<N>co		keyword();
<N>cond		{if (lang == PL) keyword(); else name();}
<N>const	keyword();
<N>create	keyword();
<N>destroy	keyword();
<N>do		keyword();
<N>downto	keyword();
<N>else		keyword();
<N>empty	{if (lang == PL) keyword(); else name();}
<N>end		{nest_out(--nest); keyword();}
<N>enum		keyword();
<N>exit		keyword();
<N>extend	keyword();
<N>external	keyword();
<N>fa		keyword();
<N>false	keyword();
<N>fi		keyword();
<N>file		keyword();
<N>final	{nest=2; keyword();}
<N>forward	keyword();
<N>global	{nest=2; keyword();}
<N>high		keyword();
<N>if		keyword();
<N>import	keyword();
<N>in		keyword();
<N>initial	{nest=2; keyword();}
<N>int		keyword();
<N>low		keyword();
<N>minrank	{if (lang == PL) keyword(); else name();}
<N>mod		keyword();
<N>monitor	{switch (lang) {
		   case SR: name(); break;
		   case FTSR: keyword(); break;
		   case PL: nest=1; keyword(); break;
	        }}
<N>monitorend	{if (lang == FTSR) keyword(); else name();}
<N>new		keyword();
<N>next		keyword();
<N>ni		keyword();
<N>noop		keyword();
<N>not		keyword();
<N>null		keyword();
<N>oc		keyword();
<N>od		keyword();
<N>on		keyword();
<N>op		keyword();
<N>optype	keyword();
<N>or		keyword();
<N>persistent	{if (lang == FTSR) keyword(); else name();}
<N>primary	{if (lang == FTSR) keyword(); else name();}
<N>private	{if (lang == FTSR) keyword(); else name();}
<N>proc		{nest=2; keyword();}
<N>procedure	{nest=2; keyword();}
<N>process	{nest=2; keyword();}
<N>ptr		keyword();
<N>real		keyword();
<N>rec		keyword();
<N>receive	keyword();
<N>recovery	{if (lang == FTSR) keyword(); else name();}
<N>ref		keyword();
<N>region	{if (lang == PL) {nest++; keyword();} else name();}
<N>reply	keyword();
<N>res		keyword();
<N>resource	{nest=1; keyword();}
<N>restart	{if (lang == FTSR) keyword(); else name();}
<N>return	keyword();
<N>returns	keyword();
<N>sem		keyword();
<N>send		keyword();
<N>separate	{nest_out(--nest); keyword();}
<N>signal	{if (lang == PL) keyword(); else name();}
<N>signal_all	{if (lang == PL) keyword(); else name();}
<N>skip		keyword();
<N>st		keyword();
<N>stderr	keyword();
<N>stdin	keyword();
<N>stdout	keyword();
<N>stop		keyword();
<N>string	keyword();
<N>to		keyword();
<N>true		keyword();
<N>type		keyword();
<N>union	keyword();
<N>val		keyword();
<N>var		keyword();
<N>vm		keyword();
<N>wait		{if (lang == PL) keyword(); else name();}
<N>when		{if (lang == PL) keyword(); else name();}
<N>xor		keyword();
<N>","		punct();
<N>":"		colon();
<N>";"		punct();
<N>"++"		punct();
<N>"+"		punct();
<N>"--"		punct();
<N>"-"		punct();
<N>"*"		punct();
<N>"**"		punct();
<N>"/"		punct();
<N>"%"		punct();
<N>"("		punct();
<N>")"		punct();
<N>"->"		arrow();
<N>"[]"		box();
<N>":="		punct();
<N>":=:"	punct();
<N>"["		punct();
<N>"]"		punct();
<N>"="		punct();
<N>">="		gteq();
<N>"<="		leeq();
<N>">"		punct();
<N>"<"		punct();
<N>"!="		neq();
<N>"~="		neq();
<N>"|"		punct();
<N>"&"		punct();
<N>"."		punct();
<N>"~"		punct();
<N>"@"		punct();
<N>"^"		punct();
<N>"//"		parallel();
<N>"{"		punct();
<N>"}"		punct();
<N>"?"		punct();
<N>"||"		punct();
<N>">>"		punct();
<N>"<<"		punct();
<N>[0-9]+			number();
<N>[0-7]+[qQ]			number();
<N>[0-9][0-9a-fA-F]*[Xx]	number();
<N>[A-Za-z][A-Za-z0-9_]*   	name();

<N>\'			{string(); BEGIN S;}
<S>\\.			{string();}
<S>[^\\ \t\n\']*	{string();}
<S>\'			{string(); BEGIN N;}

<N>\"			{string(); BEGIN D;}
<D>\\.			{string();}
<D>[^\\ \t\n\"]*	{string();}
<D>\"			{string(); BEGIN N;}

<N,BC,LC>"#<"		{
			  if (literal_escape) {
			      ret_state = YYSTATE;
			      ln = 1;
			      BEGIN LT;
			  } else {
			      comment();
			      BEGIN LC;
			  }
		        }
<LT>">"			{literal();}
<LT>"#<"		{literal(); ln++;}
<LT>[^>]*		{literal();}
<LT>">#"		{if (--ln == 0) BEGIN ret_state; else literal();}

<N>#			{comment(); BEGIN LC;}
<LC>([^# \t\n]*|#)	{comment();}
<LC>\n			{endline(); startline(); BEGIN N;}

<N>"/*"			{comment(); cn = 1; BEGIN BC;}
<BC>\\.			{comment();}
<BC>"*"			{comment();}
<BC>"/*"		{comment(); cn++;}
<BC>([^#\\ \t\n*]*|#)	{comment();}
<BC>"*/"		{comment(); if (--cn == 0) BEGIN N;}

<N,S,D,BC>\n([ \t]*\n)*	{endline(); blankline();startline();}

<N,S,D,BC>\f		newpage();
<N,S,D,LC,BC>[ \t]*	whitespace();

.			bad_char();

%%



char * yyref ()		/* return pointer to yytext buffer */
{
    return (char *) yytext;	/* must cast; might be unsigned */
}

int yywrap ()		/* supplied to eliminate need for linking with lexlib */
{
    return 1;		/* indicates no additional file; return EOF token */
}
