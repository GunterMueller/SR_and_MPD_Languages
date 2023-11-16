/*  csloop.c -- context switch loop
 *
 *  This program executes a large number of context switches and
 *  and reports the amount of time required.  The default is to
 *  loop for twenty seconds.
 *
 *  Usage:  csloop [nseconds]
 */


#include <stdio.h>
#include <time.h>
#include "../arch.h"

#define DEFTIME 20		/* default time limit, in seconds */

#define STACK_SIZE 2000		/* stack size in double words */

double d0[STACK_SIZE];  char *stk0 = (char *) d0;
double d1[STACK_SIZE];  char *stk1 = (char *) d1;
double d2[STACK_SIZE];  char *stk2 = (char *) d2;
double d3[STACK_SIZE];  char *stk3 = (char *) d3;
double d4[STACK_SIZE];  char *stk4 = (char *) d4;

int nseconds = DEFTIME;		/* approximate time limit */
long swlimit;			/* context switch limit */
long swdone;			/* context switch count */

static void super (), con ();
static double timeloop ();



/*  main program -- set up four contexts and execute supervisor  */

main (argc, argv)
int argc;
char *argv[];
{
    if (argc > 1)
	nseconds = atoi (argv[1]);
    if (nseconds < 1 | nseconds > 1000) 
	{ fprintf (stderr, "usage: %s [nseconds]\n", argv[0]); exit (1); }
    printf ("running context switch loop, arch = %s\n", ARCH);
    mpd_build_context (super, stk0, sizeof (d0), 0x01, 0x02, 0x03, 0x04);
    mpd_build_context (con, stk1, sizeof (d1), stk2, stk1, 0x13, 0x14);
    mpd_build_context (con, stk2, sizeof (d2), stk3, stk2, 0x23, 0x24);
    mpd_build_context (con, stk3, sizeof (d3), stk4, stk3, 0x33, 0x34);
    mpd_build_context (con, stk4, sizeof (d4), stk1, stk4, 0x43, 0x44);
    mpd_chg_context (stk0, (char *) NULL);
    /*NOTREACHED*/
}



/*  supervisor */

static void super ()
{
    double t;
    long n;

    /* start small and figure out how many times to loop */
    n = 1000;
    while ((t = timeloop (n)) < (0.05 * nseconds))
	n *= 3;
    n *= nseconds / t;

    /* now do the real timing and report the results */
    t = timeloop (n);
    printf ("%.3f usec per context switch: %d in %.3f seconds\n",
	1.e6 * t / n, n, t);
    exit (0);
}



/*  timeloop (n) -- return time in seconds used to execute n switches  */

static double timeloop (n)
long n;
{
    clock_t t1, t2;

    swlimit = n;
    swdone = 1;
    t1 = clock ();
    mpd_chg_context (stk1, stk0);
    t2 = clock ();
    if (t1 == -1 || t2 == -1)
	{ fprintf (stderr, "clock not available\n"); exit (1); }
    return (t2 - t1) / (double) CLOCKS_PER_SEC;
}



/* this same function is executed in all four contexts */

static void con (next, curr)
char *next, *curr;
{
    for (;;) {
	while (++swdone < swlimit)
	    mpd_chg_context (next, curr);
	mpd_chg_context (stk0, curr);
    }
}



/*  error handling  */

mpd_stk_overflow ()  { fprintf (stderr, "stack overflow\n");  exit (1); }
mpd_stk_underflow () { fprintf (stderr, "stack underflow\n"); exit (1); }
mpd_stk_corrupted () { fprintf (stderr, "stack corrupted\n"); exit (1); }
