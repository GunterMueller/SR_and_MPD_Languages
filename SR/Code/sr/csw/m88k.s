/*  m88k.s -- assembly code for the Motorola 88000 architecture
 *
 *  An m88k context array is laid out like this:
 *
 *	saved sp register ------------------------------|
 *	magic word for checking integrity		|
 *	unused stack space				| 
 *	saved registers, including r1 (return addr) <--- saved sp points here
 *	older stack data
 *
 *  The stack must always remain 8-byte aligned.
 *  We assume that the caller will ensure this.
 *
 *  R26-29 are saved for the benefit of the Green Hills compiler;
 *  they don't appear to be needed by GCC.
 */

#define MAGIC 61407		/* an unlikely integer */

/*  The next two definitions must be a multiple of 8  */
#define RSIZE 72		/* space needed for 18 saved registers */
#define ASIZE 16		/* space needed for 4-argument buffer */


	file	"m88k.s"
	data
	align	4
curr_ctx:			/* saves addr of current context array */
	word	dummy_ctx	/* initialized with dummy context */

dummy_ctx:			/* dummy initial context */
	word	dummy_stack	/* pointer to stack */
	word	MAGIC		/* magic word to pass the sanity check */

/* dummy_stack: */
	bss	dummy_stack,RSIZE,8	/* room to save the registers */



	text
	align	4



/*  sr_build_context(code,context,stksize,arg1,arg2,arg3,arg4)
 *
 *  args passed in:   r2    r3      r4     r5   r6   r7   r8
 *
 *  code	entry point of the code to be executed in the context
 *  context	buffer for holding the context array
 *  stksize	size of this buffer
 *  arg1..arg4	four int-sized arguments to be passed to the code
 *
 *  All we need to do is set up a context that will execute the startup
 *  code, below, when activated for the first time.
 */


	global	_sr_build_context
_sr_build_context:
 	or	r10,r0,MAGIC
	st	r10,r3,4	/* save magic word */
	addu	r9,r3,r4	/* end of buffer */
	subu	r9,r9,ASIZE	/* allow room for argument buffer */
	subu	r9,r9,RSIZE	/* r9 = new stack pointer */
	st	r9,r3,0		/* save in context array */
	st	r2,r9,4		/* save code address in r14 slot */
	st	r5,r9,8		/* save first arg in r15 slot */
	st	r6,r9,12	/* save second arg in r16 slot */
	st	r7,r9,16	/* save third arg in r17 slot */
	st	r8,r9,20	/* save fourth arg in r18 slot */
	st	r30,r9,52	/* inherit current frame pointer as a base */
	or.u	r10,r0,hi16(startup)
	or	r10,r10,lo16(startup)
	st	r10,r9,0	/* save startup address in r1 slot */
	jmp	r1		/* return */
	or	r0,r0,r0	/* safe instr following a branch */

startup:	
	or	r2,r0,r15	/* load arg registers for first-time call */
	or	r3,r0,r16
	or	r4,r0,r17
	or	r5,r0,r18
	or.u	r1,r0,hi16(under)	/* detect underflow if code returns */
	or	r1,r1,lo16(under)
	jmp	r14		/* jump to entry point */
	or	r0,r0,r0	/* safe instr following a branch */



/*  sr_chg_context(newctx) -- switch to the specified context  */

	global	_sr_chg_context
_sr_chg_context:
	subu	r31,r31,RSIZE	/* make room on stack for saving registers */
	or.u	r3,r0,hi16(curr_ctx)
	or	r3,r3,lo16(curr_ctx)
	ld	r4,r3,0		/* r4 = addr of current context array */
	cmp	r5,r31,r4
	bb1  ls,r5,over		/* abort if stack is overflowing */
	ld	r5,r4,4
	cmp	r5,r5,MAGIC
	bb1  ne,r5,bad		/* abort if earlier overflow clobbered magic */
	ld	r5,r2,4
	cmp	r5,r5,MAGIC
	bb1  ne,r5,bad		/* abort if new context isn't intact */

	st	r31,r4,0	/* save old SP, then registers */
	st	r1,r31,0
	st	r14,r31,4
	st	r15,r31,8
	st	r16,r31,12
	st	r17,r31,16
	st	r18,r31,20
	st	r19,r31,24
	st	r20,r31,28
	st	r21,r31,32
	st	r22,r31,36
	st	r23,r31,40
	st	r24,r31,44
	st	r25,r31,48
	st	r30,r31,52
	st	r29,r31,56
	st	r28,r31,60
	st	r27,r31,64
	st	r26,r31,68

	ld	r31,r2,0	/* load new SP, then registers */
	ld	r1,r31,0
	ld	r14,r31,4
	ld	r15,r31,8
	ld	r16,r31,12
	ld	r17,r31,16
	ld	r18,r31,20
	ld	r19,r31,24
	ld	r20,r31,28
	ld	r21,r31,32
	ld	r22,r31,36
	ld	r23,r31,40
	ld	r24,r31,44
	ld	r25,r31,48
	ld	r30,r31,52
	ld	r29,r31,56
	ld	r28,r31,60
	ld	r27,r31,64
	ld	r26,r31,68

	addu	r31,r31,RSIZE	/* adjust SP to remove register area */
	st	r2,r3,0		/* save addr of (new) current context */
	jmp	r1		/* return into new context */
	or	r0,r0,r0	/* safe instr following a branch */



/*  sr_check_stk() -- check that the stack is not overflowing  */

	global	_sr_check_stk
_sr_check_stk:
	or.u	r13,r0,hi16(curr_ctx)
	ld	r10,r13,lo16(curr_ctx)	/* load addr of curr context array */
	cmp	r11,r31,r10		/* compare stack pointer with it */
	bb1  ls,r11,over	/* check that we haven't grown beyond it */
	or	r0,r0,r0	/* nop -- can't have jmp right after bb1 */
	jmp  r1			/* return */
	or	r0,r0,r0	/* safe instr following a branch */



/*  stack problem handlers  (these calls do not return)  */

over:
	bsr	_sr_stk_overflow
	or	r0,r0,r0		/* safe instr following a branch */

under:
	bsr	_sr_stk_underflow
	or	r0,r0,r0		/* safe instr following a branch */

bad:
	bsr	_sr_stk_corrupted
	or	r0,r0,r0		/* safe instr following a branch */
