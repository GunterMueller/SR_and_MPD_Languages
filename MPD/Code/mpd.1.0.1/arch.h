/*  arch.h - define symbols according to host architecture.
 *
 *  ARCH	descriptive name of the architecture
 *  SFILE	assembly language code (.s) file to include in runtime system
 *  LOW_REAL	low(real), if not obtainable from <float.h>
 *  HIGH_REAL	high(real)
 *  BIGCC	arguments passed to CC to enlarge tree space (if needed)
 */

#ifndef ARCH

#ifdef apollo
#if _ISP__M68K == 1
#define ARCH	"Motorola 68000 (Apollo)"
#define SFILE	"ap3000.s"
#endif
#endif

#ifdef hp9000s800
#define ARCH	"HP Precision"
#define SFILE	"precision.s"
#endif

#ifdef i386
#define ARCH	"Intel 386"
#define SFILE	"i386.s"
#define LOW_REAL 2.2250738585072023e-308
#define HIGH_REAL 1.797693134862315e+308
#ifdef sequent
#define BIGCC	"-W0,-Nt3000"
#endif
#endif

#ifdef __mips
#ifndef mips
#define mips
#endif
#endif

#ifdef mips
#if defined(_ABIN32)
#define ARCH	"SGI N32"
#define SFILE	"sgi.s"
#elif defined(_ABI64)
#define ARCH	"SGI N64"
 ERROR ERROR ERROR -- N64 ABI is not supported
#else
#define ARCH	"MIPS"
#define SFILE	"mips.s"
#endif
/* max value limited so as to be acceptable to Iris cc & printf */
#define LOW_REAL 2.2250738585072014e-308
#define HIGH_REAL 1.797693134862313e+308
#ifndef __DECC
#define BIGCC	"-Wf,-XNh3000"
#endif
#endif /*mips*/

#ifdef __alpha
#define ARCH	"alpha"
#define SFILE	"alpha.s"
#endif

#ifdef mc68000
#define ARCH	"Motorola 68000"
#define SFILE	"m68k.s"
#define LOW_REAL 4.94065645841246544e-324
#define HIGH_REAL 1.797693134862315708e+308
#endif

#ifdef sysV68
#define ARCH	"Motorola 68000 (System V/68)"
#define SFILE	"v68.s"
#define LOW_REAL 4.94065645841246544e-324
#define HIGH_REAL 1.797693134862315708e+308
#endif

#ifdef hp9000s300
#define ARCH	"Motorola 68000 (HP-UX)"
#define SFILE	"bobcat.s"
#define LOW_REAL 4.94065645841246544e-324
#define HIGH_REAL 1.797693134862315708e+308
#define BIGCC	"+Ne700"
#endif

#ifdef __m88k__
#define ARCH	"Motorola 88000"
#define SFILE	"m88k.s"
#endif

#ifdef ns32000
#define ARCH	"National 32000"
#define SFILE	"encore.s"
/*
 * Real range is from <values.h> although neither number printf's as shown.
 * HIGH_REAL should be 1.7976931348623140e+308 but that causes floating exceptn.
 */
#define LOW_REAL 2.2250738585072030e-308
#define HIGH_REAL 1.7976931348622e+308
#define NO_FMOD					/* no fmod() in math library */
#endif

#ifdef _AIX
#define ARCH	"IBM RS/6000"
#define SFILE	"rs6000.s"
#endif

#ifdef sparc
#define ARCH	"SPARC"
#define SFILE	"sparc.s"
#define HIGH_REAL 1.797693134862315708e+308	/* from <values.h> */
#ifdef __svr4__
#define LOW_REAL 2.2251e-308	/* empirical; limited by Solaris asm */
#else
#define LOW_REAL 4.94065645841246544e-324
#endif
#endif

#ifdef tahoe	/* hope that's right... perhaps "cci"?... */
#define ARCH	"Tahoe"
#define SFILE	"tahoe.s"
#endif

#ifdef hcx
#define ARCH	"Harris"
#define SFILE	"tahoe.s"
#endif

#ifdef vax
#define ARCH	"Vax"
#define SFILE	"vax.s"
#endif

#ifdef __PARAGON__
#define ARCH	"Intel Paragon"
#define SFILE	"paragon.s"
#define HIGH_REAL 1.7976931348623157e+308	/* from <values.h> */
#define LOW_REAL 2.2251e-308			/* empirical cc minimum */
#endif

#endif	/* ARCH */


/*  If no architecture was selected, we have an error  */

#ifndef ARCH
    ERROR -- no architecture selected
#endif
