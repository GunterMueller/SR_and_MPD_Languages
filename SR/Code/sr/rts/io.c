/*  io.c -- input and output routines
 *
 *  Warning:  These routines depend to some extent on the internals of the
 *  standard I/O library.  They have, however, proven surprisingly portable.
 *  The code could be even more portable if we didn't want to make SR files
 *  compatible with fprintf etc. for use by externals.
 */

#include <ctype.h>
#include <stdarg.h>
#include "rts.h"

#define NOTHING			/* for use as null macro arg under ANSI C */

int sr_inchar ();
static int rdbool (), rdline (), rdtok (), get ();
static int outchar (), check_error ();
static void flushout (), wready ();

#ifdef MULTI_SR
static int fdnum ();
#endif


static File fp_table [LAST_SHARED_FD + 1];



/*
 *  Initialize I/O.
 */
void
sr_init_io ()
{
    int i;

    for (i = 0; i <= LAST_SHARED_FD; i++)
	fp_table[i] = NULL;

    if (SHARED_FD (0)) {
	fp_table[0] = stdin;
	sr_fd_lock[0].name = "stdin_mutex";
    }
    if (SHARED_FD (1)) {
	fp_table[1] = stdout;
	sr_fd_lock[1].name = "stdout_mutex";
    }
    if (SHARED_FD (2)) {
	fp_table[2] = stderr;
	sr_fd_lock[2].name = "stderr_mutex";
    }
}



/*
 *  Open a file.
 *  Return descriptor, or a descriptor for the null file if open fails.
 */
File
sr_open (fname, mode)
String *fname;
int mode;
{
    File fp;
    int fd;
    static char *flags [] = { "r", "w", "r+" };

    sr_check_stk (CUR_STACK);
    BEGIN_IO (NULL);

    * (DATA (fname) + fname->length) = '\0';
    if ((fp = fopen (DATA (fname), flags [mode])) == NULL) {
	fp = (File) NULL_FILE;
    } else {
	fd = fileno (fp);
	if (SHARED_FD (fd))		/* update table if file is shared */
	    fp_table[fd] = fp;
#ifdef SHARED_FILE_OBJS
	else {
	    /* can't handle this many open files */
	    fclose (fp);
	    fp = (File) NULL_FILE;	/* indicate failure on return */
	}
#endif
    }

    END_IO (NULL);
    return fp;
}



/*
 *  Move the file pointer.
 */
int
sr_seek (locn, fp, seektype, offset)
char *locn;
File fp;
int seektype;
int offset;
{
    int posn;

    sr_check_stk (CUR_STACK);
    BEGIN_IO (fp);
    CHECK_FILE (locn, fp, 0);
    if (fseek (fp, (long) offset, seektype) != 0)
	check_error (locn, fp);
    posn = ftell (fp);
    END_IO (fp);
    return posn;
}



/*
 *  Determine the position of the file pointer.
 */
int
sr_where (locn, fp)
char *locn;
File fp;
{
    long posn;

    sr_check_stk (CUR_STACK);
    BEGIN_IO (fp);
    CHECK_FILE (locn, fp, 0);
    if ((posn = ftell (fp)) < 0L)
	check_error (locn, fp);
    END_IO (fp);
    return posn;
}



/*
 *  Flush a file's buffers.  The file remains open.
 */
void
sr_flush (locn, fp)
char *locn;
File fp;
{
    sr_check_stk (CUR_STACK);
    BEGIN_IO (fp);
    CHECK_FILE (locn, fp, NOTHING);
    flushout (locn, fp);
    END_IO (fp);
}



/*
 *  Close a file.
 */
void
sr_close (locn, fp)
char *locn;
File fp;
{
    sr_check_stk (CUR_STACK);
    BEGIN_IO (fp);
    CHECK_FILE (locn, fp, NOTHING);

#ifdef __linux__
    if ((fp->_flags & (_S_NO_READS & _S_NO_WRITES)) == 0)
	flushout (locn, fp);
#else
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined (__OpenBSD__)
    if (fp->_flags & __SWR)
	flushout (locn, fp);
#else
    if (fp->_flag & _IOWRT)
	flushout (locn, fp);
#endif
#endif

    if (fclose (fp) == EOF)
	check_error (locn, fp);

    /*  We must call END_IO before forgetting the fileno association, since
     *  this would goof up END_IO and cause it to not unlock a fd lock.
     */
    END_IO (fp);

#ifdef MULTI_SR
    if (SHARED_FD (fdnum (fp)))
	fp_table [fdnum (fp)] = 0;		/* forget fileno association */
#endif
}



/*
 *  Remove a file by name.
 */
Bool
sr_remove (fname)
String *fname;
{
    Bool rtn;

    sr_check_stk (CUR_STACK);
    * (DATA (fname) + fname->length) = '\0';
    BEGIN_IO (NULL);
    rtn = (unlink (DATA (fname)) >= 0);
    END_IO (NULL);
    return rtn;
}



/*
 *  sr_read (locn, fp, argtypes, &arg1, ...) -- read zero or more variables.
 */
int
sr_read (char *locn, File fp, char *argt, ...)
{
    va_list ap;
    Array *a;
    String *s;
    Real *rp;
    Ptr *pp;
    Bool *bp;
    int *ip;
    long p;

    char x, c, t, buf[100], *cp;
    int nread, len, i, n, ret;
    double d;

    sr_check_stk (CUR_STACK);
    BEGIN_IO (fp);
    CHECK_FILE (locn, fp, EOF);

    va_start (ap, argt);
    nread = ret = 0;
    while (t = *argt++)  {
	switch (t) {
	    case 'b':	/* boolean argument */
		bp = va_arg (ap, Bool *);
		ret = rdbool (locn, fp, bp);
		break;
	    case 'c':
		cp = va_arg (ap, char *);
		if (rdtok (locn, fp, buf, sizeof (buf) - 1) < 0)
		    ret = EOF;
		else {
		    ret = 0;
		    *cp = buf[0];
		}
		break;
	    case 'i':	/* integer argument */
	    case 'e':	/* enum argument */
		ip = va_arg (ap, int *);
		n = rdtok (locn, fp, buf, sizeof (buf) - 1);
		if (n < 0)
		    ret = EOF;
		else {
		    c = buf[n] = '\0';
		    ret = !sr_cvint (buf, &i);
		    if (ret == 0)
			*ip = i;	/* store only if valid */
		}
		break;
	    case 'r':
		rp = va_arg (ap, Real *);
		n = rdtok (locn, fp, buf, sizeof (buf) - 1);
		if (n < 0)
		    ret = EOF;
		else {
		    c = buf[n] = '\0';
		    n = sscanf (buf, "%lf%c", &d, &c);
		    ret = (n != 1) || (c != '\0');
		    if (ret == 0)
			*rp = d;	/* store only if valid */
		}
		break;
	    case 'p':
		pp = va_arg (ap, Ptr *);
		n = rdtok (locn, fp, buf, sizeof (buf) - 1);
		if (n < 0)
		    ret = EOF;
		else {
		    c = buf[n] = '\0';
		    if (strcmp (buf, "==null==") == 0) {
			*pp = NULL;
			ret = 0;
		    } else {
			n = sscanf (buf, "%lx%c%c", &p, &x, &c);
			ret = ! (n == 1 || (n == 2 && (x == 'x' || x == 'X')));
			if (ret == 0)
			    *pp = (Ptr) p;	/* store only if valid */
		    }
		}
		break;
	    case 's':	/* string argument */
		s = (String *) va_arg (ap, String *);
		n = rdline (locn, fp, DATA (s), MAXLENGTH (s));
		if (n < 0)
		    ret = EOF;
		else
		    s->length = n;
		break;
	    case 'a':	/* char array argument */
		a = (Array *) va_arg (ap, Array *);
		len = UB (a, 0) - LB (a, 0) + 1;
		n = rdline (locn, fp, ADATA (a), len);
		if (n < 0)
		    ret = EOF;
		else
		    while (n < len)
			ADATA (a) [n++] = ' ';
		break;
	    default:
		ABORT_IO (fp, locn, "bad read format");
	}

	if (ret != 0)  {
	    if (nread > 0)
		IO_RETURN (fp, nread);
	    else if (ret == EOF)
		IO_RETURN (fp, EOF);
	    else
		IO_RETURN (fp, 0);
	}
	nread++;
    }
    va_end (ap);
    IO_RETURN (fp, nread);
}


/*
 *  Read a Boolean literal into given address from the named file.
 *  The accepted values are "true" and "false".
 *  Return 1 if bad, 0 if okay, EOF for EOF.
 */
static int
rdbool (locn, fp, pbool)
char *locn;
File fp;
Bool *pbool;
{
    int n;
    char buf[20];

    n = rdtok (locn, fp, buf, 6);
    if (n == EOF)
	return EOF;
    buf[n] = '\0';
    return !sr_cvbool (buf, pbool);
}



/*
 *  Read a line to buffer s of size n; return number of chars stuffed, or EOF.
 */
static int
rdline (locn, fp, s, n)
char *locn;
File fp;
char *s;
int n;
{
    register int c, i;

    i = 0;
    for (;;) {
	c = INCH (locn, fp);

	if (c == EOF)
	    return (i != 0) ? i : EOF;
	if (c == '\n')
	    return i;
	if (++i > n)  {
	    ungetc (c, fp);
	    return n;
	}
	*s++ = c;
    }
}



/*
 *  Read a token to buffer s of size n; return number of chars stuffed, or EOF.
 */
static int
rdtok (locn, fp, s, n)
char *locn;
File fp;
char *s;
int n;
{
    int c, i;

    while (isspace (c = INCH (locn, fp)))
	;
    if (c == EOF)
	return EOF;
    i = 0;
    while ((c != EOF) && (!isspace (c)))  {
	if (i < n)
	    s[i++] = c;
	c = INCH (locn, fp);
    }
    while (c != '\n' && isspace (c))
	c = INCH (locn, fp);
    if (c != '\n')
	ungetc (c, fp);
    return i;
}



/*
 *  sr_printf (locn, fp, sp, format, argtypes, arg1...)
 *  Write zero or more arguments to string or file according to given format.
 *  Either fp (file pointer) or sp (string pointer) is null.
 *
 *  Each conversion is limited to 509 characters as in ANSI C.
 *
 *  Besides the externally documented formats, %r (raw) is provided for use by
 *  the write (), writes (), and put () builtins.  The 509 character limit does
 *  not apply;  NUL characters are ignored;  any precision etc. in the spec
 *  are also ignored.
 */
void
sr_printf (char *locn, File fp, String *sp, String *str, Ptr argt, ...)
{
    va_list ap;
    char *fmt, a, c, *p, *b, *s;
    double v;
    char xbuf[20], fbuf[512], dbuf[512], obuf[512];
    int n;
    int ssize;
    Array *arr;

    sr_check_stk (CUR_STACK);
    va_start (ap, argt);

    if (sp != NULL) {
	*DATA (sp) = '\0';
	sp->length = 0;
	ssize = MAXLENGTH (sp);
	PRIV (io_handoff) = 0;
    } else {
	BEGIN_IO (fp);
	CHECK_FILE (locn, fp, NOTHING);
    }

    * (DATA (str) + str->length) = '\0';
    fmt = DATA (str);


    while ((c = *fmt++) != '\0') {
	if (c == '%') {
	    b = fbuf;
	    *b++ = '%';
	    while ((c = *fmt++) != '\0' && !isalpha (c) && c != '%')
		*b++ = c;
	    if (!c)
		ABORT_IO (fp, locn,
		    "unterminated conversion spec (%...) in format");
	    b[0] = c;				/* leave b on conversion char */
	    b[1] = '\0';			/* add terminating NUL */
	    if (c == '%') {
		if (sp != NULL) {
		    if (ssize > sp->length)
			* (DATA (sp) + sp->length++) = '%';
		    else
			ABORT_IO (fp, locn, "result string too small");
		}
		else
		    OUCH (locn, fp, '%');		/* spec was `%...%' */
		continue;
	    }

	    a = *argt++;
	    if (!a)
		ABORT_IO (fp, locn, "insufficient argument list for format");

	    switch (c) {
		case 'q':
		case 'i':
		    if (c == 'q')
			*b = 'o';		/* turn %q into %o */
		    else /* c == 'i' */
			*b = 'd';		/* turn %i into %d */
		    /*FALLTHROUGH*/
		case 'd':
		case 'o':
		case 'x':
		case 'X':
		    if (a != 'i' && a != 'e')
			ABORT_IO (fp, locn, "format calls for an int argument");
		    sprintf (obuf, fbuf, va_arg (ap, int));
		    break;
		case 'b':
		    if (a != 'b')
			ABORT_IO (fp, locn, "format calls for a bool argument");
		    *b = 's';
		    sprintf (obuf, fbuf, va_arg (ap, int) ? "true" : "false");
		    break;
		case 'B':
		    if (a != 'b')
			ABORT_IO (fp, locn, "format calls for a bool argument");
		    *b = 's';
		    sprintf (obuf, fbuf, va_arg (ap, int) ? "TRUE" : "FALSE");
		    break;
		case 'c':
		    if (a != 'c')
			ABORT_IO (fp, locn, "format calls for a char argument");
		    c = va_arg (ap, int);
		    if (sp == NULL && b == fbuf + 1) {	/* if pure "%c" */
			/*
			 * special short circuit path to allow write ('\0')
			 */
			OUCH (locn, fp, c);			
			continue;
		    }
		    sprintf (obuf, fbuf, c);
		    break;
		case 'r':
		    if (a == 's') {
			str = va_arg (ap, String *);
			s = DATA (str);
			n = str->length;
		    } else if (a == 'a') {
			arr = va_arg (ap, Array *);
			s = ADATA (arr);
			n = UB (arr, 0) - LB (arr, 0) + 1;
		    } else {
			ABORT_IO (fp, locn, "string argument expected");
		    }
		    if (n < 0)
			ABORT_IO (fp, locn, "illegal length for write");
		    if (sp != NULL) {
			if (ssize - sp->length >= n) {
			    p = DATA (sp) + sp->length;
			    sp->length += n;
			    while (n--)
				*p++ = *s++;
			}
			else
			    ABORT_IO (fp, locn, "result string too small");
		    }
		    else
			while (n--)
			    OUCH (locn, fp, *s++);
		    continue;
		case 's':
		    if (a == 's') {
			str = va_arg (ap, String *);
			s = DATA (str);
			n = str->length;
		    } else if (a == 'a') {
			arr = va_arg (ap, Array *);
			n = UB (arr, 0) - LB (arr, 0) + 1;
			memcpy (s = dbuf, ADATA (arr), n);
		    } else {
			ABORT_IO(fp,locn,"format calls for a string argument");
		    }
		    if (n < 0 || n > 509)
			ABORT_IO (fp, locn, "illegal string length for %s");
		    s[n] = '\0';
		    sprintf (obuf, fbuf, s);
		    break;
		case 'f':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
		    if (a == 'r')
			v = va_arg (ap, double);
		    else if (a == 'i')
			v = va_arg (ap, int);
		    else
			ABORT_IO (fp, locn,"format calls for a real argument");
		    sprintf (obuf, fbuf, v);
		    break;
		case 'p':
		    if (a != 'p')
			ABORT_IO(fp,locn,"format calls for a pointer argument");
		    p = va_arg (ap, Ptr);
		    if (!p)
			strcpy (xbuf, "==null==");
		    else
			sprintf (xbuf, "%08lX", (long) p);
		    *b = 's';
		    sprintf (obuf, fbuf, xbuf);
		    break;
		case 'h':
		case 'l':
		case 'L':
		    ABORT_IO (fp, locn,
			"illegal size modifier (h/l/L) in format spec");
		default:
		    ABORT_IO (fp, locn,
			"illegal specification character in format");
	    }
	    s = obuf;
	    if (sp != NULL) {
		while ((c = *s++) != '\0') {
		    if (ssize > sp->length)
			* (DATA (sp) + sp->length++) = c;
		    else
			ABORT_IO (fp, locn, "result string too small");
		}
	    }
	    else
		while ((c = *s++) != '\0')	/* now write buffer to file */
		    OUCH (locn, fp, c);
	} else
	    if (sp != NULL) {
		if (ssize > sp->length)
		    * (DATA (sp) + sp->length++) = c;
		else
		    ABORT_IO (fp, locn, "result string too small");
	    }
	    else
		OUCH (locn, fp, c);
    }
    va_end (ap);
    if (*argt)
	ABORT_IO (fp, locn, "too many arguments for format");

    if (sp == NULL) {
	flushout (locn, fp);			/* flush after every write */
	END_IO (fp);
    }
}



/*
 *  Read string value for "get(s)".
 */
int
sr_get_string (locn, fp, s)
char *locn;
File fp;
String *s;
{
    int n;
    n = get (locn, fp, DATA (s), MAXLENGTH (s));
    if (n != EOF)
	s->length  = n;
    return n;
}



/*
 *  Read character array for "get(a)".
 */
int
sr_get_carray (locn, fp, a)
char *locn;
File fp;
Array *a;
{
    return get (locn, fp, ADATA (a), UB (a, 0) - LB (a, 0) + 1);
}



/*
 *  Read up to "len" characters from the specified file.
 */
static int
get (locn, fp, str, len)
char *locn;
File fp;
char *str;
int len;
{
    int c;
    int count = 0;

    sr_check_stk (CUR_STACK);

    BEGIN_IO (fp);
    CHECK_FILE (locn, fp, EOF);
    for (; len > 0; len--, count++) {
	if ((c = INCH (locn, fp)) == EOF) {
	    if (count == 0)
		IO_RETURN (fp, EOF);
	    break;
	}
	*str++ = c;
    }
    IO_RETURN (fp, count);
}



/*
 *  Input a character without blocking other processes.
 *  (Called by INCH macro when the buffer is empty.)
 */
int
sr_inchar (locn, fp)
char *locn;
File fp;
{
    int c, rtn;

#ifndef __linux__
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined (__OpenBSD__)
    fp->_r++;			/* reset count clobbered by INCH macro */
#else
    fp->_cnt++;			/* reset count clobbered by INCH macro */
#endif
#endif

    wready (fp, INPUT);		/* wait for input file ready */
    c = getc (fp);		/* now that we know it won't wait... */
    if (c < 0)
	rtn = check_error (locn, fp);
    else
	rtn =  c;
    return rtn;
}



/*
 *  Output a character without blocking other processes.
 *  (Called by OUCH macro when the count is zero.)
 *
 *  For some buffering schemes, the count is always zero, so check the buffer
 *  contents;  if there's already something there, we checked before and
 *  nothing's been written since, so we need not check again.
 *
 *  NOT FOOLPROOF; depends on how stdio works.
 */
static int
outchar (locn, fp, c)
char *locn;
File fp;
char c;
{

#ifdef __linux__
    if (fp->_pptr == fp->_pbase)	/* only need to select () once */
	wready (fp, OUTPUT);		/* wait for output file ready */
#else
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined (__OpenBSD__)
    fp->_w++;				/* reset count clobbered by OUCH macro*/
    if (fp->_p == fp->_bf._base)	/* only need to select () once */
	wready (fp, OUTPUT);		/* wait for output file ready */
#else
    fp->_cnt++;				/* reset count clobbered by OUCH macro*/
    if (fp->_ptr == fp->_base)		/* only need to select () once */
	wready (fp, OUTPUT);		/* wait for output file ready */
#endif
#endif

    if (putc (c, fp) == EOF)		/* now we know this is safe */
	check_error (locn, fp);		/* handle error */
    return c;
}



/*
 *  Flush an output file without blocking other processes.
 *  Assumes any necessary locks are already held.
 *
 *  NOT FOOLPROOF: has been seen to fail on Vax, work on Sun.
 */
static void
flushout (locn, fp)
char *locn;
File fp;
{

#ifdef __linux__
    if (fp->_pptr == fp->_pbase)	/* if buffer empty, return */
	return;
#else
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined (__OpenBSD__)
    if (fp->_p == fp->_bf._base)	/* if buffer empty, return */
	return;
#else
    if (fp->_ptr == fp->_base)		/* if buffer empty, return */
	return;
#endif
#endif

    wready (fp, OUTPUT);		/* wait until ready for output */
    if (fflush (fp) == EOF)		/* flush buffer */
	check_error (locn, fp);		/* handle error */
}



/*
 *  Wait until a file is ready for input or output.
 */
static void
wready (fp, inout)
File fp;
enum io_type inout;
{
    int fd;
    fd_set fdset;
    static fd_set zeroset;

    if (inout == OUTPUT && !sr_async_flag)
	return;			/* don't wait -- want synchronous output */

    fdset = zeroset;

#ifdef MULTI_SR
    fd = fdnum (fp);	/*  if fd is not one of our shared descriptors, then
			 *  below we will have the IO Server do the work */
    if (SHARED_FD (fd)) {

	FD_SET (fd, &fdset);
	if (sr_num_job_servers > 1)
	    UNLOCK (sr_fd_lock[fd], "wready");	/* begin_io didn't lock if 1 */

	sr_iowait (&fdset, &fdset, inout);

	if (sr_num_job_servers > 1)
	    LOCK (sr_fd_lock[fd], "wready");
    } else {
#else
	{
#endif /* MULTI_SR */

	/* this must really be run on the IO Server -- the fileno stuff */
	BEGIN_IO (NULL);
	fd = fileno (fp);
	fdset = zeroset;
	FD_SET (fd, &fdset);
	END_IO (NULL);
	sr_iowait (&fdset, &fdset, inout);
    }
}



/*
 *  Check for I/O errors and abort if there were any.
 *  If not, clear possible EOF condition.  Return EOF if so, or 0.
 *  Note: this is always called inside a BEGIN_IO call, so we don't
 *  need to do it.
 */
static int
check_error (locn, fp)
char *locn;
File fp;
{
    int rtn = 0;

    if (ferror (fp))  {
	perror ("SR I/O");
	ABORT_IO (fp, locn, "I/O error");
    }
    if (feof (fp))  {
	clearerr (fp);		/* clear EOF */
	rtn = EOF;
    }
    return rtn;
}



/*  The rest of this file contains routines used only in MultiSR.  */

#ifdef MULTI_SR



/*
 *  sr_begin_io and sr_end_io are targets of the macros BEGIN_IO and END_IO if
 *  files and file buffers are not sharable among processes.  Begin and end are
 *  not quite symmetric: begin could be called as the IO server from scheduler,
 *  if there were no waiting IO jobs, so in this case we should just return.
 *  However, end must be called from the IO server.
 */
int
sr_begin_io (fp)
File fp;
{
    int fd;

    if (sr_num_job_servers == 1)
	return 0;			/* nothing to do */

    /*  We don't need to do anything with a null or noop file.  A
     *  null file will be handled by our caller, so in both cases
     *  we can return here */
    if (fp == (File) NOOP_FILE || fp == (File) NULL_FILE)
	return 0;

    /*  if it is a shared file object, then the IO Server and other
     *  job servers must get the lock.  If it is not, then other job
     *  servers must hand it off to the IO server.  When the IO server
     *  gets it it doesn't have to grab that lock */

    fd = fdnum (fp);
    if (SHARED_FD (fd)) {
	/* we can go ahead and grab the lock (if not already held) */
	/*  note that we do not set the status to DOING_IO if we don't
	 * ``hand off'' this proc to the IO SERVER (see sr_end_io) */
	LOCK (sr_fd_lock[fd], "sr_begin_io");
	return 0;
    } else if (I_AM_IOSERVER) 
	return 0;			/* nothing to do */
#ifdef SHARED_FILE_OBJS
    else if (fp == 0)
	return 0;			/* nothing to do */
#endif

    LOCK_QUEUE ("sr_begin_io");		/* will be released by scheduler */
    if (CUR_PROC->status == BLOCKED)
	sr_malf ("sr_begin_io called with BLOCKED cur_proc");
    CUR_PROC->status = DOING_IO; /* for sr_scheduler: doesn't resched */
    sr_enqueue (&sr_io_list, CUR_PROC);
    sr_scheduler ();
    return 0;
}



int
sr_end_io (fp)
File fp;
{
    int fd;

    if (sr_num_job_servers == 1)
	return 0;			/* nothing to do */


    /*  We don't need to do anything with a null or noop file.  A
     *  null file will be handled by our caller, so in both cases
     *  we can return here */
    if (fp == (File) NOOP_FILE || fp == (File) NULL_FILE)
	return 0;

    fd = fdnum (fp);

    if (CUR_PROC->status != DOING_IO) {
	/* sr_begin_io did not hand this off between job servers, so
	 * we won't either. */
	if (SHARED_FD (fd))
	    UNLOCK (sr_fd_lock[fd], "sr_end_io");
	return 0;			/* nothing to do ! */
    }

    LOCK_QUEUE ("sr_end_io");		/* will be released by scheduler */
    sr_reschedule (CUR_PROC);
    sr_scheduler ();
    return 0;
}



/*
 *  Look up the file descriptor for fp.
 *
 *  We only track files that different job servers are allowed to share.
 *  If fp points to a non-sharable file object, a negative number is returned
 *  (in which case the caller will have to have the IO activity done by
 *  the IO server).
 *
 *  If file objects are not not shared in general, then FIRST_SHARED_FD
 *  and LAST_SHARED_FD can still give a range of files that *are* shared.
 *
 *  We assume atomic access to each entry, so there is no locking of the table.
 */

static int
fdnum (fp)
File fp;
{
    int i;

    if (fp != NULL)
	for (i = FIRST_SHARED_FD; i <= LAST_SHARED_FD; i++)
	    if (fp_table[i] == fp)
		return i;
    return -1;
}
#endif /* MULTI_SR */
