!   "sparc.s" -- assembly language code for the SPARC architecture
! 
!   A SPARC context array is laid out like this:
! 
! 	saved %sp register -----------------------------|
! 	saved %fp					|
! 	saved %i7 (return addr)				|
! 	magic word for checking integrity		|
! 	unused stack space				| 
! 	current stack frame			<--- saved sp points here
! 	older stack data
!
!   For further information see the SPARC Architecture Manual, Version 8


	.seg	"text"

!   sr_build_context(code,buf,bufsize,a1,a2,a3,a4) -- create a new context.
!
!   args passed in:  %i0  %i1  %i2   %i3 i4 i5 [%sp+64+4+24]
! 
!   code	 entry point of the code to be executed in the context
!   buf		 buffer for holding the context array
!   bufsize	 size of the buffer
!   a1 - a4	 four int-sized arguments to be passed to the code
!
!   We build a context that will execute the startup code, below,
!   when first activated.

	.global	SR_BUILD_CONTEXT
SR_BUILD_CONTEXT:
	save	%sp,-(96),%sp
	add	%i1,%i2,%l1		! end of buffer
	sub	%l1,(96),%l1		! initial stack pointer
	st	%l1,[%i1]		! set %sp
	st	%l1,[%i1+4]		! set also as %fp
	set	startup-8,%l2
	st	%l2,[%i1+8]		! set "return" address
	set	4073,%l3
	st	%l3,[%i1+12]		! set magic word
	st	%i3,[%l1+68]		! save args
	st	%i4,[%l1+68+4]
	st	%i5,[%l1+68+8]
	ld	[%fp+68+24],%l4
	st	%l4,[%l1+68+12]
	st	%i0,[%l1+68+16]		! save code address
	ret
	restore

startup:
	ld	[%sp+68+0],%o0		! reload arguments from stack frame
	ld	[%sp+68+4],%o1
	ld	[%sp+68+8],%o2
	ld	[%sp+68+12],%o3
	ld	[%sp+68+16],%l0
	call	%l0			! call the initial function
	nop
	ba	under			! return is an underflow error
	nop



!  sr_chg_context(newctx,oldctx) -- switch to the specified context 
!
!  args passed in:  %i0     %i1 

	.global	SR_CHG_CONTEXT
SR_CHG_CONTEXT:
	save	%sp,-(96),%sp
	ta	3			! flush register windows
 	tst	%i1			! see if oldctx valid
	be	loadnew			! if zero, just load new context
	nop
	st	%sp,[%i1]		! save %sp
	st	%fp,[%i1+4]		! save %fp
	st	%i7,[%i1+8]		! save return addr
	ld	[%i0+12],%o2		! load magic word
	subcc	%o2,4073,%g0
	bne	bad			! if stack corrupted
	nop
loadnew:
	ld	[%i0],%sp		! load %sp
	ld	[%i0+4],%fp		! load %fp
	ld	[%i0+8],%i7		! load return addr
	ret				! return
	restore



!  sr_check_stk(context) -- check that the stack is not overflowing
!
!  arg passed in  %o0  (we do not shift the register window)

	.global	SR_CHECK_STK
SR_CHECK_STK:
	inc	16,%o0			! lower bound for safe %sp
	cmp	%sp,%o0			! compare with actual stack pointer
	blu	over			! if overflow
	nop
	retl
	nop



!  stack problem handlers  (these calls do not return)

over:	ba	SR_STK_OVERFLOW
	nop

under:	ba	SR_STK_UNDERFLOW
	nop

bad:	ba	SR_STK_CORRUPTED
	nop
