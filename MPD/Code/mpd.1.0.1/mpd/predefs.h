/*  predefs.h -- list of predefined functions
 *
 *  Each predefined function is listed here.
 *  The keyword functions high, low, and new do not appear here.
 *
 *  The caller of this include file must define the macro premac as needed.
 *  Arguments are:
 *	premac (name, minn, maxn)
 *	name	function name
 *	minn	minimum argument count
 *	maxn	maximum argument count (-1 if no max)
 */

#define MAX_FIXED_ARGS 3

/* type stuff */
premac (lb,	1, 2)
premac (ub,	1, 2)
premac (lb1,	1, 1)
premac (ub1,	1, 1)
premac (lb2,	1, 1)
premac (ub2,	1, 1)
premac (pred,	1, 1)
premac (succ,	1, 1)
premac (length,	1, 1)
premac (maxlength, 1, 1)
premac (chars, 1, 1)

/* resources etc. */
premac (locate,		2, 3)
premac (myresource,	0, 0)
premac (mymachine,	0, 0)
premac (myvm,		0, 0)
premac (mypriority,	0, 0)
premac (setpriority,	1, 1)

/* memory management */
premac (free,	1, 1)

/* basic math */
premac (abs,	1,  1)
premac (min,	1, -1)
premac (max,	1, -1)

premac (sqrt,	1, 1)
premac (log,	1, 2)
premac (exp,	1, 2)

premac (ceil,	1, 1)
premac (floor,	1, 1)
premac (round,	1, 1)

/* trigonometry */
premac (sin,	1, 1)
premac (cos,	1, 1)
premac (tan,	1, 1)
premac (asin,	1, 1)
premac (acos,	1, 1)
premac (atan,	1, 2)

/* random numbers */
premac (random,	0, 2)
premac (seed,	1, 1)

/* argument operations */
premac (numargs, 0, 0)
premac (getarg,	2, 2)

/* timer functions */
premac (nap,	1, 1)
premac (age,	0, 0)

/* basic I/O */
premac (open,	2, 2)
premac (flush,	1, 1)
premac (close,	1, 1)
premac (remove,	1, 1)
premac (get,	1, 2)
premac (put,	1, 2)
premac (seek,	3, 3)
premac (where,	1, 1)

/* formatted I/O */
premac (read,	0, -1)
premac (scanf,	1, -1)
premac (sscanf,	2, -1)
premac (write,	0, -1)
premac (writes,	0, -1)
premac (printf,	1, -1)
premac (sprintf, 2, -1)
