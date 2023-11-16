/*  mips.s -- assembly code for the MIPS RISC architecture
 *
 * ======> for SGI systems, this code assumes the "O32" ABI  (see "man abi")
 *
 *  A MIPS context array is laid out like this:
 *
 *	saved sp register ------------------------------|
 *	magic word for checking integrity		|
 *	unused stack space				| 
 *	saved registers, including $31 (return addr) <--- saved sp points here
 *	older stack data
 *
 *  Both regular and floating point registers are saved.  As of 7/91 the
 *  current Dec and SGI compilers don't seem to utilize f.p. temporaries,
 *  but gcc does.
 */

#define MAGIC 61407		/* an unlikely integer */

#define ASIZE 16		/* space for callee to save 4 arguments */
#define RSIZE 88		/* space for 10 saved registers and 6 FP regs */
	
	.text
	.align	2



/*  sr_build_context(code,context,stksize,arg1,arg2,arg3,arg4)
 *
 *  args passed in:   $4    $5      $6     $7  16($sp) .. 24($sp)
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
	.frame	$sp, 0, $31
	li	$3, MAGIC
	sw	$3, 4($5)	/* save magic word */
	addu	$2, $5, $6	/* end of buffer */
	subu	$2, ASIZE+RSIZE	/* $2 =  new stack pointer */
	sw	$2, 0($5)	/* save in context array */
	sw	$7, 48($2)	/* save first arg in $16 slot */
	lw	$3, 16($sp)
	sw	$3, 52($2)	/* save second arg in $17 slot */
	lw	$3, 20($sp)
	sw	$3, 56($2)	/* save third arg in $18 slot */
	lw	$3, 24($sp)
	sw	$3, 60($2)	/* save fourth arg in $19 slot */
	sw	$4, 64($2)	/* save entry point in $20 slot */
	la	$3, startup
	sw	$3, 84($2)	/* save startup address in $31 slot */
	j	$31		/* return */

startup:	
	move	$4,$16		/* load arg registers for first-time call */
	move	$5,$17
	move	$6,$18
	move	$7,$19
	la	$31,under	/* detect underflow if called code returns */
	move	$25,$20		/* MIPS ABI requires $25 for indirect call */
	j	$25		/* jump to entry point */
	.end	sr_build_context



/*  sr_chg_context (newctx, oldctx) -- switch to the specified context 
 *
 *  args passed in:   $4    $5
 */

	.globl	sr_chg_context
	.ent	sr_chg_context
sr_chg_context:
	subu	$sp, RSIZE	/* make room on stack for saving registers */
	.frame	$sp, RSIZE, $31

	beq	$5, 0, savedone	/* don't use oldctx if 0 */

	li	$13, MAGIC	/* magic constant */
	lw	$9, 4($5)	/* old magic word */
	ble	$sp, $5, over	/* if stack is currently overflowing */
	lw	$10, 4($4)	    /* new magic word */
	bne	$9, $13, over	/* if it overflowed before & clobbered magic */
	bne	$10, $13, bad	    /* if new context isn't intact */

	.mask	0xC0FF0000, -4
	sw	$sp, 0($5)	/* save stack pointer, then registers */
	sd	$30, 80($sp)	/* includes $31, our caller's return address */
	sd	$22, 72($sp)
	sd	$20, 64($sp)
	sd	$18, 56($sp)
	sd	$16, 48($sp)
	.fmask	0xFFF00000, -4
	s.d	$f30, 40($sp)	/* save floating-point registers */
	s.d	$f28, 32($sp)
	s.d	$f26, 24($sp)
	s.d	$f24, 16($sp)
	s.d	$f22, 8($sp)
	s.d	$f20, 0($sp)

savedone:

	lw	$sp, 0($4)	/* load new stack pointer, then registers */
	ld	$30, 80($sp)	/* includes $31, the resumption address */
	ld	$22, 72($sp)
	ld	$20, 64($sp)
	ld	$18, 56($sp)
	ld	$16, 48($sp)
	l.d	$f30, 40($sp)	/* load new floating-point registers */
	l.d	$f28, 32($sp)
	l.d	$f26, 24($sp)
	l.d	$f24, 16($sp)
	l.d	$f22, 8($sp)
	l.d	$f20, 0($sp)
	addu	$sp, RSIZE	/* adjust stack pointer to remove reg area */
	j	$31		/* return into new context */

	.end	sr_chg_context



/*  sr_check_stk(ctx) -- check that the stack is not overflowing  */

	.globl	sr_check_stk
	.ent	sr_check_stk
sr_check_stk:
	.frame	$sp, 0, $31

	ble	$sp, $4, over	/* check that we haven't grown beyond bounds */
	j	$31		/* return */



/*  stack problem handlers  (these calls do not return)  */

over:	jal	sr_stk_overflow
under:	jal	sr_stk_underflow
bad:	jal	sr_stk_corrupted

	.end	sr_check_stk
