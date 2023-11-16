/*  v68s.s -- System V/68 assembly language code
 *
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
	
	data
curr_stack:			/* saves addr of current context (stack) */
	long	dummy_stack
dummy_stack:			/* fake initial context */
	space	RSIZE		/* room to save initial register set */
	long	MAGIC		/* magic word to pass the sanity check */

	text



/*  sr_build_context(code,buf,bufsize,a1,a2,a3,a4) -- create a new context.
 *
 *  code	 entry point of the code to be executed in the context
 *  buf		 buffer for holding the context array
 *  bufsize	 size of the buffer
 *  a1 - a4	 four int-sized arguments to be passed to the code
 */


	global	sr_build_context
sr_build_context:
	mov.l	%a6,-(%sp)	/* save caller's frame pointer */
	mov.l	12(%sp),%a1	/* a1 = pointer to context array */
	mov.l	%a1,%a6
	add.l	16(%sp),%a6	/* a6 = pointer to end of context's stack */

	mov.l	32(%sp),-(%a6)	/* push arg4 */
	mov.l	28(%sp),-(%a6)	/* push arg3 */
	mov.l	24(%sp),-(%a6)	/* push arg2 */
	mov.l	20(%sp),-(%a6)	/* push arg1 */

	mov.l	&under,-(%a6)	/* push error addr in case context returns */
	mov.l	8(%sp),-(%a6)	/* push address for starting execution */

	mov.l	%sp,%a0		/* save pointer to stack we were called on */
	mov.l	%a6,%sp		/* set user stack pointer */
	movm.l	&RMASK,(%a1)	/* save registers in user context block */
	add.l	&RSIZE,%a1
	mov.l	&MAGIC,(%a1)	/* store magic word for integrity checking */
	mov.l	%a0,%sp		/* restore our stack pointer */
	mov.l	(%sp)+,%a6	/* restore caller's frame pointer */
	rts			/* return */



/*  sr_chg_context(newstack) -- switch context to the specified stack  */

	global	sr_chg_context
sr_chg_context:
	mov.l	curr_stack,%a0	/* load address of current context stack */
	movm.l	&RMASK,(%a0)	/* save registers (including sp) */
	add.l	&RSIZE,%a0
	cmp.l	%sp,%a0		/* check that stack isn't overflowing */
	bcs	over
	cmp.l	(%a0),&MAGIC	/* catch earlier overflow (maybe) */
	bne	over
	mov.l	4(%sp),%a0	/* load address of new context */
	lea	RSIZE(%a0),%a1
	cmp.l	(%a1),&MAGIC	/* make sure new stack looks okay */
	bne	bad
	mov.l	%a0,curr_stack	/* save address of new context */
	movm.l	(%a0),&RMASK	/* load new context (including sp) */
	rts			/* return into new context */



/*  sr_check_stk() -- check that the stack is not overflowing  */

	global	sr_check_stk
sr_check_stk:
	mov.l	curr_stack,%a0
	add.l	&RSIZE,%a0
	cmp.l	%sp,%a0
	bcs	over
	rts



/*  stack problem handlers  (these calls do not return)  */

over:	jsr	sr_stk_overflow
under:	jsr	sr_stk_underflow
bad:	jsr	sr_stk_corrupted
