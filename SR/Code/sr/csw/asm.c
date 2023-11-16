/*  asm.c - include assembly language file for host architecture.
 *
 *  This file is run through cc -E, postprocessed, then fed to as(1).
 *  See the Makefile for details.
 */

#ifndef lint

#include "../arch.h"

#if (defined(__linux__) && defined(__ELF__)) || defined(__svr4__)
#define SR_BUILD_CONTEXT	sr_build_context
#define SR_CHG_CONTEXT		sr_chg_context
#define SR_CHECK_STK		sr_check_stk
#define SR_STK_OVERFLOW		sr_stk_overflow
#define SR_STK_UNDERFLOW	sr_stk_underflow
#define SR_STK_CORRUPTED	sr_stk_corrupted
#else			/* other systems use leading underscore */
#define SR_BUILD_CONTEXT	_sr_build_context
#define SR_CHG_CONTEXT		_sr_chg_context
#define SR_CHECK_STK		_sr_check_stk
#define SR_STK_OVERFLOW		_sr_stk_overflow
#define SR_STK_UNDERFLOW	_sr_stk_underflow
#define SR_STK_CORRUPTED	_sr_stk_corrupted
#endif

#include SFILE

#endif /* lint */
