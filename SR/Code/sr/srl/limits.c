/*  limits.c -- runtime limit handling  */


#include <ctype.h>
#include "globals.h"
#include "funcs.h"
#include "../config.h"



int async_flag = 0;			/* asynchronous output? */
int max_co_stmts = MAX_CO_STMTS;	/* limit on active "co" statements */
int max_classes = MAX_CLASSES;		/* limit on "in" operation classes */
int max_loops = MAX_LOOPS;		/* limit on loops between cswitches */
int max_operations = MAX_OPERATIONS;	/* limit on active operations */
int max_processes = MAX_PROCESSES;	/* limit to number of processes */
int max_rmt_reqs = MAX_RMT_REQS;	/* limit on pending remote requests */
int max_resources = MAX_RESOURCES;	/* limit on active resources */
int max_semaphores = MAX_SEMAPHORES;	/* limit on number of semaphores */
int stack_size = STACK_SIZE;		/* size of a process stack */

static struct lim { char c; int *v; char *s; } limits[] = {
    'C', &max_co_stmts,   "maximum number of active `co' statements",
    'L', &max_loops,      "maximum number of loops before context switch",
    'N', &max_classes,    "maximum number of `in' operation classes",
    'O', &max_operations, "maximum number of active operations",
    'P', &max_processes,  "maximum number of processes",
    'Q', &max_rmt_reqs,   "maximum number of pending remote requests",
    'R', &max_resources,  "maximum number of active resources",
    'V', &max_semaphores, "maximum number of semaphores",
    'S', &stack_size,     "size of a process stack",
    0, 0, 0};



/*
 *  Set the value of the limit associated with option character c.
 *  Abort with a diagnostic if the integer is illegal.
 */
void
setlimit (c, s)
int c;
char *s;
{
    int v;
    char *p, temp[100];
    struct lim *l;

    v = 0;
    p = s;
    while (isdigit (*p))
	v = 10 * v + *p++ - '0';
    if (*p != '\0') {
	sprintf (temp, "srl: illegal integer value: -%c %s", c, s);
	mexit (temp);
    }
    for (l = limits; l->v; l++)
	if (c == l->c)  {
	    *l->v = v;
	    return;
	}
    sprintf (temp, "srl: mishandled option -%c", c);
    mexit (temp);
}



/*  List the current runtime limit values.  */

void
showlimits ()
{
    struct lim *l;

    printf ("SR runtime limits:\n");
    for (l = limits; l->v; l++)
	printf ("    -%c %d\t%s\n", l->c, *l->v, l->s);
}



/* generate C code to initialize global limit variables  */

void
writelimits (fp)
FILE *fp;
{

    fprintf (fp, "int sr_max_co_stmts = %d;\n", max_co_stmts);
    fprintf (fp, "int sr_max_classes = %d;\n", max_classes);
    fprintf (fp, "int sr_max_loops = %d;\n", max_loops);
    fprintf (fp, "int sr_max_operations = %d;\n", max_operations);
    fprintf (fp, "int sr_max_processes = %d;\n", max_processes);
    fprintf (fp, "int sr_max_rmt_reqs = %d;\n", max_rmt_reqs);
    fprintf (fp, "int sr_max_resources = %d;\n", max_resources);
    fprintf (fp, "int sr_max_semaphores = %d;\n", max_semaphores);
    
    fprintf (fp, "int sr_stack_size = %d;\n", stack_size);
    fprintf (fp, "int sr_async_flag = %d;\n", async_flag);
}
