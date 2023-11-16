/*  sgi.s -- assembly code for MIPS architecture using the SGI N32 ABI */

/*
 * Copyright (c) 1999 David Choong
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 *  Modified jul99/gmt for inclusion in SR.
 *  I don't claim to understand this; I just removed the O32 section.
 */

/*
 *  DEVELOPER NOTES (READ!!!) : Because of the build infrastructure
 *                              in sr one cannot include system
 *				headers that contain any C code for
 *                              assembly context switching.
 *				This is because this code is passed
 *				directly to as by -E.  As a result, 
 *				I have defined the required
 *				_MIPS_SIM definitions that are located
 *				in sgidefs.h.  This is not the correct
 *				thing to do however, this is a quick
 *				hack that will allow for multiple
 *				versions of the SGI abi.
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
 *
 * _IncrDev	Nov 10, 1988 by David Choong (choong@cs.ualberta.ca)
 *
 *				Notes:	- Supports the n32 abi from SGI
 *					- now must define the abi that you
 *					  want to build with
 *					- I hope to find some time and do
 *                                        a n64 abi version.
 *
 */

#define MAGIC 61407		/* an unlikely integer.  This is used to
				      check that the context
				      array is not clobbered */

#define ASIZE 32		/* space for callee to save 4 arguments 
				     (8 bytes/arg x 4) */
#define RSIZE 128		/* space for 10 saved registers and 6 FP regs	
				     (8 bytes/register x 10 +
				      8 bytes/fpregister x 6) */
#define SR_REG_SIZE_IN_BYTES 8	/* the size in bytes of the registers */
	

	.text
	.align	2
	
	
/*  sr_build_context(code,context,stksize,arg1,arg2,arg3,arg4)
 *
 *  args passed in:   $4    $5      $6     $7  $8      .. $11
 *
 *  code	entry point of the code to be executed in the context
 *  context	buffer for holding the context array
 *  stksize	size of this buffer
 *  arg1..arg4	four int-sized arguments to be passed to the code
 *
 *  All we need to do is set up a context that will execute the startup
 *  code, below, when activated for the first time.
 */

	.globl	sr_build_context	/* make the function extern */
	.ent	sr_build_context	/* for debugging purposes */
sr_build_context:			/* function label */
	.frame	$sp, 0, $31			
	dli	$3, MAGIC		/* Save the MAGIC num in register $3 */
	sd	$3, SR_REG_SIZE_IN_BYTES($5)
					/* Store a doubleword (64 bits) from
			                   register $3 to the address that is
				           contained in register $5 with an
				           offset of SR_REG_SIZE_IN_BYTES */
	add	$2, $5, $6		/* Add the address in register $5
					   (the context buffer pointer) to
					   register $6 (the size of the
					   context buffer) to get the max
					   address of the buffer and put it
					   in register $2 */
	sub	$2, ASIZE+RSIZE		/* Subtract (ASIZE+RSIZE) from register
					   $2 and put the result in register $2.
					   The address that register $2 contains
					   will be put into the first 8 bytes
					   starting at the address contained in
					   register $5.  This is for later use
					   so that the context switch can find
					   the saved registers */
	sd	$2, 0($5)	/* copy the saved address pointer to the
				   first SR_REG_SIZE_IN_BYTES bytes of
				   memory that register $5 points to which
				   is the context array for the thread */
	sd	$7, 48($2)	/* save first arg into the context array 
				   in $16 slot */
	sd	$8, 56($2)	/* save second arg into the context array 
				   in $17 slot */
	sd	$9, 64($2)	/* save third arg into the context array 
				   in $18 slot */
	sd	$10, 72($2)	/* save fourth arg into the context array 
				   in $19 slot */
	sd	$4, 80($2)	/* save entry point into the context array 
				   in $20 slot */
	la	$3, startup	/* load register $3 with the address of
				   startup in memory */
	sd	$3, 120($2)	/* save startup address in $31 slot */
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
	sub	$sp, RSIZE	/* make room on stack for saving registers */
	.frame	$sp, RSIZE, $31

	beq	$5, 0, savedone	/* don't use oldctx if 0 */

	dli	$12, MAGIC	/* magic constant.  Renumbered 13 to the
				   first temp register which is 12 */
	ld	$13, 8($5)	/* old magic word.  Register $9 was
				   a temp register under the o32 abi but, 
				   it is now a function parameter register
				   so it was changed to the second temp
				   register */
	ble	$sp, $5, over	/* if stack is currently overflowing */
	ld	$14, SR_REG_SIZE_IN_BYTES($4)
				/* new magic word.  Register $10 was
				   a temp register under the o32 abi but,
				   it is now a function parameter register
				   so it was changed to the third temp
				   register */
	bne	$13, $12, over	/* if it overflowed before & clobbered magic */
	bne	$14, $12, bad	/* if new context isn't intact */

	.mask	0xC0FF0000, -4
	sd	$sp, 0($5)	/* save stack pointer, then registers */
	sd	$31, 120($sp)	/* save the return address (value of $31) */
	sd	$23, 104($sp)
	sd	$22, 96($sp)
	sd	$21, 88($sp)
	sd	$20, 80($sp)
	sd	$19, 72($sp)
	sd	$18, 64($sp)
	sd	$17, 56($sp)
	sd	$16, 48($sp)
	.fmask	0xFFF00000, -4
	s.d	$f30, 40($sp)	/* save floating-point registers */
	s.d	$f28, 32($sp)
	s.d	$f26, 24($sp)
	s.d	$f24, 16($sp)
	s.d	$f22, 8($sp)
	s.d	$f20, 0($sp)

savedone:

	ld	$sp, 0($4)	/* load new stack pointer, then registers */
	ld	$31, 120($sp)
	ld	$23, 104($sp)
	ld	$22, 96($sp)
	ld	$21, 88($sp)
	ld	$20, 80($sp)
	ld	$19, 72($sp)
	ld	$18, 64($sp)
	ld	$17, 56($sp)
	ld	$16, 48($sp)
	l.d	$f30, 40($sp)	/* load new floating-point registers */
	l.d	$f28, 32($sp)
	l.d	$f26, 24($sp)
	l.d	$f24, 16($sp)
	l.d	$f22, 8($sp)
	l.d	$f20, 0($sp)
	add	$sp, RSIZE	/* adjust stack pointer to remove reg area */
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
