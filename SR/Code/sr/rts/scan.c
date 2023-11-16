/*  scan.c -- scanf and sscanf  */

#include <ctype.h>
#include <stdarg.h>
#include "rts.h"

static int scanToken (), scanInteger (), scanReal (), scanPointer ();
static int scanTokenFromSet ();

#define BACKCH(c,f,S,s) (S ? ((s>DATA (S)) ? (s)-- : 0) : (char*)ungetc (c, f))
#define NEXTCH(l,f,S,s) (S ? ((*s=='\0') ? EOF : * (Char*)(s)++) : INCH (l, f))

#define SCAN_LEN 50
#define ANSI_LIMIT 512

/*
 *  sr_scanf (locn, fp, sarg, fmt, argt, arg1...) -- handle scanf and sscanf
 *
 *	locn	source code location, for traceback
 *	fp	file pointer for scanf; null for sscanf
 *	sarg	result string for sscanf; null for scanf
 *	fmt	scanning format
 *	argt	string indicating argument types
 *	arg1...	pointers to arguments
 *
 *  This implements a mind-boggling number of input formats:
 *	o, q :	octal integer only (may or may not end with q or Q)
 *	d, u :	decimal integer only
 *	x :	hexadecimal integer only (may or may not end with x or X)
 *	i :	integer type is determined by input.  if ends in q or Q
 *		then it is octal.  ending in x or X means hex.
 *		otherwise it is decimal.  error if hex digits are read
 *		and the literal does not end with x or X.
 *	c :	read chars without skipping white space, default field width
 *		is one.
 *	s :	reads string separated by white space.  will read until
 *		a space character is read or the limit of the target
 *		string is encountered.
 *	e, f, g :	reals.
 *	p :	reads eight hex chars, default field width is eight.
 *		accepts the special string "==null==".
 *	b :	boolean.  Acceptable string are "true", "TRUE", "false" or
 *		"FALSE".  The default field width is to read for one of
 *		above strings (4 or 5 chars).  If a field width is specified,
 *		scan stops after field width chars are read and the scanned
 *		input is compared against one of the acceptable strings.
 *
 *  field specifiers are of the form: %[*][digits]$
 *  where the digits are optional, and $ is one of the above formats.
 *  The optional '*' indicates suppression: the input will be read but not
 *  assigned.  Even if a field is suppressed the input is checked as if it
 *  will be assigned.  Field width must be greater than 0 and less than
 *  or equal to ANSI_LIMIT (512).  In addition, the number of digits
 *  permitted for a field width is SCAN_LEN (50).
 *
 *  This function returns when it reaches EOF or if any conversion fails.
 *  Upon reaching EOF the number of successful conversions is returned
 *  (or EOF).  The number of successful conversions is returned otherwise.
 */
int
sr_scanf (char *locn, FILE *fp, String *sarg, String *sfmt, Ptr argt, ...)
{
    va_list ap;
    Array *a;
    String *sp;
    Real *rp, real;
    Ptr *pp, pointer;
    Bool *bp, b;
    Char *cp;
    int *ip;
    long lp;

    int i, l, n, suppress, fldWidth, hasFldWidth, nread = 0;
    char *fmt;
    char c;
    int ch;
    char *s, *inString;
    char buf[ANSI_LIMIT + 1];
    char charset[256];
    int in_set;

    sr_check_stk (CUR_STACK);

    va_start (ap, argt);

    if (sarg) {
	inString = DATA (sarg);			/* get string address */
	inString[sarg->length] = '\0';		/* terminate with \0 */
	PRIV (io_handoff) = 0;
    } else {
	BEGIN_IO (fp);
	CHECK_FILE (locn, fp, EOF);
    }

    fmt = DATA (sfmt);
    fmt[sfmt->length] = '\0';			/* terminate format with \0 */

    /*
     *  Main Loop.
     */
    c = *fmt++;
    while (c != '\0') {
	if (c == '%') {
	    /*
	     *  if this char is a %, then this is the beginning of a
	     *  field specifier.
	     */
	    c = *fmt++;

	    if (c == '%') {		/* read the literal % from the input */
		ch = NEXTCH (locn, fp, sarg, inString);
		if (ch != c)
		    if (ch == EOF && nread == 0)
			IO_RETURN (fp, EOF);
		    else
			IO_RETURN (fp, nread);
		c = *fmt++;
		continue;
	    } else if (c == '*') {
		suppress = 1;
		c = *fmt++;
	    } else
		suppress = 0;

	    if (isdigit (c)) {
		/*
		 *  Read the optional field width
		 */
		buf[0] = c;
		/*
		 * even though buf will not overflow until n > 512, stop
		 * after 50.  this is consistent with sr_read ().
		 */
		for (n = 1; isdigit (c = *fmt++) && n < SCAN_LEN; n++)
		    buf[n] = c;

		if (n >= SCAN_LEN)
		    ABORT_IO (fp, locn, "field width specifier too long");

		buf[n] = '\0';
		n = sscanf (buf, "%d", &fldWidth);
		if (n != 1)
		    ABORT_IO(fp,locn,"field width specifier conversion failed");
		if (fldWidth > ANSI_LIMIT)
		    ABORT_IO (fp, locn, "field width too large");
		else if (fldWidth <= 0)
		    ABORT_IO (fp,locn,"field width must be greater than zero");
		hasFldWidth = 1;	      /* this is needed for b, c & p */
	    } else {
		if (*argt == 'c' && !suppress)
		    fldWidth = 1;
		else
		    fldWidth = ANSI_LIMIT;
		hasFldWidth = 0;	      /* this is needed for b, c & p */
	    }

	    switch (c) {
	    case '\0':
		ABORT_IO (fp, locn, "premature termination of format string");
		break;

	    case 'o':
		c = 'q';
		/* NO BREAK */
	    case 'q':
	    case 'd':
	    case 'u':
	    case 'x':
	    case 'i':
		/*
		 *  all the integer cases are handled here.
		 */

		n = scanInteger (locn, fp, sarg, &inString, buf, fldWidth, &c);
		if (n == EOF)
		    IO_RETURN (fp, nread ? nread : EOF);
		if (n == 0)
		    IO_RETURN (fp, nread);

		if (!suppress) {
		    /*
		     *  if this field is not suppressed then get the next
		     *  argument.  It should be a pointer to an integer.
		     */
		    if (*argt != 'i')
			ABORT_IO (fp, locn,
			    "arguments do not match format string");
		    ip = va_arg (ap, int *);
		    ++argt;

		    if (c == 'q')
			n = sscanf (buf, "%o", &i);
		    else if (c == 'x')
			n = sscanf (buf, "%x", &i);
		    else
			n = sscanf (buf, "%d", &i);
		    if (n == 1) {
			*ip = i;
			++nread;
		    } else
			IO_RETURN (fp, nread);
		}
		break;		/*----  end of integer conversion */

	    case '[':
		/*
		 * Handle the charset (%[...]) format type here.
		 * First, build the charset table.
		 */
		if (*fmt == '^') {
		    in_set = 0;
		    fmt++;
		} else
		    in_set = 1;
		memset (charset, !in_set, sizeof (charset));
		s = fmt;

		while (((c = *fmt++) != ']') || (fmt == s + 1)) {
		    if (c == '\0')
			ABORT_IO (fp, locn,
			    "premature termination of format string");
		    if (*fmt == '-') {
			fmt++;
			n = *fmt++ & 0xFF;
			if (n == '\0')
			    ABORT_IO (fp, locn,
				"premature termination of format string");
			for (i = c & 0xFF; i <= n; i++)
			    charset[i] = in_set;
		    } else {
			charset[c & 0xFF] = in_set;
		    }
		}

		/*
		 * The rest of this is similar to %s handling.
		 */
		if (fldWidth == 1 && *argt == 'c' && !suppress) {
		    /*
		     * do this only if format is '%1s', arg is char AND
		     * the field is NOT suppressed.
		     */
		    ch = NEXTCH (locn, fp, sarg, inString);

		    if (ch == EOF)
			IO_RETURN (fp, nread ? nread : EOF);

		    if (!charset[ch])
			IO_RETURN (fp, nread ? nread : EOF);

		    cp = va_arg (ap, Char *);
		    *cp = ch;
		    ++nread;
		    ++argt;

		} else if (*argt == 'a' && !suppress) {

		    /*
		     * scan into array of characters
		     */
		    ++argt;
		    a = (Array *) va_arg (ap, Array *);
		    l = UB (a, 0) - LB (a, 0) + 1;

		    /*
		     *  Scan max of (length of string, field width)
		     */
		    i = (l < fldWidth) ? l : fldWidth;
		    if ((n = scanTokenFromSet
			(locn, fp, sarg, &inString, buf, i, charset)) == EOF)
			    IO_RETURN (fp, nread ? nread : EOF);
		    if (n == 0)
			IO_RETURN (fp, nread);

		    ++nread;
		    strncpy (ADATA (a), buf, n);
		    /*
		     *  blank out the remainder of the input array
		     */
		    while (n < l)
			ADATA (a) [n++] = ' ';

		} else {
		    n = scanTokenFromSet (locn, fp, sarg, &inString, buf,
						fldWidth, charset);
		    if (n == EOF)
			IO_RETURN (fp, nread ? nread : EOF);
		    if (n == 0)
			IO_RETURN (fp, nread);

		    if (!suppress) {
			if (*argt != 's')
			    ABORT_IO (fp, locn,
				"arguments do not match format string");
			
			sp = (String *) va_arg (ap, String *);
			++argt;
			
			/*
			 * set length limit (l) to lesser of
			 * size of String or or the number of chars read.
			 */
			l = MAXLENGTH (sp);
			l = (l < n) ? l : n;
			
			strncpy (DATA (sp), buf, l);
			sp->length = l;
			++nread;
		    }
		}
		break;	/*----  end of charset conversion */

	    case 's':
		/*
		 * handle the string (s) format type here.
		 * if this field is suppressed then do not consume an arg.
		 * have to special case the format %1s to allow assignment
		 * to a char.
		 */
		if (fldWidth == 1 && *argt == 'c' && !suppress) {
		    /*
		     * do this only if format is '%1s', arg is char AND
		     * the field is NOT suppressed.
		     */
		    /* consume white space */
		    while (isspace (ch = NEXTCH (locn, fp, sarg, inString)))
			;

		    if (ch == EOF)
			IO_RETURN (fp, nread ? nread : EOF);

		    cp = va_arg (ap, Char *);
		    *cp = ch;
		    ++nread;
		    ++argt;

		} else if (*argt == 'a' && !suppress) {

		    /*
		     * scan into array of characters
		     */
		    ++argt;
		    a = (Array *) va_arg (ap, Array *);
		    l = UB (a, 0) - LB (a, 0) + 1;

		    /*
		     *  Scan no more than max (length of string, field width)
		     */
		    i = (l < fldWidth) ? l : fldWidth;
		    if ((n = scanToken (locn,fp,sarg,&inString,buf,i)) == EOF)
			IO_RETURN (fp, nread ? nread : EOF);
		    if (n == 0)
			IO_RETURN (fp, nread);

		    ++nread;
		    strncpy (ADATA (a), buf, n);
		    /*
		     *  blank out the remainder of the input array
		     */
		    while (n < l)
			ADATA (a) [n++] = ' ';

		} else {
		    n = scanToken (locn, fp, sarg, &inString, buf, fldWidth);
		    if (n == EOF)
			IO_RETURN (fp, nread ? nread : EOF);
		    if (n == 0)
			IO_RETURN (fp, nread);

		    if (!suppress) {
			if (*argt != 's')
			    ABORT_IO (fp, locn,
				"arguments do not match format string");
			
			sp = (String *) va_arg (ap, String *);
			++argt;
			
			/*
			 * set length limit (l) to lesser of
			 * size of String or or the number of chars read.
			 */
			l = MAXLENGTH (sp);
			l = (l < n) ? l : n;
			
			strncpy (DATA (sp), buf, l);
			sp->length = l;
			++nread;
		    }
		}
		break;		/*----  end of string format conversion */

	    case 'c':
		if (!hasFldWidth)
		    fldWidth = 1;

		if (suppress) {
		    for (i = 0; i < fldWidth; ++i) {
			if (ch == EOF)
			    IO_RETURN (fp, nread ? nread : EOF);
			ch = NEXTCH (locn, fp, sarg, inString);
		    }
		    break;
		}

		if (*argt == 'c') {
		    if (hasFldWidth && fldWidth != 1)
			ABORT_IO (fp, locn,
			    "invalid field width for char variable");
		    cp = va_arg (ap, Char *);
		    ch = NEXTCH (locn, fp, sarg, inString);
		    if (ch == EOF)
			IO_RETURN (fp, nread ? nread : EOF);
		    *cp = ch;
		    ++argt;
		    ++nread;
		} else if (*argt == 'a') {
		    char *p;

		    ++argt;
		    a = (Array *) va_arg (ap, Array *);
		    p = ADATA (a);
		    l = UB (a, 0) - LB (a, 0) + 1;
		    if (l == 0)
			break;			/* empty array */

		    i = (l < fldWidth) ? l : fldWidth;
		    for (n = 0; n < i; ++n) {
			ch = NEXTCH (locn, fp, sarg, inString);
			if (ch == EOF)
			    break;
			buf[n] = ch;
		    }

		    if (n == 0)			/* must have reached EOF */
			IO_RETURN (fp, nread ? nread : EOF);

		    ++nread;
		    strncpy (p, buf, n);
		    if (n < l) {
			s = p;
			s += n;
			for (; n < l; n++, *s++ = ' ')
			    ;
		    }
		    if (ch == EOF)
			IO_RETURN (fp, nread);	/* nread IS greater than 0 */
		} else if (*argt == 's') {
		    sp = (String *) va_arg (ap, String *);
		    ++argt;
			
		    l = MAXLENGTH (sp);
		    i = (l < fldWidth) ? l : fldWidth;
		    for (n = 0; n < i; ++n) {
			ch = NEXTCH (locn, fp, sarg, inString);
			if (ch == EOF)
			    break;
			buf[n] = ch;
		    }

		    if (n == 0)			/* must have reached EOF */
			IO_RETURN (fp, nread ? nread : EOF);

		    strncpy (DATA (sp), buf, n);
		    sp->length = n;
		    ++nread;

		    if (ch == EOF)
			IO_RETURN (fp, nread);	/* nread IS greater than zero */
		} else
		    ABORT_IO (fp, locn,"arguments do not match format string");
		break;

	    case 'e':
	    case 'f':
	    case 'g':
		n = scanReal (locn, fp, sarg, &inString, buf, fldWidth);
		if (n == EOF)
		    IO_RETURN (fp, nread ? nread : EOF);
		if (n == 0)
		    IO_RETURN (fp, nread);

		if (!suppress) {
		    if (*argt != 'r')
			ABORT_IO (fp, locn,
			    "arguments do not match format string");
		    rp = va_arg (ap, Real *);
		    ++argt;
		    i = sscanf (buf, "%lf", &real);
		    if (i == 1) {
			*rp = real;
			++nread;
		    } else
			IO_RETURN (fp, nread);
		}

		break;

	    case 'b':
		/*
		 * if a field width has been specified then read exactly
		 * fldWidth chars.  otherwise read 4 if true and 5 if false.
		 */
		if (hasFldWidth) {
		    n = scanToken (locn, fp, sarg, &inString, buf, fldWidth);
		    if (n == EOF)
			IO_RETURN (fp, nread ? nread : EOF);
		    if (n != fldWidth)
			IO_RETURN (fp, nread);
		    else if (strncmp (buf, "true ", fldWidth) == 0 ||
			    strncmp (buf, "TRUE ", fldWidth) == 0)
			b = TRUE;
		    else if (strncmp (buf, "false", fldWidth) == 0 ||
			    strncmp (buf, "FALSE", fldWidth) == 0)
			b = FALSE;
		    else
			IO_RETURN (fp, nread);
		} else {
		    n = scanToken (locn, fp, sarg, &inString, buf, 4);
		    if (n == EOF)
			IO_RETURN (fp, nread ? nread : EOF);

		    if (n != 4)
			IO_RETURN (fp, nread);
		    else if (strncmp (buf, "true", 4) == 0 ||
			    strncmp (buf, "TRUE", 4) == 0)
			b = TRUE;
		    else if (strncmp (buf, "fals", 4) == 0 ||
			    strncmp (buf, "FALS", 4) == 0) {
			if ((ch = NEXTCH (locn, fp, sarg, inString)) == EOF)
			    IO_RETURN (fp, (nread > 0) ? nread : EOF);
			if (ch == 'e' && buf[0] == 'f')
			    b = FALSE;
			else if (ch == 'E' && buf[0] == 'F')
			    b = FALSE;
			else
			    IO_RETURN (fp, nread);
		    } else
			IO_RETURN (fp, nread);
		}

		if (!suppress) {
		    if (*argt != 'b')
			ABORT_IO (fp, locn,
			    "arguments do not match format string");
		    bp = va_arg (ap, Bool *);
		    *bp = b;
		    ++nread;
		    ++argt;
		}
		break;		/*---- end of bool conversion */

	    case 'p':
		/*
		 * default for pointer field is eight.
		 */
		if (!hasFldWidth)
		    fldWidth = 8;

		n = scanPointer (locn, fp, sarg, &inString, buf, fldWidth);
		if (n == EOF)
		    IO_RETURN (fp, nread ? nread : EOF);
		if (n == 0)
		    IO_RETURN (fp, nread);

		if (strcmp (buf, "==null==") == 0)
		    pointer = NULL;
		else {
		    i = sscanf (buf, "%lx", &lp);
		    if (i != 1)
			IO_RETURN (fp, nread);
		    pointer = (Ptr) lp;
		}

		if (!suppress) {
		    if (*argt != 'p')
			ABORT_IO (fp, locn,
			    "arguments do not match format string");
		    pp = va_arg (ap, Ptr *);
		    ++argt;
		    *pp = pointer;
		    ++nread;
		}
		break;

	    default:
		ABORT_IO (fp, locn, "invalid field specifier in format string");
	    } /* switch */

	} else {

	    /*
	     * format char is not a '%'.
	     * consume the next char in the input.  if it doesn't match
	     * the format char exactly then return.
	     */
	    ch = NEXTCH (locn, fp, sarg, inString);
	    if (c != ch)
		IO_RETURN (fp, nread);
	}
	if (c != '\0')
	    c = *fmt++;
    } /* while */

    /* check and see if there were too many args passed */
    if (*argt != '\0')
	ABORT_IO (fp, locn, "more arguments than fields in format string");
    IO_RETURN (fp, nread);
}



/*
 *  Scan a token from either the file or string into the buffer.
 *  Token is any string of consecutive non-white chars.
 *  Skips leading white space and scans up to len chars into buf.
 *
 *  This is always called between a BEGIN_IO and an END_IO so we
 *  do not do it here.
 */
static int
scanToken (locn, fp, S, s, buf, len)
char *locn;
FILE *fp;
String *S;
char **s, *buf;
int len;
{
    int n, ch;

    /* skip white space */
    while (isspace (ch = NEXTCH (locn, fp, S, *s)))
	;

    if (ch == EOF)
	return EOF;

    n = 0;
    while (ch != EOF && !isspace (ch) && n < len) {
	buf[n++] = ch;
	ch = NEXTCH (locn, fp, S, *s);
    }

    BACKCH (ch, fp, S, *s);

    buf[n] = '\0';
    return n;
}

/*
 *  Scan a token from either the file or string into the buffer.
 *  Token is any string of chars from the charset supplied.
 *  Scans up to len chars into buf.
 *
 *  This is always called between a BEGIN_IO and an END_IO so we
 *  do not do it here.
 */
static int
scanTokenFromSet (locn, fp, S, s, buf, len, charset)
char *locn;
FILE *fp;
String *S;
char **s, *buf;
int len;
char *charset;
{
    int n, ch;

    ch = NEXTCH (locn, fp, S, *s);

    if (ch == EOF)
	return EOF;

    n = 0;
    while (ch != EOF && charset[ch] && n < len) {
	buf[n++] = ch;
	ch = NEXTCH (locn, fp, S, *s);
    }

    BACKCH (ch, fp, S, *s);

    buf[n] = '\0';
    return n;
}



/*
 * Read characters that make an integer literal into buf (without conversion).
 * Return the number of characters accepted.
 *
 * len is the maximum field width.
 *
 * type tells what kind of integer is expected (d, q, x or i).
 * In the case of 'i' will accept 'd' or 'x'.  type is set to 'x' if a hex
 * digit is encountered.  If "123abc" is input for type 'i', then assign
 * 123 to int and put back the c.   Type 'u' cannot begin with a '-';
 * all other types may have an optional sign.
 *
 *  This is always called between a BEGIN_IO and an END_IO so we
 *  do not do it here.
 */
static int
scanInteger (locn, fp, S, s, buf, len, type)
char *locn;
FILE *fp;
String *S;
char **s, *buf;
int len;
char *type;
{
    int n, ch;
    int need_x = 0;

    /* skip white space */
    while (isspace (ch = NEXTCH (locn, fp, S, *s)))
	;
    if (ch == EOF)
	return EOF;

    /* check for leading sign */
    n = 0;
    if (ch == '-') {
	if (*type == 'u') {
	    BACKCH (ch, fp, S, *s);
	    return 0;
	}
	buf[n++] = ch;
    } else if (ch == '+') {
	buf[n++] = ch;
    } else {
	BACKCH (ch, fp, S, *s);
    }

    /* scan the digits */
    while (n < len && (ch = NEXTCH (locn, fp, S, *s)) != EOF) {
	
	if (ch >= '0' && ch <= '7') {
	    /* okay for all formats */
	    buf[n++] = ch;
	} else if (ch == '8' || ch == '9') {
	    /* not okay for octal (q) format */
	    if (*type == 'q') {
		BACKCH (ch, fp, S, *s);
		break;
	    } else {
		buf[n++] = ch;
	    }
	} else if ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
	    /* only okay for hex (x) and i formats */
	    if (*type == 'i') {
		*type = 'x';
		need_x = 1;
	    }

	    if (*type == 'x') {
		buf[n++] = ch;
	    } else {
		BACKCH (ch, fp, S, *s);
		break;
	    }
	} else if ((ch == 'x' || ch == 'X') && (*type == 'x' || *type == 'i')) {
	    *type = 'x';
	    break;
	} else if ((ch == 'q' || ch == 'Q') && (*type == 'q' || *type == 'i')) {
	    *type = 'q';
	    break;
	} else {
	    BACKCH (ch, fp, S, *s);		/* put back readahead char */
	    break;
	}
    }

    /*
     *  if started as 'i' format and read hex chars then must have
     *  an 'x' or 'X' at the end.
     */
    if (need_x && (ch != 'x' && ch != 'X')) {
	BACKCH (ch, fp, S, *s);
	return 0;
    }

    buf[n] = '\0';				/* terminate the string */
    return n;					/* return its length */
}



/*
 *  Scan a real literal.  Consumes leading white space, an optional sign, etc.
 *  Stops scanning at first character that cannot belong to a real.
 *  Copies scanned characters (less white space) into buf.
 *  Scans no more than len chars.  Returns length of string if successful,
 *  zero otherwise.
 *
 *  This is always called between a BEGIN_IO and an END_IO so we
 *  do not do it here.
 */
static int
scanReal (locn, fp, S, s, buf, len)
char *locn;
FILE *fp;
String *S;
char **s, *buf;
int len;
{
    int n, ch;
    int intpart = 0;	/* integer part seen? */
    int decimal = 0;	/* decimal point seen? */

    /* consume white space */
    while (isspace (ch = NEXTCH (locn, fp, S, *s)))
	;

    if (ch == EOF)
	return EOF;

    n = 0;
    /* save optional sign char */
    if (ch == '-' || ch == '+') {
	buf[n++] = ch;
	ch = NEXTCH (locn, fp, S, *s);
    }

    if (n >= len)
	return 0;

    /* save all digits */
    if (isdigit (ch))
	intpart = 1;

    while (isdigit (ch)) {
	buf[n++] = ch;
	if (n >= len) {
	    buf[n] = '\0';
	    return n;
	}
	ch = NEXTCH (locn, fp, S, *s);
    }

    if (n >= len) {
	buf[n] = '\0';
	return n;
    }

    /* can have a decimal followed by more digits */
    if (ch == '.') {
	decimal = 1;
	buf[n++] = ch;
	ch = NEXTCH (locn, fp, S, *s);

	if (n >= len)
	    return 0;
	if (!isdigit (ch) && !intpart)
	    return 0;

	while (isdigit (ch)) {
	    buf[n++] = ch;
	    if (n >= len) {
		buf[n++] = '\0';
		return n;
	    }
	    ch = NEXTCH (locn, fp, S, *s);
	}
    }

    /* can have an exponent */
    if ((ch == 'e' || ch == 'E')) {
	buf[n++] = ch;
	if (n >= len)
	    return 0;
	ch = NEXTCH (locn, fp, S, *s);
	if ((ch == '-' || ch == '+')) {
	    buf[n++] = ch;
	    if (n >= len)
		return 0;
	    ch = NEXTCH (locn, fp, S, *s);
	}

	if (!isdigit (ch)) {	/* must have at least one digit in exponent */
	    if (decimal) {
		BACKCH (ch, fp, S, *s);
		buf[n] = '\0';
		return n;
	    } else
		return 0;
	}

	while (isdigit (ch)) {
	    buf[n++] = ch;
	    if (n >= len) {
		buf[n] = '\0';
		return n;
	    }
	    ch = NEXTCH (locn, fp, S, *s);
	}
    }

    /* an extra char is ALWAYS read */
    if (ch != EOF)
	BACKCH (ch, fp, S, *s);

    buf[n] = '\0';
    return n;
}



/*
 *  Scan for a pointer.  Skips leading white space.  Accepts special
 *  string "==null==" for the NULL pointer.
 *
 *  This is always called between a BEGIN_IO and an END_IO so we
 *  do not do it here.
 */
#define isptrchar(c) (isdigit (c) || (c>='a'&&c<='f') || (c>='A'&&c<='F'))

static int
scanPointer (locn, fp, S, s, buf, len)
char *locn;
FILE *fp;
String *S;
char **s, *buf;
int len;
{
    int n, ch;

    /* skip white space */
    while (isspace (ch = NEXTCH (locn, fp, S, *s)))
	;

    if (ch == EOF)
	return EOF;

    /* check for special string '==null==' */
    if (ch == '=') {
	for (n = 0; n < len; ++n) {
	    if (isspace (ch))
		break;
	    buf[n] = ch;
	    ch = NEXTCH (locn, fp, S, *s);
	}
	BACKCH (ch, fp, S, *s);
    } else {
	for (n = 0; n < len; ++n) {
	    if (!isptrchar (ch))
		break;
	    buf[n] = ch;
	    ch = NEXTCH (locn, fp, S, *s);
	}
	if (ch != 'x' && ch != 'X')
	    BACKCH (ch, fp, S, *s);
    }
    buf[n] = '\0';
    return n;
}
