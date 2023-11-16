/*  runerr.h - runtime error messages
 *
 *  These are the messages for errors detected by the generated code, plus some
 *  others caught inside the RTS that make use of the same formatting routines.
 *  This is *not* a list of all the possible runtime errors.
 *
 *  This file is included by both compiler and RTS with various definitions
 *  of RUNERR in effect.  The generated code passes the error number and 
 *  variable arguments to the RTS, which looks up the error message in a table.
 *
 *  Messages can include these substitutions:
 *
 *	fmt  argtype	meaning
 *	---  --------	------------------------
 *	%d   int	decimal integer
 *	%B   Array *	array bounds
 *	%L   String *	current string length
 *	%M   String *	maximum string length
 *	%S   String *	string value (truncated to reasonable length)
 *
 *  Each definition below gives symbol, error number, and message.
 */

RUNERR(E_SEMV,11,"illegal semaphore initializer value: %d")

RUNERR(E_ABND,20,"illegal bounds [%d:%d] for array creation")
RUNERR(E_ASUB,21,"illegal subscript [%d] of array with bounds %B")
RUNERR(E_ASLC,22,"illegal slice [%d:%d] of array with bounds %B")
RUNERR(E_ASIZ,23,"size mismatch on array assignment:  %B := %B")
RUNERR(E_ACHG,25,"size mismatch on assignment to slice: [%d:%d] := %B")
RUNERR(E_AREP,26,"illegal repetition count %d in array constructor")

RUNERR(E_SSUB,31,"illegal subscript [%d] of string[%M] with current length %L")
RUNERR(E_SSLC,32,"illegal slice [%d:%d] of string[%M] with current length %L")
RUNERR(E_SSIZ,33,"string of length %L is too long to assign to string[%M]")
RUNERR(E_SELM,34,"element %d (length=%d) is too long to assign to string[%d]")
RUNERR(E_SCHG,35,"string of length %L is wrong length to assign to slice [%d:%d]")

RUNERR(E_CCNV,41,"illegal conversion: char(%d)")
RUNERR(E_BCNV,42,"illegal conversion: bool(\"%S\")")
RUNERR(E_ICNV,43,"illegal conversion: int(\"%S\")")
RUNERR(E_RCNV,44,"illegal conversion: real(\"%S\")")
