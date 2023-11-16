/*  tahoe.s -- Assembly code for Tahoe architecture under Harris HCX/UX
 *
 *  A Harris context array is laid out like this:
 *
 *	saved fp register -------------------------|
 *	magic word for checking integrity	   |
 *	unused stack space			   |
 *	stack frame for continuing execution	<--- saved fp points here
 *	current stack data, if any
 *	stack frame for error handler in case of erroneous return
 *	arguments from sr_build_context call
 *
 *  General registers 6-12 are saved in the stack frame when changing context,
 *  along with pc and fp.  We don't save r0-r5 because C doesn't expect
 *  them to survive a function call.
 */

#define MAGIC $618033989		/* any unlikely long integer */
	
	.text
	.align	2		/* NB Harris doesn't use logarithmic align */



/*  sr_build_context(code,buf,stksize,a1,a2,a3,a4) -- create a new context.
 *
 *  code	entry point of the code to be executed in the context
 *  buf		a buffer for holding the context array
 *  stksize	size of the buffer
 *  a1 - a4	four int-sized arguments to be passed to the code
 */


	.globl	_sr_build_context
_sr_build_context:
	.word	0		/* save no registers */
	addl3	8(fp),12(fp),r1	/* r1 = end of stack */
	movl	r1,sp		/* can't autodecrement r1 on the Harris */

	/* copy the arguments into the stack buffer */
	pushl	28(fp)		/* arg 4 */
	pushl	24(fp)		/* arg 3 */
	pushl	20(fp)		/* arg 2 */
	pushl	16(fp)		/* arg 1 */

	/* build a stack frame to catch (invalid) return from code */
	pushl	$0		/* fp will not be used */
	pushl	$20		/* save mask/removed = (4 args)*4 + 4 */
	pushab	under		/* pc points to underflow handler */
	
	/* build another stack frame to "return" to the given entry point */
	pushab	-20(r1)		/* fp = pointer to underflow frame */
	pushl	$4		/* save mask/removed = (0 args)*4 + 4 */
	addl3	$2,4(fp),-(sp)	/* pc = entry point + 2 (skip reg mask) */

	/* save frame pointer, and also magic word for detecting underflow */
	movl	8(fp),r2	/* r2 = beginning of stack buffer */
	movab	-32(r1),(r2)	/* save pointer to new frame we just made */
	movl	MAGIC,4(r2)	/* store magic word for checking integrity */
	ret			/* return */



/*  sr_chg_context(newstk,oldstk) -- switch context to the specified stack  */

	.globl	_sr_chg_context
_sr_chg_context:
	.word	0x1FC0		/* save the registers C assumes are preserved */
	movl	8(fp),r1	/* r1 = old stack */
	movl	4(fp),r2	/* r2 = new stack */

	tstl	r1		/* see if we're saving the context */
	jeql	first

	cmpl	fp,r1		/* catch current stack overflow */
	jlequ	over		/* (can't use sp as source on Harris) */
	cmpl	MAGIC,4(r1)	/* catch earlier overflow (maybe) */
	jneq	over
	movl	fp,(r1)		/* save old fp */

first:
	cmpl	MAGIC,4(r2)	/* check that new stack looks okay */
	jneq	bad

	movl	(r2),fp		/* set fp for new context */
	ret			/* return into new context */



/*  sr_check_stk(context) -- check that the stack is not overflowing  */

	.globl	_sr_check_stk
_sr_check_stk:
	.word	0
	cmpl	fp,4(fp)	/* again, can't use sp as source on Harris */
	jleq	over
	ret



/*  stack problem handlers  (these calls do not return)  */

over:	calls	$0,_sr_stk_overflow
under:	calls	$0,_sr_stk_underflow
bad:	calls	$0,_sr_stk_corrupted
