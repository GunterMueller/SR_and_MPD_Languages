;;  precision.s -- context switching for the HP Precision architecture
;
;   With this architecture the stack grows upward.
;   A context array is laid out like this:
;
;	stack size word				(lowest address)
;	saved sp
;	saved general registers
;	saved floating point registers
;	argument save area for top-level callee (16 bytes)
;	frame marker  (32 bytes)
;	    ...
;	stack frames
;	    ...
;	unused space
;	    ...
;	magic word for catching overflow	(highest address)



MAGIC	.EQU	1019		; an arbitrary small integer for sanity checking

; the next three must be multiples of 8 for proper alignment
RSIZE	.EQU	120		; size of register save area (incl size word)
ASIZE	.EQU	16		; size of argument save area
PSIZE	.EQU	32		; size of procedure mark

	.GLOBAL
L$curr_ctx			; pointer to current context
	.WORD	L$dummy_ctx	; initially set to dummy context
	.ALIGN	8
L$dummy_ctx			; skeletal old context for first context switch
	.WORD	4+RSIZE+4	; length
	.BLOCK	RSIZE		; register save area
	.WORD	MAGIC		; magic word



;;  sr_build_context(func,buf,bufsize,a1,a2,a3,a4)
;
;   args passed in:  r26  r25   r24  r23 stack...
;
;   func is the entry point of the code to be executed in the context.
;   stack is a buffer for holding the stack.
;   stacksize is the size of this buffer.
;   a1 - a4 are four int-size args to be passed to func.
;
;   We build a context that will execute the startup code, below,
;   when activated for the first time.

	.CODE
	.EXPORT	sr_build_context
sr_build_context
	.PROC
	.CALLINFO
	.ENTRY	
	STW	%arg2, (%arg1)		; init stacksize word
	ADD	%arg1, %arg2, %r21
	LDI	MAGIC, %r22
	STW	%r22, -4(%r21)		; init magic word
	ADDI	RSIZE+ASIZE+PSIZE, %arg1, %r22
	STW	%r22, 4(%arg1)		; init stack pointer
	LDIL	L'L$startup, %r1
	LDO	R'L$startup(%r1), %r22
	STW	%r22, 8(%arg1)		; init "return" address

	; save args in the slots of %r16 through %r13
	; save code address in slot of %r12
	STW	%arg3, 64(%arg1)	; %r16
	LDW	-52(%sp), %r22
	STW	%r22, 60(%arg1)		; %r15
	LDW	-56(%sp), %r22
	STW	%r22, 56(%arg1)		; %r14
	LDW	-60(%sp), %r22
	STW	%r22, 52(%arg1)		; %r13

	; under HP-UX 8.0, func address may be indirect
	BB,>=,N	%arg0,30,L$funcaddr	; if not indirect
	DEPI	0,31,2,%arg0		; clear indirect bit
	LDW	0(%arg0),%arg0		; load real address
L$funcaddr
	STW	%arg0, 48(%arg1)	; %r12  (func addr)

	; need to save valid floating point status register
	LDO	80(%arg1), %r20
	FSTDS,MA  %fr0, 8(%r20)		; base+80

	BV,N	(%r2)			; return

L$startup
	COPY	%r16, %arg0
	COPY	%r15, %arg1
	COPY	%r14, %arg2
	COPY	%r13, %arg3
	LDIL	L'L$under, %rp
	LDO	R'L$under(%rp), %rp	; direct return
	STW	%rp, -24(%sp)		; indirect return
	BV,N	(%r12)

	.EXIT
	.PROCEND




; sr_chg_context(newstack) -- switch to the specified stack

	.EXPORT sr_chg_context
sr_chg_context
	.PROC
	.CALLINFO
	.ENTRY
	; validate old stack
	ADDIL	L'L$curr_ctx-$global$, %dp
	LDO	R'L$curr_ctx-$global$(%r1), %r19	; &curr_ctx
	LDW	(%r19), %r20		; curr stack pointer
	LDW	(%r20), %r22		; stack length
	ADD	%r20, %r22, %r21	; end of stack
	LDW	-4(%r21), %r21		; magic word
	SUBI,=	MAGIC, %r21, %r21
	  B,N	L$bad			; if magic word clobbered
	; validate new stack
	LDW	(%arg0), %r22		; stack length
	ADD	%arg0, %r22, %r21	; end of stack
	LDW	-4(%r21), %r21		; magic word
	SUBI,=	MAGIC, %r21, %r21
	  B,N	L$bad			; if magic word clobbered
	; save general registers
	STW	%sp, 4(%r20)
	STW	%r2, 8(%r20)
	STW	%r3, 12(%r20)
	STW	%r4, 16(%r20)
	STW	%r5, 20(%r20)
	STW	%r6, 24(%r20)
	STW	%r7, 28(%r20)
	STW	%r8, 32(%r20)
	STW	%r9, 36(%r20)
	STW	%r10, 40(%r20)
	STW	%r11, 44(%r20)
	STW	%r12, 48(%r20)
	STW	%r13, 52(%r20)
	STW	%r14, 56(%r20)
	STW	%r15, 60(%r20)
	STW	%r16, 64(%r20)
	STW	%r17, 68(%r20)
	STW	%r18, 72(%r20)
	; save floating registers  (note: must be double-word aligned)
	LDO	80(%r20), %r22
	FSTDS,MA  %fr0, 8(%r22)		; base+80
	FSTDS,MA  %fr12, 8(%r22)	; base+88
	FSTDS,MA  %fr13, 8(%r22)	; base+96
	FSTDS,MA  %fr14, 8(%r22)	; base+104
	FSTDS,MA  %fr15, 8(%r22)	; base+112

	; load general registers
	LDW	4(%arg0), %sp
	LDW	8(%arg0), %r2
	LDW	12(%arg0), %r3
	LDW	16(%arg0), %r4
	LDW	20(%arg0), %r5
	LDW	24(%arg0), %r6
	LDW	28(%arg0), %r7
	LDW	32(%arg0), %r8
	LDW	36(%arg0), %r9
	LDW	40(%arg0), %r10
	LDW	44(%arg0), %r11
	LDW	48(%arg0), %r12
	LDW	52(%arg0), %r13
	LDW	56(%arg0), %r14
	LDW	60(%arg0), %r15
	LDW	64(%arg0), %r16
	LDW	68(%arg0), %r17
	LDW	72(%arg0), %r18
	; load floating registers
	LDO	80(%arg0), %r22
	FLDDS,MA  8(%r22), %fr0		; base+80
	FLDDS,MA  8(%r22), %fr12	; base+88
	FLDDS,MA  8(%r22), %fr13	; base+96
	FLDDS,MA  8(%r22), %fr14	; base+104
	FLDDS,MA  8(%r22), %fr15	; base+112

	STW	%arg0, (%r19)		; update current-context pointer
	BV,N	(%r2)			; return into new context
	.EXIT
	.PROCEND

	
	.EXPORT sr_check_stk
sr_check_stk
	.PROC
	.CALLINFO
	.ENTRY
	ADDIL	L'L$curr_ctx-$global$, %dp
	LDW	R'L$curr_ctx-$global$(%r1), %r20    ; current context pointer
	LDW	(%r20), %r22		; stack length
	ADD	%r20, %r22, %r21	; end of stack
	SUB,<	%sp, %r21, %r0
	  B,N	L$over			; if stack grew too far
	LDW	-4(%r21), %r21		; magic word
	SUBI,=	MAGIC, %r21, %r21
	  B,N	L$bad			; if magic word clobbered
	BV,N	(%r2)			; return
	.EXIT
	.PROCEND



; error handlers

	.IMPORT sr_stk_overflow
	.IMPORT sr_stk_underflow
	.IMPORT sr_stk_corrupted

L$over	LDIL	L%sr_stk_overflow, %r1
	BE,N	R%sr_stk_overflow(%sr4,%r1)

L$under	LDIL	L%sr_stk_underflow, %r1
	BE,N	R%sr_stk_underflow(%sr4,%r1)

L$bad	LDIL	L%sr_stk_corrupted, %r1
	BE,N	R%sr_stk_corrupted(%sr4,%r1)

	.END
