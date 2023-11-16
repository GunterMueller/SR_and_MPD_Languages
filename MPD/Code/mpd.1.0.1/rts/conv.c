/*  conv.c -- runtime support of builtin conversion operators  */

#include "rts.h"
#include <ctype.h>

static Ptr mpdstring ();



/********************  miscellaneous  ********************/



/*
 *  mpd_chars (s) -- convert a string into a char array.
 */
Array *
mpd_chars (s)
String *s;
{
    Array *a;
    Dim *d;
    int size;

    mpd_check_stk (CUR_STACK);
    size = sizeof (Array) + sizeof (Dim) + s->length;
    a = (Array *) mpd_alc (size, 1);
    a->size = size;
    a->offset = sizeof (Array) + sizeof (Dim);
    a->ndim = 1;
    d = (Dim *) (a + 1);
    d->lb = 1;
    d->ub = s->length;
    d->stride = 1;
    memcpy (ADATA (a), DATA (s), s->length);
    return a;
}



/********************  conversion to string  ********************/


/*
 *  mpd_fmt_int (n) -- convert an int to a String.
 */
Ptr
mpd_fmt_int (n)
int n;
{
    char s[20];
    sprintf (s, "%d", n);
    return mpdstring (s, strlen (s));
}



/*
 *  mpd_fmt_char (c) -- convert a char to a String.
 */
Ptr
mpd_fmt_char (c)
Char c;
{
    char t;	/* use a local to avoid problems with promoted args */

    t = c;
    return mpdstring (&t, 1);
}



/*
 *  mpd_fmt_bool (b) -- convert a Bool to a String.
 */
Ptr
mpd_fmt_bool (b)
Bool b;
{
    if (b)
	return mpdstring ("true", 4);
    else
	return mpdstring ("false", 5);
}



/*
 *   mpd_fmt_real (r) -- convert a real to a String.
 */
Ptr
mpd_fmt_real (r)
Real r;
{
    char s[20];

    sprintf (s, "%#g", r);
    return mpdstring (s, strlen (s));
}



/*
 *   mpd_fmt_ptr (p) -- convert a pointer to a String.
 */
Ptr
mpd_fmt_ptr (p)
Ptr p;
{
    char s[20];		/* can be up to 16 digits + \0 on 64-bit machine */

    if (p == NULL)
	strcpy (s, "==null==");
    else
	sprintf (s, "%08lX", (long) p);
    return mpdstring (s, strlen (s));
}



/*
 *  mpd_fmt_arr (a) -- convert an array of chars to a string.
 */
Ptr
mpd_fmt_arr (a)
Array *a;
{
    return mpdstring (ADATA (a), UB (a, 0) - LB (a, 0) + 1);
}



/*
 *  mpdstring (s, n) - return pointer to a newly allocated, initialized string.
 */
static Ptr
mpdstring (s, n)
char *s;
int n;
{
    String *t;

    t = mpd_alc_string (n);
    memcpy (DATA (t), s, n);
    t->length = n;
    return (Ptr) t;
}



/********************  conversion from string  ********************/



/*
 *  mpd_boolval (locn, string) -- convert string to boolean.
 */
int
mpd_boolval (locn, s)
char *locn;
String *s;
{
    Bool b;

    mpd_check_stk (CUR_STACK);
    DATA (s) [s->length] = '\0';
    if (!mpd_cvbool (DATA (s), &b))
	mpd_runerr (locn, E_BCNV, s);
    return b;
}



/*
 *  mpd_charval (s) -- convert string to character.
 *  Returns the first non-white space char in the string, or
 *  '\0' if there are only space chars in s.
 */
int
mpd_charval (s)
String *s;
{
    char *p;

    mpd_check_stk (CUR_STACK);
    p = DATA (s);
    p[s->length] = '\0';
    while (isspace (*p))
	p++;
    return *p;
}



/*
 *  mpd_intval (locn, string) - convert string to integer.
 */
int
mpd_intval (locn, s)
char *locn;
String *s;
{
    int n;

    mpd_check_stk (CUR_STACK);
    DATA (s) [s->length] = '\0';
    if (!mpd_cvint (DATA (s), &n))
	mpd_runerr (locn, E_ICNV, s);
    return n;
}



/*
 *  mpd_ptrval (locn, string) -- convert string to pointer.
 */
Ptr
mpd_ptrval (locn, s)
char *locn;
String *s;
{
    mpd_check_stk (CUR_STACK);
    if (s->length == 8 && strncmp (DATA (s), "==null==", 8) == 0)
	return 0;
    else
	return (Ptr) mpd_intval (locn, s);
}



/*
 *  mpd_realval (locn, string) -- convert string to real.
 */
Real
mpd_realval (locn, s)
char *locn;
String *s;
{
    double x;
    char c[2];

    mpd_check_stk (CUR_STACK);
    DATA (s) [s->length] = '\0';
    c[0] = '\0';
    if (sscanf (DATA (s), "%lf%1s", &x, c) != 1 || c[0] != '\0')
	mpd_runerr (locn, E_RCNV, s);
    return x;
}



/*
 *  mpd_cvbool (s, &b) -- convert string to boolean, return success or failure.
 *
 *  Accepts "true", "TRUE", "false" and "FALSE".
 *  Makes the assignment to the boolean var only if conversion succeeds.
 *  Returns 1 if successful, 0 otherwise.
 *
 *  Note: string pointer must be terminated with a null char.
 */
int
mpd_cvbool (sp, bp)
char *sp;
Bool *bp;
{
    char word[6], more[2], c, *p;

    mpd_check_stk (CUR_STACK);
    if (sscanf (sp, "%5s%1s", word, more) != 1)
	return 0;			/* nothing there, or too much */

    for (p = word; c = *p; p++)
	if (isupper (c))
	    *p = tolower (c);		/* convert to lower case */

    if (strcmp (word, "true") == 0 || strcmp (word, "t") == 0) {
	*bp = TRUE;
	return 1;
    }

    if (strcmp (word, "false") == 0 || strcmp (word, "f") == 0) {
	*bp = FALSE;
	return 1;
    }

    return 0;				/* unrecognized */
}



/*
 *  mpd_cvint (s, &i) -- convert string to integer, return success or failure.
 *
 *  Accepts integers in either decimal, octal or hexadecimal input.
 *  If the entire string is not converted then the conversion fails.
 *  Returns 1 if successful, 0 otherwise.
 *
 *  Note: string pointer must be terminated with a null char.
 */
int
mpd_cvint (sp, ip)
char *sp;
int *ip;
{
    char type = 'd';
    char c[2];
    int i, t, n;

    mpd_check_stk (CUR_STACK);
    while (isspace (*sp))
	++sp;

    n = strlen (sp);
    i = 0;
    if (sp[i] != '-' && sp[i] != '+')
	++i;


    for (; i < n; ++i) {
	if (!isdigit (sp[i])) {
	    if ((sp[i] >= 'a' && sp[i] <= 'f') ||
		(sp[i] >= 'A' && sp[i] <= 'F')) {
		type = 'x';
	    } else if (sp[i] == 'x' || sp[i] == 'X') {
		/* can only get this char at the very end of the string */
		for (++i; i < n; ++i)
		    if (!isspace (sp[i]))
			return 0;
		type = 'x';
		break;
	    } else if (sp[i] == 'q' || sp[i] == 'Q') {
		/* can only get this char at the very end of the string */
		for (++i; i < n; ++i)
		    if (!isspace (sp[i]))
			return 0;
		type = 'q';
		break;
	    } else if (isspace (sp[i])) {
		for (++i; i < n; ++i)
		    if (!isspace (sp[i]))
			return 0;
		break;
	    }
	}
    }

    switch (type) {
	case 'd':
	    c[0] = '\0';
	    t = sscanf (sp, "%d%1s", &n, c);
	    if (t != 1 || c[0] != 0)
		return 0;
	    break;
	case 'x':
	    t = sscanf (sp, "%x%1s", &n, c);
	    if (t != 2 || (c[0] != 'x' && c[0] != 'X'))
		return 0;
	    break;
	case 'q':
	    t = sscanf (sp, "%o%1s", &n, c);
	    if (t != 2 || (c[0] != 'q' && c[0] != 'Q'))
		return 0;
	    break;
	default:
	    mpd_malf ("unreachable type in mpd_cvint");
    }
    *ip = n;
    return 1;
}
