/*  netpath.c -- for making a host-independent path of the executable  */

#include <ctype.h>
#include <stdio.h>
#include "../config.h"
#include "../gen.h"
#include "../srmulti.h"
#include "multimac.h"
#include "debug.h"

#define MAX_SUBST 35

static void trimc (), subst ();
static int fmatch (), words ();



/*  netpath (fname, dir, mapfile, result) -- synthesize network path of file.
 *
 *  fname is name of file; dir is directory in which to look.
 *  mapfile is a file of substitution patterns mapping "host:file" to netpath.
 *
 *  netpath returns a pointer to the result, or 0 for failure.
 */

char *
netpath (fname, dir, mapfile, result)
char *fname, *dir, *mapfile, *result;
{
    FILE *f;
    char hbuf[HOST_NAME_LEN + 1 + MAX_PATH + 1], line[MAX_LINE];
    char *pa[MAX_SUBST], *w[2];
    int la[MAX_SUBST];
    char *p;

    DEBUG (D_SRX_ACT, "netpath (%s, %s, %s)", fname, dir, mapfile);

    /* apparently gethostname must be run on the IO Server */
    BEGIN_IO (NULL);
    gethostname (hbuf, MAX_LINE);	/* get hostname */
    END_IO (NULL);

    if (! (p = strchr (hbuf, '.')))	/* chop at first '.' */
	p = hbuf + strlen (hbuf);
    if (*fname == '/')  {
	dir = "";			/* ignore dir if file starts with '/' */
	fname++;
	}
    sprintf (p, ":%s/%s", dir, fname);	/* form "host:file" string */
    DEBUG (D_SRX_ACT, "target  %s", hbuf, 0, 0);

    f = fopen (mapfile, "r");		/* open mapping file */
    if (!f)  {
	perror (mapfile);
	return 0;
	}

    while (fgets (line, MAX_LINE, f))  {	/* read pattern line */
	trimc (line, '#');			/* trim comments */
	if (words (line, w, 2) < 2)		/* break into words */
	    continue;				/* ignore empty lines */
	if (fmatch (hbuf, w[0], pa, la)) {	/* check for pattern match */
	    fclose (f);
	    DEBUG (D_SRX_ACT, "matched %s, template %s", w[0], w[1], 0);
	    subst (w[1], '$', pa, la, result);	/* substitute params */
	    return result;
	}
    }

    fclose (f);
    fprintf (stderr, "no match for %s in pattern file %s\n", hbuf, mapfile);
    return 0;
}



/*  trimc (s, c) -- trim string at comments character c.
 *
 *  The string s is chopped by adding a null character after the last
 *  "significant" character.  Everything after the first occurrence of c
 *  is discarded, and then any trailing whitespace is truncated.
 */

static void
trimc (s, c)
char *s, c;
{
    char *p;

    if (! (p = strchr (s, c)))		/* trim comments */
	p = s + strlen (s);		/* or find end if none */
    do
	p--;
    while (p >= s && isspace (*p));	/* trim whitespace */
    p[1] = '\0';			/* truncate string */
}



/*  words (s, w, n) -- break string into words.
 *
 *  The n-element array w is filled with pointers to the words in s.  Words are
 *  separated by spans of whitespace; an initial span is ignored.  s is modified
 *  in place with '\0' added to terminate each word.  Words returns the number
 *  of words found.
 */

static int
words (s, w, n)
char *s;
char *w[];
int n;
{
    int i;

    for (i = 0; i < n;)  {	/* for each word until array is filled */
	while (isspace (*s))		/* skip whitespace */
	    s++;
	if (!*s)			/* quit at end of line */
	    break;
	w[i++] = s;			/* set word pointer */
	while (*s && !isspace (*s))
	    s++;			/* find end */
	if (*s)
	    *s++ = '\0';		/* terminate (unless already EOL) */
    }
    while (i < n)
	w[--n] = s;		/* fill remaining slots with empty strings */
    return i;			/* return count of words found */
}



/*  fmatch (fname, pattern, pa, la) -- match filename against pattern.
 *
 *  fmatch checks fname against pattern, returning 1 iff successfully matched.
 *  pattern can contain special globbing patterns
 *	?	to match any character except '/'
 *	*	to match any number of non-'/' characters
 *	**	to match any number of characters including '/'
 *
 *  pa is a [char] pointer array and la is an [int] length array to receive
 *  start and length of each substring matched by a globbing pattern.  The
 *  size of each array should equal the number of globbing patterns.
 */

static int
fmatch (f, p, pa, la)
char *f, *p, *pa[];
int la[];
{
    char c;

    for (;;)
	switch (c = *p++)  {

	    case '\0':			/* end of pattern */
		return *f == '\0';

	    case '?':			/* match any character but '/' */
		if (*f == '\0' || *f == '/')
		    return 0;
		*pa++ = f++;		/* set match address and length */
		*la++ = 1;
		break;

	    case '*':			/* match span of characters */
		if (*p == '*')  {
		    ++p;
		    c = '\0';			/* for **, stop only at EOL */
		} else
		    c = '/';			/* for *, also stop at '/' */
		*pa = f;		/* save address of match */
		*la = 0;		/* init length of match */
		/* try matching the rest of the pattern (recursing) beginning at
		 * successive positions in the file name until one succeeds */
		for (;;) {
		    if (fmatch (f, p, pa + 1, la + 1))	/* recurse */
			return 1;		/* quit on success */
		    if (*f == '\0' || *f == c)
			return 0;		/* quit on EOL and maybe '/' */
		    ++f;		/* bump position in filename */
		    ++*la;		/* increment match length */
		    }

	    default:			/* match one particular character */
		if (*f == '\0')
		    return 0;
		if (*f++ != c)
		    return 0;
		break;
	}
}



/*  subst (src, c, pa, la, dst) -- substitute parameters in template.
 *
 *  src is a string template containing "$n" wherever parameter n is to be
 *  substituted.  $ is the substitution character c, and n is '1' through '9'
 *  or 'A' through 'Z', allowing up to 35 parameters. pa is the parameter array;
 *  la, if supplied, is an array of parameter lengths.  dst receives the result.
 */

static void
subst (src, c, pa, la, dst)
char *src, c, *pa[], *dst;
int la[];
{
    char t;
    int l, n;
    char *p;

    while (t = (*dst++ = *src++))	/* normally, just copy characters */
	if (t == c)  {			/* whoops, time to substitute */
	    dst--;			/* backup store pointer */
	    n = *src++ - '1';		/* get parameter number */
	    if (n > 9)			/* adjust for alpha range */
		n = 9 + ((n + '1' - 'A') & 037);	/* allow A-Z and a-z */
	    if (p = pa[n])  {		/* no substitution if no pointer */
		l = la ? la[n] : strlen (p);	/* get param length */
		strncpy (dst, p, l);		/* copy parameter */
		dst += l;			/* bump store pointer */
	    }
	}
}
