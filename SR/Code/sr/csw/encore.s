/*  encore.s -- Encore Multimax (NS32032) assembly code
 *
 *  (This should also work for other NS32000 machines if they use the same
 *   calling sequence and register conventions.)
 *
 *  An Encore context array is laid out like this:
 *
 *	saved sp register -------------------------|
 *	saved fp register			   |
 *	magic word for checking integrity	   |
 *	unused stack space			   |
 *	stack frame for continuing execution	<--- saved sp,fp  points here
 *	current stack data, if any
 *	stack frame for error handler in case of erroneous return
 *	arguments from sr_build_context call
 *
 *  General registers 3-7 are saved in the stack frame when changing context,
 *  along with fp and pc.  We don't save r0-r2 because C doesn't expect
 *  them to survive a function call.
 *  Save both sp and fp so can use exit instruction.
 *  note: I'm sure someone with an NS32000 manual could code this much better.
 */

#define MAGIC 618033989		/* any unlikely long integer */
#define REGS r3,r4,r5,r6,r7	/* registers C expects to survive call */
	.data
curr_stack:			/* saves addr of current context (stack) */
	.double	dummy_stack
dummy_stack:			/* fake initial context */
	.double	-1		/* this sp should never be restored */
	.double	-1		/* this fp should never be restored */
	.double	MAGIC

	.text
	.align	1



/*  sr_build_context(code,stack,stksize,arg1,arg2,arg3,arg4)
 *                   -- create a new context.
 *
 *  code is the entry point of the code to be executed in the context.
 *  stack is a buffer for holding the stack.
 *  stksize is the size of this buffer.
 *  arg1-4 are the arguments.
 */


	.globl	_sr_build_context
_sr_build_context:
	enter	[],$0		/* save no registers */
	sprd	sp,r1		/* r1 = current sp */
	movd	12(fp),r0
	addd	16(fp),r0	/* r0 = end of stack */
	lprd	sp,r0		/* use sp to build stack buffer */

	/* copy the arguments into the stack buffer */
	movd	32(fp),tos
	movd	28(fp),tos
	movd	24(fp),tos
	movd	20(fp),tos

	/* build a stack frame to catch (invalid) return from code */
	addr	under,tos	/* pc points to underflow handler */
	
	/* build another stack frame to "return" to the given entry point */
	movd	8(fp),tos	/* pc = entry point */
	movd	$0,tos		/* unused fp */
	sprd	sp,r0		/* save the sp for below */
	save	[REGS]		/* save registers in user context block */

	/* save pointers, and also magic word for detecting underflow */
	movd	12(fp),r2	/* r2 = beginning of stack buffer */
	sprd	sp,0(r2)	/* save pointer to new frame we just made */
	movd	r0,4(r2)	/* save pointer to new frame we just made */
	movd	$MAGIC,8(r2)	/* store magic word for checking integrity */

	lprd	sp,r1		/* restore our stack pointer */
	exit	[]
	ret	$0		/* return */



/*  sr_chg_context(newstack) -- switch context to the specified stack  */

	.globl	_sr_chg_context
_sr_chg_context:
	enter	[REGS],$0	/* save the registers C assumes are preserved */
				/* fp is saved first */
	movd	curr_stack,r1	/* r1 = old stack */
	movd	8(fp),r2	/* r2 = new stack */
	movd	r2,curr_stack	/* save for next switch */

	sprd	sp,r0
	cmpd	r0,r1		/* catch current stack overflow */
	bls	over
	cmpd	$MAGIC,8(r1)	/* catch earlier overflow (maybe) */
	bne	over
	cmpd	$MAGIC,8(r2)	/* check that new stack looks okay */
	bne	bad

	sprd	sp,0(r1)	/* save old sp */
	sprd	fp,4(r1)	/* save old fp */
	lprd	sp,0(r2)	/* set sp for new context */
	lprd	fp,4(r2)	/* set fp for new context */
	exit	[REGS]
	ret	$0		/* return into new context */



/*  sr_check_stk() -- check that the stack is not overflowing  */

	.globl	_sr_check_stk
_sr_check_stk:
	sprd	sp,r0 
	cmpd	r0,curr_stack
	bls	over
	ret	$0



/*  stack problem handlers  (these calls do not return)  */

over:	bsr	?_sr_stk_overflow
under:	bsr	?_sr_stk_underflow
bad:	bsr	?_sr_stk_corrupted
