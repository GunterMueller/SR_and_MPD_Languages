/*
 *  svr4.c -- context switch code for Unix System V, release 4.
 *
 *  Some Makefile changes are needed to use this code.
 *
 *  This code has been successfully tested under Solaris 2.1 and 2.2;
 *  it is known not to work with SR_PARALLEL > 1 but this has not been
 *  investigated.
 */

#include <stdlib.h>
#include <math.h>
#include <ucontext.h>

typedef struct {		/* layout of beginning of SR context area */
    ucontext_t ucontext;		/* SVR4 context structure */
    int arg1, arg2, arg3, arg4;		/* initial arguments to pass */
} stk_hdr;

static void startup();
void sr_stk_underflow();



/*
 *  sr_build_context (func, buf, bufsize, arg1, arg2, arg3, arg4)
 *
 *  Build a context that will call func(arg1,arg2,arg3,arg4) when activated
 *  and will catch an underflow error if func returns.  We use an intermediary
 *  in order to catch that return.
 */
void
sr_build_context (func, buf, bufsize, arg1, arg2, arg3, arg4)
void (*func)();
char *buf;
int bufsize, arg1, arg2, arg3, arg4;
{
    stk_hdr *stk = (stk_hdr *) buf;		/* put header at front of buf */
    bufsize -= sizeof (stk_hdr);		/* adjust size accordingly */
    getcontext (&stk->ucontext);		/* initialize context */

#ifdef sparc
    /* Solaris 2.1 makecontext() neglects to adjust for down-growing stack */
    /* but also seems to need a little room above the top */
    stk->ucontext.uc_stack.ss_sp = buf + bufsize - 128;
    stk->ucontext.uc_stack.ss_size = bufsize - sizeof (ucontext_t) - 128;
#else
    stk->ucontext.uc_stack.ss_sp = buf + sizeof (ucontext_t);
    stk->ucontext.uc_stack.ss_size = bufsize - sizeof (ucontext_t);
#endif

    /*
     * This would be simpler if makecontext() could pass five args to the
     * called function.  Under Solaris 2.1 there's a bug and only four work,
     * so we have to store the args in the buffer area.
     */
    stk->arg1 = arg1;
    stk->arg2 = arg2;
    stk->arg3 = arg3;
    stk->arg4 = arg4;
    makecontext (&stk->ucontext, startup, 2, (int) func, (int) stk);
}

/*
 *  startup (func, stk) -- intermediary for startup and underflow detection.
 */
static void
startup (func, stk)
void (*func)();
stk_hdr *stk;
{
    (*func) (stk->arg1, stk->arg2, stk->arg3, stk->arg4);
    sr_stk_underflow();
}



/*
 *  sr_chg_context (newctx, oldctx) -- change contexts.
 */
void
sr_chg_context (new, old)
char *new, *old;
{
    if (old)
	swapcontext (&((stk_hdr*) old)->ucontext, &((stk_hdr*) new)->ucontext);
    else
	setcontext (&((stk_hdr*) new)->ucontext);
}



/*
 *  sr_check_stk (stk) -- check for stack overflow.
 *
 *  We have no idea of how to do that, so we do nothing.
 */
void
sr_check_stk(stk)
char *stk;
{
    /* nothing */
}
