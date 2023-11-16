 /* Adapted for use on hp9000/300 from sun.s.  For comments see that file.  */

	data 
curr_ctx:			
	long 	dummy_ctx
dummy_ctx:			
	space	48		
	long 	618033989		

	text 


	global	_sr_build_context
_sr_build_context:
	mov.l	%a6,-(%sp)		
	mov.l	12(%sp),%a1	
	mov.l	%a1,%a6
	add.l	16(%sp),%a6	

	mov.l	32(%sp),-(%a6)
	mov.l	28(%sp),-(%a6)
	mov.l	24(%sp),-(%a6)
	mov.l	20(%sp),-(%a6)

	mov.l	&under,-(%a6)	
	mov.l	8(%sp),-(%a6)	

	mov.l	%sp,%a0		
	mov.l	%a6,%sp		
	movem.l	&0xFCFC,(%a1)	
	add.l	&48,%a1
	mov.l	&618033989,(%a1)	
	mov.l	%a0,%sp		
	mov.l	(%sp)+,%a6		
	rts			


	global	_sr_chg_context
_sr_chg_context:
	mov.l	curr_ctx,%a0	
	movem.l	&0xFCFC,(%a0)	
	add.l	&48,%a0
	cmp.l	%sp,%a0		
	bcs	over
	cmp.l	(%a0),&618033989
	bne	over
	mov.l	4(%sp),%a0	
	lea	48(%a0),%a1
	cmp.l	(%a1),&618033989
	bne	bad
	mov.l	%a0,curr_ctx	
	movem.l	(%a0),&0xFCFC	
	rts			


	global	_sr_check_stk
_sr_check_stk:
	mov.l	curr_ctx,%a0
	add.l	&48,%a0
	cmp.l	%sp,%a0
	bcs	over
	rts


over:	jsr 	_sr_stk_overflow
under:	jsr 	_sr_stk_underflow
bad:	jsr 	_sr_stk_corrupted
