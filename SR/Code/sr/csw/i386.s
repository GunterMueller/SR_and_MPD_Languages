/*  i386.s - intel 80386 context switching code for Sequent Symmetry
 *
 *  (This should work also for other Intel 80386/486/Pentium machines
 *   if they use the same calling sequences and register conventions.)
 *
 *  A context array is laid out like this:
 *
 *	saved registers (%ebp, %ebx, %esi, %edi)
 *  	magic word for checking integrity
 *	unused stack space
 *	saved %esp			<--- saved ebp points here
 *	saved pc (return address)
 *	error routine addr (if code returns, which is an error, it will go here)
 *	arguments from sr_build_context call
 *
 *  Registers %ebp, %ebx, %esi, and %edi are saved at the base of the array.
 *  %eax, %ecx, and %edx aren't saved because C doesn't expect them to survive
 *  over function calls.  %esp is saved on the stack at subroutine entry.
 *
 *  Empirical testing (7/91) seems to show that no floating point registers
 *  need be saved for either cc or gcc, either with or without -O.
 */

#define MAGIC 79797979 		/* any unlikely long integer */
#define DMAGIC $79797979
#define RSIZE 16		/* size of register save area */
#define DRSIZE $16



	.text
#ifdef sequent
	.align	2
#else
	.align	4
#endif



/*  sr_build_context(code,buf,bufsize,a1,a2,a3,a4) -- create a new context.
 *
 *  code	 entry point of the code to be executed in the context
 *  buf		 buffer for holding the context array
 *  bufsize	 size of the buffer
 *  a1 - a4	 four int-sized arguments to be passed to the code
 */

	.globl	SR_BUILD_CONTEXT

SR_BUILD_CONTEXT:
	pushl	%ebp 		/* save caller's frame pointer */
	movl	%esp,%ebp	/* save caller's stack pointer */

	movl	12(%ebp), %eax	/* %eax = pointer to context array */
	movl	%eax, %esp
	addl	16(%ebp), %esp	/* point stack to end of context array */

	pushl	32(%ebp)	/* push arg4 */
	pushl	28(%ebp)	/* push arg3 */
	pushl	24(%ebp)	/* push arg2 */
	pushl	20(%ebp)	/* push arg1 */
	pushl	$under		/* push error addr in case context returns */
	pushl	8(%ebp)		/* push address for startind execution */
	pushl	$0		/* push dummy frame pointer */

	movl	%esp, (%eax)	/* set initial frame pointer*/
	movl	DMAGIC, RSIZE(%eax)  /* store magic word for integrity check */

	movl	%ebp,%esp	/* restore stack and frame pointers */
	popl	%ebp
	ret			/* return */



/*  sr_chg_context(newstk,oldstk) -- switch context to the specified stack  */

	.globl	SR_CHG_CONTEXT
SR_CHG_CONTEXT:
	pushl	%ebp		/* save old frame pointer */
	movl	%esp,%ebp	/* save old stack pointer in frame pointer */

	movl	12(%ebp),%eax	/* load address of current context stack */
	cmpl	$0,%eax
	je	first		/* skip if we don't want to save it */
	movl	%ebp,  0(%eax)	/* save registers in user context block */
	movl	%ebx,  4(%eax)
	movl	%esi,  8(%eax)
	movl	%edi, 12(%eax)

	addl	DRSIZE,%eax
	cmpl	%eax,%esp	/* check that stack isn't overflowing */
	jbe	over
	cmpl	DMAGIC,(%eax)	/* catch earlier overflow (maybe) */
	jne	over

first:
	movl	8(%ebp), %eax	/* load address of new context */
	cmpl	DMAGIC, RSIZE(%eax)	/* make sure new stack looks okay */
	jne	bad

	movl	 0(%eax), %ebp	/* load registers for new context */
	movl	 4(%eax), %ebx
	movl	 8(%eax), %esi
	movl	12(%eax), %edi

	movl	%ebp,%esp	/* restore %esp and %ebp */
	popl	%ebp
	ret			/* return into new context */



/*  sr_check_stk(context) -- check that the stack is not overflowing  */

	.globl	SR_CHECK_STK
SR_CHECK_STK:
	movl	4(%esp), %eax
	addl	DRSIZE, %eax
	cmpl	%eax, %esp
	jbe	over
	ret



/*  stack problem handlers  (these calls do not return)  */

over:	call	SR_STK_OVERFLOW
under:	call	SR_STK_UNDERFLOW
bad:	call	SR_STK_CORRUPTED
