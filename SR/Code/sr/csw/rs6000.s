/*  rs6000.s -- SR assembly code for the RS6000 architecture
 *
 *  Jon Crowcroft, 1991, 1993
 *  Gregg Townsend, 1994
 *
 *  The RS6000 stack grows from high address to low.
 *  A context array is laid out like this:
 *
 *	TOP (stack address + stack size)
 *	--------------------------------------------
 *	  -4	saved SP
 *	  -8	TOC
 *	 -12	LR
 *	 -16	CR
 *	 -20	unused
 *	 -88	GP 13 - 31  (68 bytes)
 *	-232	FP 14 - 31  (144 bytes)
 *	-288	callee's save area (56 bytes)
 * 	 ...
 * 	room for growth
 * 	 ...
 *	  +4	magic word  (should be stored and checked here)
 *	  +0	stack size  (stored here)
 *	--------------------------------------------
 *	BOTTOM (stack address points here)
 */

/*
 * To Do:
 * Check magic number in stack
 * context[context[0]-8] == MAGIC
 * Check stack bounds!!!
 * sp is in context[context[0]-4]
 * sp >=context[context[0]] -> Underflow
 * sp < context -> Overflow
 */

	.file	"rs6000.s"
	.set	stkoff, 288		/* starting stack offset */

/* data entry in Table of Contents */
.toc
T.contextdat: .tc contextdat[tc], .contextdat[rw]
	.globl	.contextdat
	.csect	.contextdat[rw]
.contextdat:
gogogo:
	.long	sr_startup_context



/* stack problem handlers  (these calls do not return) */

	.csect	sr_stk_error[PR]

	.extern	.sr_stk_overflow
	.extern	.sr_stk_underflow
	.extern	.sr_stk_corrupted

over:	b	.sr_stk_overflow
under:	b	.sr_stk_underflow
bad:	b	.sr_stk_corrupted



/*  sr_build_context(code,context,stksize,arg1,arg2,arg3,arg4)
 *               GPR:  3     4       5      6    7    8    9
 *
 *  code	entry point of the code to be executed in the context
 *  context	buffer for holding the context array
 *  stksize	size of this buffer
 *  arg1..arg4	four int-sized arguments to be passed to the code
 *
 */

	.globl	sr_build_context[ds]	/* runtime linkage space */
	.csect	sr_build_context[ds]
	.long	.sr_build_context[PR]
	.long	TOC[tc0]
	.long	0

.toc					/* function entry in TOC */
T.sr_build_context: .tc sr_build_context[tc], .sr_build_context[PR]

	.globl	.sr_build_context[PR]	/* the program segment */
	.csect	.sr_build_context[PR]

	.align	2

	st	5, 0(4)		/* save stack size */
	a	4, 4, 5		/* adjust GP 4 to point to stack TOP */
	l	10, T.contextdat(2)

	.using	.contextdat[rw], 10

	/* save code address and args in GP 13 - 17 of context */

	l	13, 0(3)	/* code */
	or	14, 6, 6	/* args 1 */
	or	15, 7, 7	/* args 2 */
	or	16, 8, 8	/* args 3 */
	or	17, 9, 9	/* args 4 */
	stm	13, -88(4)	/* save GP registers */

	/* save miscellaneous registers */

	si	5, 4, stkoff	/* safe first stk offset */
	st	5, -4(4)	/* SP */

	st	2, -8(4)	/* TOC */

	l	5, gogogo	
	st	5, -12(4)	/* startup code -> LR slot  */

	mfcr	0
	st	0, -16(4)	/* CR */

	brl			/* ret */

/* on entry here, r3 has context, r13 new Link Reg - don't touch r13-31 */
.globl sr_startup_context
sr_startup_context: /* only need args first time proc starts */
	mtlr	13		/* code start -> link reg */
	or	3, 14, 14	/* args 1 */
	or	4, 15, 15	/* args 2 */
	or	5, 16, 16	/* args 3 */
	or	6, 17, 17	/* args 4 */
	brl
	b	.sr_stk_underflow 	/* if anyone returns - underflow! */
/* end sr_build_context */



/*
 * sr_chg_context(newctx, oldctx)
 *                GPR 3    GPR 4
 *
 *  a bit like Called Routines job - see Ch 4 - 13 in Assembler Ref
 */
	.globl	sr_chg_context[ds]	/* runtime linkage space */
	.csect	sr_chg_context[ds]
	.long	.sr_chg_context[PR]
	.long	TOC[tc0]
	.long	0
.toc				/* function entry in TOC */
T.sr_chg_context: .tc sr_chg_context[tc], .sr_chg_context[PR]

	.globl	.sr_chg_context[PR]
	.csect	.sr_chg_context[PR]
	l	10, T.contextdat(2)
	.using	.contextdat[rw], 10

	cmpi	0, 0, 4, 0
	beq	load
	l	5, 0(4)		/* to top */
	a	4, 4, 5		/* get to top of stk */

	/*
	 * Save Old Context
	 */
	st	1, -4(4)	/* SP */
	st	2, -8(4)	/* TOC */
	mflr	0
	st	0, -12(4)	/* LR (return address) */
	mfcr	0
	st	0, -16(4)	/* CR */

	stm	13, -88(4)	/* GPRs 13-31 */

	stfd	14, -232(4)	/* FPRs */
	stfd	15, -224(4)
	stfd	16, -216(4)
	stfd	17, -208(4)
	stfd	18, -200(4)
	stfd	19, -192(4)
	stfd	20, -184(4)
	stfd	21, -176(4)
	stfd	22, -168(4)
	stfd	23, -160(4)
	stfd	24, -152(4)
	stfd	25, -144(4)
	stfd	26, -136(4)
	stfd	27, -128(4)
	stfd	28, -120(4)
	stfd	29, -112(4)
	stfd	30, -104(4)
	stfd	31, -96(4)

	/*
	 * Restore New Context
	 */
load:
	l	5, 0(3)
	a	3, 3, 5
	l	1, -4(3)	/* sp from newcontext -> sp */

	lm	13, -88(3)	/* restore GPRs 13-31 -> context */

	lfd	14, -232(3)	/* FPRs */
	lfd	15, -224(3)
	lfd	16, -216(3)
	lfd	17, -208(3)
	lfd	18, -200(3)
	lfd	19, -192(3)
	lfd	20, -184(3)
	lfd	21, -176(3)
	lfd	22, -168(3)
	lfd	23, -160(3)
	lfd	24, -152(3)
	lfd	25, -144(3)
	lfd	26, -136(3)
	lfd	27, -128(3)
	lfd	28, -120(3)
	lfd	29, -112(3)
	lfd	30, -104(3)
	lfd	31, -96(3)

/* don't touch 13-31 */
	l	2, -8(3)	/* TOC -> context */
	l	0, -12(3)	/* LR -> context */
	mtlr	0		/* get LR */
	l	0, -16(3)	/* CR -> context */
	mtcr	0		/* get CR */
	brl			/* execute new context */
	brl			/* ?? -- seems necessary */
/* end sr_chg_context */



/*
 * sr_check_stk() -- check that the stack is not overflowing
 */
	.globl	sr_check_stk[ds]	/* runtime linkage space */
	.csect	sr_check_stk[ds]
	.long	.sr_check_stk[PR]
	.long	TOC[tc0]
	.long	0

.toc					/* function entry in TOC */
T.sr_check_stk: .tc sr_check_stk[tc], .sr_check_stk[PR]
	.globl	.sr_check_stk[PR]
	.csect	.sr_check_stk[PR]

	brl			/* return */

/* end	sr_check_stk */
