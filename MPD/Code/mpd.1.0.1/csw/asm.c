/*  asm.c - include assembly language file for host architecture.
 *
 *  This file is run through cc -E, postprocessed, then fed to as(1).
 *  See the Makefile for details.
 */

#ifndef lint

#include "../arch.h"

#if (defined(__linux__) && defined(__ELF__)) || defined(__svr4__)
#define MPD_BUILD_CONTEXT	mpd_build_context
#define MPD_CHG_CONTEXT		mpd_chg_context
#define MPD_CHECK_STK		mpd_check_stk
#define MPD_STK_OVERFLOW	mpd_stk_overflow
#define MPD_STK_UNDERFLOW	mpd_stk_underflow
#define MPD_STK_CORRUPTED	mpd_stk_corrupted
#else			/* other systems use leading underscore */
#define MPD_BUILD_CONTEXT	_mpd_build_context
#define MPD_CHG_CONTEXT		_mpd_chg_context
#define MPD_CHECK_STK		_mpd_check_stk
#define MPD_STK_OVERFLOW	_mpd_stk_overflow
#define MPD_STK_UNDERFLOW	_mpd_stk_underflow
#define MPD_STK_CORRUPTED	_mpd_stk_corrupted
#endif

#include SFILE

#endif /* lint */
