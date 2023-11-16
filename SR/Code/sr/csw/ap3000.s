/*  ap3000.s -- Apollo 3000 68K assembly language code
 *
 *  A context array is laid out like this:
 *
 *	saved registers
 *  	magic word for checking integrity
 *	unused stack space
 *	saved pc		<--- saved sp points here  
 *	valid stack data (if any)
 *	address of error handler (in case of erroneous return)
 *	arguments from sr_build_context call
 *
 *  Registers are always saved at the base of the array. a0/a1/d0/d1 aren't
 *  saved because C doesn't expect them to survive over function calls.
 *  The set does include sp, so we don't need to handle it separately.
 */ 

#define MAGIC 618033989		/* any unlikely long integer */
#define RMASK 0xFCFC		/* register save mask (d2-d7, a2-a6, sp) */
#define RSIZE 48		/* size of register save area */

	.data
base_frame:			/* saves initial value of A6 */
	.long	0
curr_stack:			/* saves addr of current context (stack) */
	.long	dummy_stack
dummy_stack:			/* fake initial context */
	.skip	RSIZE		/* room to save initial register set */
	.long	MAGIC		/* magic word to pass the sanity check */

	.text


/*  sr_build_context(code,buf,bufsize,a1,a2,a3,a4) -- create a new context.
 *
 *  code	 entry point of the code to be executed in the context
 *  buf		 buffer for holding the context array
 *  bufsize	 size of the buffer
 *  a1 - a4	 four int-sized arguments to be passed to the code
 */


	.globl	sr_build_context
sr_build_context:
	cmpl	#0,base_frame	/* check to see if base_frame established */
	jne	bc1		/* if nonzero, it's valid */
	movl	a6,base_frame	/* no, save value from this first call */
bc1:
	movl	a6,sp@-		/* save caller's frame pointer */
	movl	sp@(12),a1	/* a1 = pointer to context array */
	movl	a1,a6
	addl	sp@(16),a6	/* a6 = pointer to end of context's stack */

	movl	sp@(32),a6@-	/* push arg4 */
	movl	sp@(28),a6@-	/* push arg3 */
	movl	sp@(24),a6@-	/* push arg2 */
	movl	sp@(20),a6@-	/* push arg1 */

	movl	#under,a6@-	/* push error addr in case context returns */
	movl	sp@(8),a6@-	/* push address for starting execution */

	movl	sp,a0		/* save pointer to stack we were called on */
	movl	a6,sp		/* set user stack pointer */
	movl	base_frame,a6	/* link to base context */
	moveml	#RMASK,a1@	/* save registers in user context block */
	addl	#RSIZE,a1
	movl	#MAGIC,a1@	/* store magic word for integrity checking */
	movl	a0,sp		/* restore our stack pointer */
	movl	sp@+,a6		/* restore caller's frame pointer */
	rts			/* return */



/*  sr_chg_context(newstack) -- switch context to the specified stack  */

	.globl	sr_chg_context
sr_chg_context:
	movl	curr_stack,a0	/* load address of current context stack */
	moveml	#RMASK,a0@	/* save registers (including sp) */
	addl	#RSIZE,a0
	cmpl	a0,sp		/* check that stack isn't overflowing */
	jcs	over
	cmpl	#MAGIC,a0@	/* catch earlier overflow (maybe) */
	jne	over
	movl	sp@(4),a0	/* load address of new context */
	lea	a0@(RSIZE),a1
	cmpl	#MAGIC,a1@	/* make sure new stack looks okay */
	jne	bad
	movl	a0,curr_stack	/* save address of new context */
	moveml	a0@,#RMASK	/* load new context (including sp) */
	rts			/* return into new context */


/*  sr_check_stk() -- check that the stack is not overflowing  */

	.globl	sr_check_stk
sr_check_stk:
	movl	curr_stack,a0
	addl	#RSIZE,a0
	cmpl	a0,sp
	jcs	over
	rts


/*  stack problem handlers  (these calls do not return)  */

over:	jbsr	sr_stk_overflow
under:	jbsr	sr_stk_underflow
bad:	jbsr	sr_stk_corrupted
