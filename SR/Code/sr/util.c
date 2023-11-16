/*  util.c -- general utility routines shared by multiple components  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>	/* needed by HP-UX */
#include <fcntl.h>	/* needed by HP-UX */
#include "gen.h"
#include "util.h"
#include "paths.h"
#include "config.h"



/******************************  error exits  ******************************/



char *xprefix = 0;	/* set this to prefix messages with arg0 or whatever */



/*  mexit(s) -- exit immediately, with message  */

void
mexit (s)
char *s;
{
    fflush (stdout);
    if (xprefix)
	fprintf (stderr, "%s: ", xprefix);
    fprintf (stderr, "%s\n", s);
    exit (1);
}



/*  pexit(s) -- call perror(s) and exit immediately  */

void
pexit (s)
char *s;
{
    fflush (stdout);
    if (xprefix)
	fprintf (stderr, "%s: ", xprefix);
    perror (s);
    exit (1);
}



/***************************  memory allocation  ***************************/



/*  alloc(n) -- allocate n bytes, with success guaranteed  */

char *
alloc (n)
int n;
{
    char *s;

    s = malloc ((unsigned) n);
    if (!s)
	mexit ("out of memory");
    return s;
}



/*  salloc(s) -- allocate and initialize string, with success guaranteed  */

char *
salloc (s)
char *s;
{
    return strcpy (alloc (strlen (s) + 1), s);
}



/****************************  string checking  ****************************/



/*  strtail(s,t) -- return pointer to tail substring t of s, or 0 if no match */

char *
strtail (s, t)
char *s, *t;
{
    int ls = strlen (s);
    int lt = strlen (t);
    if (ls < lt)
	return 0;
    s = s + ls - lt;
    if (strcmp (s, t) == 0)
	return s;
    else
	return 0;
}



/****************************  file handling  ****************************/



/*  mustopen(filename,type) -- fopen() with guaranteed success */

FILE *
mustopen (filename, type)
char *filename, *type;
{
    FILE *fp;

    fp = fopen (filename, type);
    if (!fp)
	pexit (filename);
    return fp;
}



/*  modtime(filename) -- return modification time of a file, or 0 if no file  */

long
modtime (filename)
char *filename;
{
    struct stat status;

    if (stat (filename, &status) != 0)
	return 0;
    else
	return (long) status.st_mtime;
}



/****************************  exec assistance  ****************************/



int trcexec;	/* if set, trace execs on standard error output */



/*  spawn(argv,dir) -- vfork, chdir, exec, wait  */

int
spawn (argv, dir)
char *argv[];
char *dir;
{
    int status;
    int pid;

    fflush (stdout);
    fflush (stderr);
    pid = vfork ();
    if (pid < 0)
	pexit ("can't fork");
    else if (pid == 0) {
	/* we're the child process */
	if (dir)		/* change current directory if specified */
	    if (chdir (dir) != 0)
		pexit (dir);
	doexec (argv);		/* exec the program */
	perror (argv[0]);	/* diagnose error if exec failed */
	exit (1);
    }
    wait (&status);
    return status;
}



/*  doexec(argv) -- trace if requested, exec program, abort on error  */

void
doexec (argv)
char *argv[];
{
    char **argp;

    fflush (stdout);
    if (trcexec)  {
	fprintf (stderr, "+");
	for (argp = argv; *argp; argp++)
	    fprintf (stderr, " %s", *argp);
	fprintf (stderr, "\n");
    }
    fflush (stderr);
    execv (argv[0], argv);
    pexit (argv[0]);
}



/****************************  path searching  ****************************/



static char *searchpath;

/* setpath(ifdir,expt) -- set path for future searches  */

void
setpath (ifdir, expt)
char *ifdir;
int expt;
{
    char *evpath;

    evpath = getenv ("SR_PATH");
    if (evpath == NULL)
	evpath = "";
    searchpath = alloc (strlen (evpath) + strlen (ifdir) + 100);
    if (expt)
	sprintf (searchpath, "%s:.:%s:%s/links", evpath, ifdir, SRSRC);
    else
	sprintf (searchpath, "%s:.:%s:%s", evpath, ifdir, SRLIB);
}



/*  pathopen(filename,suffix,buf) -- search path and open file for reading
 *
 *  If the file is found, its name is left in buf.
 */

FILE *
pathopen (filename, suffix, buf)
char *filename, *suffix, *buf;
{
    FILE *fp;
    char *p, *q;

    if (strchr(filename, '/')) {		/* if absolute path */
	sprintf(buf, "%s%s", filename, suffix);
	return fopen(buf, "r");
	}

    p = searchpath;
    for (;;) {
	while (*p == ':')
	    p++;
	if (*p == '\0')
	    break;
	while (p[0] == '.' && p[1] == '/' && p[2] != ':' && p[2] != '\0')
	    p += 2;
	q = buf;
	while (*p != '\0' && *p != ':')
	    *q++ = *p++;
	sprintf (q, "/%s%s", filename, suffix);
	if ((fp = fopen (buf, "r")) != NULL)
	    return fp;
    }
    sprintf (buf, "%s%s", filename, suffix);
    return NULL;
}
