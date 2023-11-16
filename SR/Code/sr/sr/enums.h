/*************************  Enumerated Types  *************************/

/*  Enumerated types with names of the form "e_xxxxx" are processed by awk.
 *  Do not change the layout or this may break.  */



/*  Predef -- predefined functions.  actual list is read from predefs.h.  */


typedef enum {
	PRE_BOGUS,		/* zero value is illegal */
#define premac(name,minn,maxn) PASTE(PRE_,name),
#include "predefs.h"
#undef premac
	PRE_END
} Predef;



/*  Child -- usage of left child field (union variant) in a Node  */

typedef enum {
	CH_NIL,
	CH_NODE,
	CH_SYM,
	CH_VAR,
	CH_NAME,
	CH_INT,
	CH_REAL,
	CH_STR
} Child;



/*  Compat -- degrees of signature compatibility  */

typedef enum {
	C_EXACT,
	C_ASSIGN,
	C_COMPARE
} Compat;



/*  Runtime error messages  */

typedef enum {
#define RUNERR(sym,num,msg) sym=num,
#include "../runerr.h"
#undef RUNERR
	E_END
} runerr;
