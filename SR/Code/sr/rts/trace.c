/*  trace.c -- invocation trace routines  */

#include "rts.h"
#include <fcntl.h>

#ifdef __PARAGON__
#include <nx.h>
#endif

static char *trace_locn ();



/*
 *  sr_init_trace (arg)
 *
 *  If arg is non-null, it a string representation of an existing fd to use.
 */
void
sr_init_trace (arg)
char *arg;
{
    char *fname;
    
#ifndef __PARAGON__		/* on Paragon, every VM opens trace file */
    if (arg != NULL) {				/* if not first vm */
	sscanf (arg, "%d", &sr_trc_fd);
	sr_trc_flag = (sr_trc_fd > 0);
	return;
    }
#endif

    fname = getenv (ENV_TRACE);			/* if no file name */
    if (fname == NULL || strlen (fname) == 0)
	return;

    sr_trc_flag = 1;
    if (strcmp (fname, "stdout") == 0)
	sr_trc_fd = STDOUT;
    else if (strcmp (fname, "stderr") == 0)
	sr_trc_fd = STDERR;				
    else {					/* if named file */
#ifndef __PARAGON__
	sr_trc_fd = open (fname, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0666);
#else
	sr_trc_fd = open (fname, O_WRONLY | O_CREAT | O_APPEND, 0666); 
#endif
	if (sr_trc_fd < 0) {
	    sr_trc_flag = 0;
	    RTS_WARN ("cannot open trace file");
	}
    }
}



/*
 *  sr_trace (action, locn, comment)
 * 
 *  General procedure for invocation tracing.
 *  Called by all RTS procedures that need to trace the invocation.
 */
int
sr_trace (action, locn, addr)
char *action, *locn;
Ptr addr;
{
    char obuf[250];
    char *p = obuf;

    if (*(CUR_PROC->pname) != '[') {		/* if user process */
	/* print location, proc name and proc id */
	p = trace_locn (obuf, locn);		/* format location */
	if (sr_exec_up)				/* if more than one vm */
	    sprintf (p, "vm(%d).%-14s  %-8s  %-8lX  %-8lX\n", 
		sr_my_vm, CUR_PROC->pname, action, (long)CUR_PROC, (long)addr);
	else
	    sprintf (p, "%-20s  %-8s  %-8lX  %-8lX\n", 
		CUR_PROC->pname, action, (long) CUR_PROC, (long) addr);
	write (sr_trc_fd, obuf, strlen (obuf));
    } 
    return 0;
}



/*
 *  trace_locn (buf, locn)
 *
 *  Translate a "locn" pointer into a text string (with a different format
 *  from that produced by sr_fmt_locn).  Return pointer to *end* of string.
 */
static char *
trace_locn (buf, locn)
char *buf, *locn;
{
    int lno;
    char *e;

    if (!locn)
	strcpy (buf, "(unknown location)  ");
    else {
	lno = 1;
	while (*--locn != '\002')
	    lno++;
	sprintf (buf, "%s, %d  ", locn + 1, lno);
    }
    e = buf + strlen (buf);
    while (e < buf + 20)
	*e++ = ' ';
    *e = '\0';
    return e;
}
