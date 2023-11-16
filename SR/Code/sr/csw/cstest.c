/*  cstest.c -- context switch test
 *
 *  This program checks the SR context switch routines in various ways.
 *  The expected output appears in cstest.stdout.
 *  Note that a "stack underflow" exit IS expected at the end.
 */


#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include "../arch.h"
#include "../gen.h"

#define STACK_SIZE 16000	/* stack size; must be a multiple of 16 */
#define BOUNDARY 256		/* boundary size; must be multiple of 16 */

#define N_RECURSE 10		/* number of recursive calls */
#define N_STACK 5		/* number of stacks (also see code) */

#define CK_BEFORE 0xB4B4B4B4	/* check value for area below the stack */
#define CK_AFTER  0xAFAFAFAF	/* check value for area above the stack */

extern void sr_build_context ();
extern void sr_chg_context ();

struct envelope *spawn ();
double mul3 ();
void dotest(), ckenv(), err(), dstack(), dump();
void foo(), bar(), baz(), boo(), stir();
void sr_check_stk(), sr_stk_corrupted(), sr_stk_overflow(), sr_stk_underflow();



/* enclose the stacks in a larger structure, to check for errors */

struct envelope {
    long before[BOUNDARY];	/* for boundary checks before */
    char stack[STACK_SIZE];	/* actual stack - must be mod-8 aligned */
    long after[BOUNDARY];	/* for boundary checks after */
};

struct envelope *env[N_STACK];	/* four stacks in envelopes */
char *s1, *s2, *s3, *s4, *s5;	/* actual context pointers */

int recursion_depth = 0;	/* current recursion depth */
int expect_error = 0;		/* is error expected? */



/*  main program.  */

main ()
{
    setbuf (stdout, (char *) NULL);	/* unbuffer stdout & stderr */
    setbuf (stderr, (char *) NULL);
    fprintf (stderr, "running context switch test, arch = %s\n", ARCH);
    dotest ((char *) NULL);
    err ("dotest() returned to main()");
    /*NOTREACHED*/
}



/*  create four contexts, then jump to the first.  */

void
dotest (context)
char *context;		/* caller's context */
{
    static char *p[] = { "one", "two", "three", "four" };

    env[0] = spawn(foo, "foo", (long)p[0], (long)0x111,(long)0x222,(long)0x333);
    env[1] = spawn(bar, "bar", (long)p[1], (long)p[2], (long)0x303,(long)0x404);
    env[2] = spawn(baz, "baz", (long)p[0], (long)p[1], (long)p[2], (long)0x300);
    env[3] = spawn(boo, "boo", (long)p[0], (long)p[1], (long)p[2], (long)p[3]);
    env[4] = spawn(stir,"stir",(long)0x501,(long)0x502,(long)0x503,(long)0x504);
    s1 = env[0]->stack;
    s2 = env[1]->stack;
    s3 = env[2]->stack;
    s4 = env[3]->stack;
    s5 = env[4]->stack;
    ckenv ();
    sr_chg_context (s4, context);	/* should not return */
    err ("s4 returned to dotest()");
}



/*  create a stack and spawn a process.  */

struct envelope *
spawn (pc, name, arg1, arg2, arg3, arg4) 
void (*pc) ();
char *name;
long arg1, arg2, arg3, arg4;
{
    int i;
    struct envelope *r;

    printf ("creating stack for %s\n", name);
    r = (struct envelope *) malloc (sizeof (struct envelope));
    if (!r)
	err ("can't get memory for new stack");
    
    for (i = 0; i < BOUNDARY; i++) {
	r->before[i] = CK_BEFORE;
	r->after[i] = CK_AFTER;
    }
    for (i = 0; i < STACK_SIZE; i++)
	r->stack[i] = rand ();

    sr_build_context (pc, r->stack, STACK_SIZE, arg1, arg2, arg3, arg4);
    return r;
}



/*  four coroutines for checking context switching.  */

/* foo is in context s1 */
void
foo (a)
char *a;
{
    printf (" 5. foo1(%s)\n", a);
    sr_check_stk (s1);
    ckenv ();
    sr_chg_context (s2, s1);
    printf (" 8. foo2(%s)\n", a);
    sr_check_stk (s1);
    ckenv ();
    sr_chg_context (s3, s1);
    printf ("XX. foom!!!!!!!\n");
    exit (1);
}

/* bar is in context s2 */
void
bar (a, b)
char *a, *b;
{
    double d;

    printf (" 6. bar1(%s,%s)\n", a, b);
    sr_check_stk (s2);
    ckenv ();
    sr_chg_context (s3, s2);
    printf ("10. bar2(%s,%s)\n", a, b);
    sr_check_stk (s2);

    puts ("===== checking floating point =====");
    d = mul3 (mul3 (2., 3., 5., s2), mul3 (7., 11., 13., s2), 
	mul3 (17., 19., 23., s2), s2);
    if (d != 223092870.)
	printf ("oops -- product returned was %.20g\n", d);
    sr_check_stk (s2);

    if (recursion_depth++ < N_RECURSE) {
	puts ("====== recursing ======");
	dotest (s2);
	err ("dotest() returned to bar()");
    }
    puts ("====== finishing up ======");
    printf ("now expect underflow error:\n");
    expect_error = 1;
    return;	/* should underflow and be caught */ 
}

/* baz is in context s3 */
void
baz (a, b, c)
char *a, *b, *c;
{
    printf (" 7. baz1(%s,%s,%s)\n", a, b, c);
    sr_check_stk (s3);
    ckenv ();
    sr_chg_context (s1, s3);
    printf (" 9. baz2(%s,%s,%s)\n", a, b, c);
    sr_check_stk (s3);
    ckenv ();
    sr_chg_context (s2, s3);
    printf ("XX. bazzzzzzzt!!!!!\n");
    exit (1);
}

/* boo is in context s4 */
void
boo (a, b, c, d)
char *a, *b, *c, *d;
{
    printf (" 1. boo1(%s,%s,%s,%s)\n", a, b, c, d);
    sr_check_stk (s4);
    ckenv ();
    sr_chg_context (s4, s4);
    printf (" 2. boo2(%s,%s,%s,%s)\n", a, b, c, d);
    sr_check_stk (s4);
    ckenv ();
    sr_chg_context (s4, s4);
    printf (" 3. boo3(%s,%s,%s,%s)\n", a, b, c, d);
    sr_check_stk (s4);
    ckenv ();
    sr_chg_context (s4, s4);
    printf (" 4. boo4(%s,%s,%s,%s)\n", a, b, c, d);
    sr_check_stk (s4);
    ckenv ();
    sr_chg_context (s1, s4);
    printf ("XX. boom!!!!!!!\n");
    exit (1);
}

/* stir is in context s5 */
void
stir ()		/* stir floating point registers, then switch to stack 2 */
{
    double x = 1.7, y = 3.2, z = 2.4;
    for (;;) {
	printf ("stirring...");
	x = sin ((y = cos (x + y + .4)) - (z = cos (x + z + .6)));
	sr_chg_context (s2, s5);
    }
}



/*  multiply three doubles, with context switching  */

double mul3 (x, y, z, ctx)
double x, y, z;
char *ctx;		/* context */
{
    printf ("mul3...");
    sr_chg_context (s5, ctx);		/* stir up the f.p. registers */
    printf ("returning\n");
    return x * y * z;
}



/*  check stack envelopes  */

void
ckenv ()
{
    int i, j;
    int nerrs = 0;
    struct envelope *e;

    for (i = 1; i <= N_STACK; i++) {
	e = env[i-1];
	for (j = 0; j < BOUNDARY; j++)
	    if (e->before[j] != CK_BEFORE) {
		printf ("error -- memory corrupted below stack %d\n", i);
		nerrs++;
		break;
	    }
	for (j = 0; j < BOUNDARY; j++)
	    if (e->after[j] != CK_AFTER) {
		printf ("error -- memory corrupted above stack %d\n", i);
		nerrs++;
		break;
	    }
    }
    if (nerrs)
	err ("aborting due to memory corruption");
}



/*  stack error handlers  */

void sr_stk_corrupted () { err ("csw: stack corrupted"); }
void sr_stk_overflow ()  { err ("csw: stack overflow"); }
void sr_stk_underflow () { err ("csw: stack underflow"); }



/*  announce error, and exit.  */

void
err (s)
char *s;
{
    puts (s);
    if (expect_error)
	exit (0);
    else
	exit (1);
}



/*  dump stack s; this may help with debugging.  */
/*  NOT TESTED ON 64-BIT ARCHITECTURES.  */

void
dstack (s, label)
char *s, *label;
{
    dump (s, 0, 7, label);
    dump (s, STACK_SIZE - 100, STACK_SIZE - 1, "");
}



/*  dump(addr,m,n,label) -- dump addr+m through addr+n, with label.  */

void
dump (addr, m, n, label)
char *addr, *label;
int m, n;
{
    char c, *p = addr + m, *q = addr + n;  int i;
    if (!label)
	label = "";
    printf ("%s ---------------------------------------\n", label);
    while (p <= q)  {
	printf ("\t%08X (%2d)  %08X  ", (int) p, p - addr, i = * (int *) p);
	c = toascii (*p++);  putchar (isprint (c) ? c : '.');
	c = toascii (*p++);  putchar (isprint (c) ? c : '.');
	c = toascii (*p++);  putchar (isprint (c) ? c : '.');
	c = toascii (*p++);  putchar (isprint (c) ? c : '.');
	if (i != 0 && abs (i) <= 10000)
	    printf ("%6d\n", i);
	else
	    putchar ('\n');
    }
}
