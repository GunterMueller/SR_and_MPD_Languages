/*  compiler.h -- general definitions for the sr compiler
 *
 *  This file is included by all components of the compiler.
 *  Constants and macros are defined here.
 *  This file also includes other generally-needed definition files.
 */

#include "../gen.h"
#include "../srmulti.h"
#include "../sr.h"	/* includes <stdio.h> */
#include "../util.h"
#include "../config.h"
#include "enums.h"
#include "structs.h"
#include "globals.h"
#include "protos.h"



/*************************  Manifest Constants  *************************/

/*  various implementation limitations  */
#define MAX_PROC		200	/* procs per resource */
#define MAX_NEST		20	/* syntactical nesting */
#define MAX_ALLOC		200	/* unfreed invocation blocks */

/*  Errors  */
#define E_WARN   0x80000000		/* flag for nonfatal error */
#define E_IMPORT 0x40000000		/* flag for import text */	
#define E_EXTEND 0x20000000		/* flag for extend text */	
#define E_FILE   0x1FFF0000		/* mask against file index */
#define E_LINE   0x0000FFFF		/* mask against line number */
#define E_FSHIFT 16			/* shift for file index */
#define WARN(s) err(E_WARN+srclocn,s,NULLSTR)
#define FATAL(s) err(srclocn,s,NULLSTR)
#define EFATAL(e,s) err((e)->e_locn,s,NULLSTR)

/*  Null character pointer  (stdio's NULL isn't char* on all systems)  */
#define NULLSTR ((char *) 0)

/*  Debugging configuration  */
#define DEBUG_DEFAULT "bcefops"



/*************************  Type Checking Macros  *************************/

#define TF_ORDERED 1	/* flag for ordered types (n.b. includes "real") */
#define TF_CONVERT 2	/* flag for convertable types (cast, I/O) */
#define TF_DESCR   4	/* flag use of descriptor (string or array) */

#define TFLAGS(t) (tflags[(int)(t)])
#define GFLAGS(g) (TFLAGS((g)->g_type))
#define TSIZE(t) (tsize[(int)(t)])

#define DESCRIBED(g) (GFLAGS(g)&TF_DESCR)
#define ORDERED(g) (GFLAGS(g)&TF_ORDERED)
#define CONVERTIBLE(g)  \
    (GFLAGS(g)&TF_CONVERT||((g)->g_type==T_ARRAY&&(g)->g_usig->g_type==T_CHAR)) 



/*************************  Data Access Macros  *************************/

#define NEW(t) (t*)ralloc(sizeof(t))

/*  is symbol table entry part of our own resource?  */
#define IMPORTED(s) ((s)->s_imp != NULL && (s)->s_imp != curres->r_sym)

/*  left child, iff a node pointer; and right child, for symmetry  */
#define LNODE(e) ((leftchild[(int)(e)->e_opr] == CH_NODE) ? (e)->e_l : NULLNODE)
#define RNODE(e) ((e)->e_r)

/*  walk through list l using char pointer e  */
#define FOREACH(e,l) \
    for ((e) = (char *) (l)->l_first; \
	(e) ? ((e) += sizeof (Lelem)) : 0; \
	(e) = (char *) ((Lelptr) ((e) - sizeof (Lelem)))->l_next)



/*************************  Debugging Macros  *************************/

/*  ASSERT, BOOM -- compiler malfunctions  */
#ifndef NDEBUG
#define ASSERT(ex) {if(!(ex))assfail(__FILE__,__LINE__);}
#define BOOM(s1,s2) kboom(s1,s2,__FILE__,__LINE__)
#else /*NDEBUG*/
#define ASSERT(ex)
#define BOOM(s1,s2) kboom(NULLSTR,NULLSTR,__FILE__,__LINE__)
#endif /*NDEBUG*/

/*  flush a file iff under Saber C (makes functions useful as debugging aids) */
#if defined(__SABER__) || defined(__CODECENTER__)
#define SFLUSH(f) fflush(f)
#else
#define SFLUSH(f)
#endif
