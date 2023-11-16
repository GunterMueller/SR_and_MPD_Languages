/*  util.h - definitions for use with util.c  */

#include <stdio.h>


extern char *xprefix;		/* error exit message prefix */
extern void mexit(), pexit();	/* exit with error message */

extern char *alloc(), *salloc();/* memory allocation with guaranteed success */

extern char *strtail();		/* tailstring comparison */

extern long modtime();		/* modification time of a file */

extern void setpath();		/* initialize for path search */
extern FILE *pathopen();	/* file open with path search */
extern FILE *mustopen();	/* file open with guaranteed success */

extern int trcexec;		/* trace exec calls? */
extern spawn();			/* fork and exec */
extern void doexec();		/* exec in place */
