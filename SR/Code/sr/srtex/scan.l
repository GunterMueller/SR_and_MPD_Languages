%a 1200
%o 1450
%n 400
%k 400
%p 3000
%e 1200

B	[ \t]*\n

%Start N S D LC BC
    /*
      N = normal
      S = single quote string, like 'abcd'
      D = double quote string, like "abcd"
      LC = line comment --- # like this
      BC = block comment like the one we're in
    */

%{
#ifdef __CLCC__
#pragma Warning_level(0)
#endif
%}

%%

    int cn;
    int nest = 0;
    BEGIN N;

<N>P		keyword();
<N>V		keyword();
<N>af		keyword();
<N>and		keyword();
<N>any		keyword();
<N>begin	{nest++; keyword();}
<N>body		keyword();
<N>bool		keyword();
<N>by		keyword();
<N>call		keyword();
<N>cap		keyword();
<N>char		keyword();
<N>co		keyword();
<N>const	keyword();
<N>create	keyword();
<N>destroy	keyword();
<N>do		keyword();
<N>downto	keyword();
<N>else		keyword();
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
<N>mod		keyword();
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
<N>proc		{nest=2; keyword();}
<N>procedure	{nest=2; keyword();}
<N>process	{nest=2; keyword();}
<N>ptr		keyword();
<N>real		keyword();
<N>rec		keyword();
<N>receive	keyword();
<N>ref		keyword();
<N>reply	keyword();
<N>res		keyword();
<N>resource	{nest=1; keyword();}
<N>return	keyword();
<N>returns	keyword();
<N>sem		keyword();
<N>send		keyword();
<N>separate	{nest_out(--nest); keyword();}
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

<N>#			{comment(); BEGIN LC;}
<LC>[^ \t\n]*		{comment();}
<LC>\n			{endline(); startline(); BEGIN N;}

<N>"/*"			{comment(); cn = 1; BEGIN BC;}
<BC>\\.			{comment();}
<BC>"*"			{comment();}
<BC>"/*"		{comment(); cn++;}
<BC>[^\\ \t\n*]*	{comment();}
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
