/*  alpha.s -- assembly code for the Alpha architecture
 *
 *  derived from mips.s; info from gcc documentation
 *  and code from Ben Gamsa <ben@sys.toronto.edu> helped much
 *
 *  An Alpha context array is laid out like this:
 *
 *	saved sp register ------------------------------|
 *	magic word for checking integrity		|
 *	unused stack space				| 
 *	saved int/fp registers			     <--- saved sp points here
 *	saved fp/proc val/ret pc registers
 *	older stack data
 *
 *  Both regular and floating point registers are saved.
 */

#define MAGIC 61407		/* an unlikely integer */

/*  space for 6 saved int registers ($9 - $14), 8 FP regs ($f2 - $f9),
 *  3 special int regs ($15, $27, $26), padded to a multiple of 16.
 */
#define RSIZE 144

	.text
	.align 4



/*  sr_build_context(code,context,stksize,arg1,arg2,arg3,arg4)
 *
 *  args passed in:   $16   $17     $18    $19  $20  $21 0($sp)
 *
 *  code	entry point of the code to be executed in the context
 *  context	buffer for holding the context array
 *  stksize	size of this buffer
 *  arg1..arg4	four int-sized arguments to be passed to the code
 *
 *  All we need to do is set up a context that will execute the startup
 *  code, below, when activated for the first time.
 */

	.globl	sr_build_context
	.ent	sr_build_context
sr_build_context:
	.frame	$sp, 16, $26, 0
	ldil	$1, MAGIC
	stq	$1, 8($17)	/* save magic word */
	addq	$17, $18, $2	/* end of buffer */
	subq	$2, RSIZE, $2	/* $2 =  new stack pointer */
	stq	$2, 0($17)	/* save in context array */
	stq	$19, 0($2)	/* save first arg in $9 slot */
	stq	$20, 8($2)	/* save second arg in $10 slot */
	stq	$21, 16($2)	/* save third arg in $11 slot */
	ldq	$3, 0($sp)
	stq	$3, 24($2)	/* save fourth arg in $12 slot */
	stq	$16, 32($2)	/* save entry point in $13 slot */
	lda	$3, startup
	stq	$3, 128($2)	/* save startup address in $26 slot */
	stq	$3, 120($2)	/* and in $27 slot */
	ret	$31, ($26), 1	/* return */

startup:	
	/* put args in arg registers ($16 - $19) for first-time call */
	bis	$9, $9, $16
	bis	$10, $10, $17
	bis	$11, $11, $18
	bis	$12, $12, $19
	bis	$13, $13, $27 	/* restore $27 */
	jsr	$26, ($13), 0	/* call entry point */
	ldgp	$gp, 0($26)
	jsr	$26, sr_stk_underflow	/* if it returned, note underflow */
	.end	sr_build_context



/*  sr_chg_context (newctx, oldctx) -- switch to the specified context 
 *
 *  args passed in:   $16    $17
 */

	.globl	sr_chg_context
	.ent	sr_chg_context
sr_chg_context:
	.frame	$sp, 16, $26, 0
	subq	$sp, RSIZE, $sp	/* make room on stack for saving registers */
	.frame	$sp, RSIZE, $31

	beq	$17, savedone	/* don't use oldctx if 0 */
				/* could rewrite below to test new */
				/* if oldctx is 0 but */
				/* that happens just once on startup */
	ldil	$5, MAGIC	/* magic constant */
	ldq	$2, 8($17)	/* old magic word */
	subq	$sp, $26, $6	/* skip check if it's the real stack */
	ble	$6, checknew
	subq	$sp, $17, $4	/* if stack is currently overflowing */
	ble	$4, over
checknew:
	ldq	$3, 8($16)	/* new magic word */
	subq	$2, $5, $4	/* if it overflowed before & clobbered magic */
	bne	$4, over
	subq	$3, $5, $4	/* if new context isn't intact */
	bne	$4, bad

	stq	$sp, 0($17)	/* save stack pointer, then registers */

	stq	$9, 0($sp)
	stq	$10, 8($sp)
	stq	$11, 16($sp)
	stq	$12, 24($sp)
	stq	$13, 32($sp)
	stq	$14, 40($sp)

	stt	$f2, 48($sp)
	stt	$f3, 56($sp)
	stt	$f4, 64($sp)
	stt	$f5, 72($sp)
	stt	$f6, 80($sp)
	stt	$f7, 88($sp)
	stt	$f8, 96($sp)
	stt	$f9, 104($sp)

	stq	$15, 112($sp)
	stq	$27, 120($sp)

	stq	$26, 128($sp)

savedone:
	ldq	$sp, 0($16)	/* load new stack pointer, then registers */

	ldq	$9, 0($sp)
	ldq	$10, 8($sp)
	ldq	$11, 16($sp)
	ldq	$12, 24($sp)
	ldq	$13, 32($sp)
	ldq	$14, 40($sp)

	ldt	$f2, 48($sp)
	ldt	$f3, 56($sp)
	ldt	$f4, 64($sp)
	ldt	$f5, 72($sp)
	ldt	$f6, 80($sp)
	ldt	$f7, 88($sp)
	ldt	$f8, 96($sp)
	ldt	$f9, 104($sp)

	ldq	$15, 112($sp)
	ldq	$27, 120($sp)

	ldq	$26, 128($sp)

	addq	$sp, RSIZE, $sp

	ret	$31, ($26), 1	/* return into new context */
	.end	sr_chg_context



/*  sr_check_stk(ctx) -- check that the stack is not overflowing  */

	.globl	sr_check_stk
	.ent	sr_check_stk
sr_check_stk:
	.frame	$sp, 16, $26, 0

	subq	$sp, $26, $6	/* skip check if it's the real stack */
	ble	$6, check_done
	subq	$sp, $16, $4	/* check that we haven't grown beyond bounds */
	ble	$4, over
check_done:
	ret	$31, ($26), 1	/* return */



/*  stack problem handlers  (these calls do not return)  */

over:	jmp	sr_stk_overflow
bad:	jmp	sr_stk_corrupted
	.end	sr_check_stk
