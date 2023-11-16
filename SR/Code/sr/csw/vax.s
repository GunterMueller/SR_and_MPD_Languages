/*  vax.s -- Vax assembly code  (Unix or Ultrix)
 *
 *  A Vax context array is laid out like this:
 *
 *	saved fp register -------------------------|
 *	magic word for checking integrity	   |
 *	unused stack space			   |
 *	stack frame for continuing execution	<--- saved fp points here
 *	current stack data, if any
 *	stack frame for error handler in case of erroneous return
 *	arguments from sr_build_context call
 *
 *  General registers 6-11 are saved in the stack frame when changing context,
 *  along with pc, fp, and ap.  We don't save r0-r5 because C doesn't expect
 *  them to survive a function call.
 */

#define MAGIC $618033989		/* any unlikely long integer */
	
	.text
	.align	1



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
	addl3	8(ap),12(ap),r1	/* r1 = end of stack */

	/* copy the arguments into the stack buffer */
	movl	28(ap),-(r1)	/* arg 4 */
	movl	24(ap),-(r1)	/* arg 3 */
	movl	20(ap),-(r1)	/* arg 2 */
	movl	16(ap),-(r1)	/* arg 1 */
	movl	$4,-(r1)	/* arg count */
	movl	r1,r3		/* save the arg pointer */

	/* build a stack frame to catch (invalid) return from code */
	movab	under,-(r1)	/* pc points to underflow handler */
	movl	$0,-(r1)	/* fp will not be used */
	movl	$0,-(r1)	/* ap will not be used */
	movl	$0,-(r1)	/* psw */
	movl	$0,-(r1)	/* condition handler */
	
	/* build another stack frame to "return" to the given entry point */
	addl3	$2,4(ap),-(r1)	/* pc = entry point + 2 (skip reg mask) */
	movab	4(r1),-(r1)	/* fp = pointer to underflow frame */
	movl	r3,-(r1)	/* ap */
	movl	$0,-(r1)	/* psw */
	movl	$0,-(r1)	/* condition handler */

	/* save frame pointer, and also magic word for detecting underflow */
	movl	8(ap),r2	/* r2 = beginning of stack buffer */
	movl	r1,(r2)+	/* save pointer to new frame we just made */
	movl	MAGIC,(r2)	/* store magic word for checking integrity */
	ret			/* return */



/*  sr_chg_context(newstk,oldstk) -- switch context to the specified stack  */

	.globl	_sr_chg_context
_sr_chg_context:
	.word	0xFC0		/* save the registers C assumes are preserved */
	movl	8(ap),r1	/* r1 = old stack */
	movl	4(ap),r2	/* r2 = new stack */

	tstl	r1		/* see if we're saving the context */
	jeql	first

	cmpl	sp,r1		/* catch current stack overflow */
	jleq	over
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
	cmpl	sp,4(ap)
	jleq	over
	ret



/*  stack problem handlers  (these calls do not return)  */

over:	calls	$0,_sr_stk_overflow
under:	calls	$0,_sr_stk_underflow
bad:	calls	$0,_sr_stk_corrupted
