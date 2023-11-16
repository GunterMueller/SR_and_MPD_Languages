/*  paragon.s -- assembly language code for the i860 architecture
 *
 *  A i860 context array is laid out like this:
 *
 *	saved code address 4
 *	saved r2 - r15	   14 * 4 = 56 + 4  = 60
 *	saved f2 - f6	    6 * 4 = 20 + 60 = 84
 *	magic word for checking integrity	84 + 4  = 88
 *	arguments for call to code	4 * 4 = 16 + 88 = 104
 *	unused stack space
 *
 *  For further information see the i860 Microprocessor Family
 *  Programmer's Reference Manual
 */


/*  sr_build_context(code,buf,bufsize,a1,a2,a3,a4) -- create a new context.
 *
 *  args passed in:  r16  r17  r18   r19 r20 r21 r22
 *
 *  code	 entry point of the code to be executed in the context
 *  buf		 buffer for holding the context array
 *  bufsize	 size of the buffer
 *  a1 - a4	 four int-sized arguments to be passed to the code
 */

	.text
	.globl	_sr_build_context
	.align 32
_sr_build_context:
	st.l	r16,0(r17)      /* save entry point of code */
	addu	r18,r17,r18     /* calculate stack pointer */
	and 	-15,r18,r18	/* enforce 16 byte boundary rule */
	st.l	r18,4(r17)      /* save stack pointer */
	st.l	r18,8(r17)      /* save frame pointer */
	mov	31,r16
	st.l    r16,84(r17) 	/* save magic word  */
	st.l    r19,88(r17)	/* save a1 - a4 */
	st.l	r20,92(r17)
	st.l    r21,96(r17)
	st.l	r22,100(r17)
	mov r18,r16
	bri r1
	nop

/*  sr_chg_context(newctx,oldctx) -- switch to the specified context 
 *
 *  args passed in:  r16     r17 
 */
	.globl	_sr_chg_context
	.align 32
_sr_chg_context:
	
	bte	0,r17,loadnew 	/* jump if new context to loadnew */
	nop
	ld.l	84(r17),r18     /* check magic word	 */
	btne	31,r18,bad
	nop
	st.l	r1,0(r17)	/* save register (incl return addr SP etc.) */
	st.l	r2,4(r17)
	st.l	r3,8(r17)
	st.l	r4,12(r17)
	st.l	r5,16(r17)
	st.l	r6,20(r17)
	st.l	r7,24(r17)
	st.l	r8,28(r17)
	st.l	r9,32(r17)
	st.l	r10,36(r17)
	st.l	r11,40(r17)
	st.l	r12,44(r17)
	st.l	r13,48(r17)
	st.l	r14,52(r17)
	st.l	r15,56(r17)
	fst.l	f2,60(r17)	/* save float register */
	fst.l	f3,64(r17)
	fst.l	f4,68(r17)
	fst.l	f5,72(r17)
	fst.l	f6,76(r17)
	fst.l	f7,80(r17)
	
loadnew:
	ld.l 	0(r16),r21	/* load return address to r21 */
	ld.l 	4(r16),r2	/* load register  */
	ld.l 	8(r16),r3
	ld.l 	12(r16),r4
	ld.l 	16(r16),r5
	ld.l 	20(r16),r6
	ld.l 	24(r16),r7
	ld.l 	28(r16),r8
	ld.l 	32(r16),r9
	ld.l 	36(r16),r10
	ld.l 	40(r16),r11
	ld.l 	44(r16),r12
	ld.l 	48(r16),r13
	ld.l 	52(r16),r14
	ld.l 	56(r16),r15
	fld.l 	60(r16),f2	/* load float register */
	fld.l 	64(r16),f3
	fld.l 	68(r16),f4
	fld.l 	72(r16),f5
	fld.l 	76(r16),f6
	fld.l 	80(r16),f7
	mov     r16,r20		/* load parameters */
	ld.l 	88(r20),r16	
	ld.l 	92(r20),r17	
	ld.l 	96(r20),r18	
	ld.l 	100(r20),r19
	nop
	calli r21		/* return to new context */
	nop
	br	under		/* should be never reached (underflow error) */
	nop

/*  sr_check_stk(context) -- check that the stack is not overflowing
 *
 *  arg passed in  r16  
 */
	.globl	_sr_check_stk
	.align 32
_sr_check_stk:
	addu	88,r16,r16	/* checks stack pointer for overflow */
	subs    r16,r2,r0
	bnc	over		/* if overflow error */
	nop
	bri	r1		/* else return */
	nop
	

/*  stack problem handlers  (these calls do not return)  */

	.align 32
over:	br	_sr_stk_overflow
	nop

	.align 32
under:	br	_sr_stk_underflow
	nop

	.align 32
bad:	br	_sr_stk_corrupted
	nop
