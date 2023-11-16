/*
 *  structs.h -- structure definitions
 *
 *  This file defines several C structs used throughout the compiler.
 *
 *  Enumerated types with names of the form "e_xxxxx" are processed by awk to
 *  build "names.h".  Be aware of this if contemplating any changes in format.
 */



/*
 *  List -- a double-ended, doubly-linked list of any sort of data
 *  (See list.c for additional comments)
 */

typedef struct list {		/* header */
    struct lelem *l_first;	/* first element */
    struct lelem *l_last;	/* last element */
    int l_esize;		/* element size including element header */
} *List;

/* IMPORTANT: for proper alignment, make sizeof(Lelem) a multiple of ALCGRAN. */

typedef struct lelem {		/* element */
    struct lelem *l_next;	/* next element */
    struct lelem *l_prev;	/* previous element */
} Lelem, *Lelptr;		/* (data follows) */



/*
 *  Node -- a parse tree node
 *  
 *  Parsing a resource produces a single, large binary tree.
 *  The tree starts out as a faithful representation of the input
 *  but is then altered by type coercion, conversion to canonical form,
 *  etc.  Code is generated directly from the tree.
 *
 *  The "e_opr" field distinguishes the various node types.  This field
 *  contains a Operator enum, as defined in "operators.h".  The particular
 *  opr value determines the use of the other fields.
 */

typedef enum e_operator {
#define OPR(name,left) PASTE(O_,name),
#include "operators.h"
#undef OPR
O_END
} Operator;

typedef struct node {
    Operator e_opr;		/* node type (O_xxxx) */
    struct sig *e_sig;		/* signature */
    union {			/* left pointer or value */
	struct node   *eu_l;	    /* left child node */
	struct symbol *eu_sym;	    /* symbol struct */
	struct var    *eu_var;	    /* var struct */
	char *eu_name;		    /* identifier name */
	int eu_int;		    /* integer value */
	Real *eu_rptr;		    /* pointer to real value */
	String *eu_sptr;	    /* pointer to string value */
    } e_u;
    struct node *e_r;		/* right pointer */
    int e_locn;			/* source line number */
} Node, *Nodeptr;

#define	NULLNODE ((Nodeptr) 0)

/*  shorthand field references  */
#define e_l	e_u.eu_l
#define e_sym	e_u.eu_sym
#define e_var	e_u.eu_var
#define e_name	e_u.eu_name
#define e_int	e_u.eu_int
#define e_rptr  e_u.eu_rptr
#define e_sptr  e_u.eu_sptr



/*
 *  Signat -- the signature of a symbol, variable, or expression
 *
 *  A signature is a DAG of Signat nodes.  The g_type field gives the
 *  top-level classification of a type (int, pointer, union, etc.)
 *  and other fields lead to underlying information (e.g. fields of a record).
 *
 *  The "uses" column below is keyed to the comments in the Signat structure
 *  declaration, and indicates which fields of the structure are valid for
 *  a particular Type.
 */

typedef enum e_type {

    /* tag	  uses	meaning */
    /*-----	  ----	------- */
    T_BOGUS,	/*	if seen, this is an error */
    T_VOID,	/*	for use when no type applies */
    T_ANY,	/*	argtype for externals */
    T_NLIT,	/*	NULL/NOOP literals */
    T_BOOL,	/*	bool */
    T_CHAR,	/*	char */
    T_INT,	/*	int */
    T_ENUM,	/*   s	enum; idlist s */
    T_REAL,	/*	real */
    T_FILE,	/*	file */
    T_STRING,	/*   b	string(b) */
    T_ARRAY,	/*  ub	array(b) of u */
    T_PTR,	/*  u	ptr to u */
    T_REC,	/* r s	record; fieldlist s */
    T_VCAP,	/*	cap vm */
    T_RCAP,	/*  u	cap for resource u */
    T_OCAP,	/*  u	cap for op u */
    T_GLB,	/* p s	global; prototype p, symtab entry s */	
    T_RES,	/* p s	resource; prototype p, symtab entry s */
    T_OP,	/* pus	operation; prototype p, param list s, returns u */
    TCOUNT	/*      not used as a type; gives number of defined types */
} Type;

typedef struct sig {
    Type g_type;		/* entry type (T_xxxx) */
    struct sig *g_usig;		/* u: signature of underlying type */
    struct node *g_lb, *g_ub;	/* b: exprs giving bounds */
    struct proto *g_proto;	/* p: prototype (param list etc.) */
    struct symbol *g_sym;	/* s: member/arg list, or resource sym */
    struct recdata *g_rec;	/* r: record type data */
} Signat, *Sigptr;

#define NULLSIG ((Sigptr) 0)



/*  Symbol -- the meaning of a name
 *
 *  A tree of Symbol nodes reflects the block structure of the current resource.
 *  The meaning of an identifier is found by searching from the current leaf
 *  back towards the root, ignoring other branches (which represent identifiers
 *  that are currently out of scope).  
 *
 *  The "s_kind" field distinguishes different uses of the Symbol struct and
 *  thereby different types of meaning of an identifier.
 */

typedef enum e_kind {
    /* tag	uses variant	meaning */
    /*-----	------------	------- */
    K_BOGUS,	/*		erroneous value */
    K_BLOCK,	/* s_pb		block (tree branches for scoping) */
    K_VAR,	/* s_var	variable (includes const, param, result) */
    K_FIELD,	/* s_var	field of a record, union, or enum */
    K_FVAL,	/*		formal param, passed by value */
    K_FVAR,	/*		formal param, variable */
    K_FRES,	/*		formal param, result only */ 
    K_FREF,	/*		formal param, passed by reference */
    K_TYPE,	/*		defined type */
    K_ELIT,	/* s_seq	enumeration literal */
    K_RES,	/* s_res	resource */
    K_IMP,	/*		link via s_child to imported symbol */
    K_OP,	/* s_op		user-defined operation */ 
    K_PREDEF	/* s_pre	predefined function */
} Kind;

typedef struct symbol {
    Kind s_kind;		/* entry type (K_xxxx) */
    int s_depth;		/* scope nesting depth */
    char *s_name;		/* name (as known by SR) */
    char *s_gname;		/* generated name (for use in C code) */
    struct symbol *s_imp;	/* resource from which imported */
    struct symbol *s_next;	/* forward link */
    struct symbol *s_prev;	/* backward link */
    struct symbol *s_child;	/* child link */
    struct sig *s_sig;		/* signature */
    union {
	struct var *su_var;	/* var/const details */
	struct res *su_res;	/* resource details */
	struct op  *su_op;	/* operation details */
	Predef      su_pre;	/* predefined function index */
	struct symbol *su_lnk;	/* link to another symtab entry */
	char	   *su_pb;	/* parameter block pointer name, if any */
	int	    su_seq;	/* parameter number or enum literal value */
    } s_u;
} Symbol, *Symptr;

#define	NULLSYM ((Symptr) 0)

/*  shorthand field references  */
#define s_var	s_u.su_var
#define s_res	s_u.su_res
#define s_op	s_u.su_op
#define s_pre	s_u.su_pre
#define s_lnk	s_u.su_lnk
#define s_pb	s_u.su_pb
#define s_seq	s_u.su_seq



/*
 *  Var -- an object in memory 
 *
 *  Basically, one entry for each item allocated in memory.  Includes named
 *  user variables, anonymous temporaries, array descriptors, etc.;  also used
 *  for fields in a parameter block or record.
 */

typedef enum e_variety {
    V_BOGUS,	/* error value */
    V_GLOBAL,	/* global */
    V_RPARAM,	/* resource parameter */
    V_RVAR,	/* resource variable */
    V_PARAM,	/* normal procedure parameter */
    V_REFNCE,	/* reference parameter */
    V_LOCAL,	/* local */
    V_RMBR	/* record member */
} Variety;

typedef struct var {
    Variety v_vty;		/* variable variety (V_xxxx) */
    Operator v_dcl;		/* how declared: O_{VAR,CON,FLD,PAR}DCL */
    struct symbol *v_sym;	/* symbol table entry, if any */
    struct sig *v_sig;		/* signature */
    struct node *v_value;	/* value, if compile-time constant */
    int v_seq;			/* serial number, for generating a name */
    Bool v_set, v_used;		/* is value set? is it used? */
} Var, *Varptr;

#define NULLVAR ((Varptr) 0)



/*  Param -- one entry in a parameter list  */

typedef struct param {
    char *m_name;		/* name, if any */
    int m_seq;			/* number */
    Sigptr m_sig;		/* signature */
    Operator m_pass;		/* how passed */
    struct param *m_next;	/* next parameter */
} Param, *Parptr;



/*  Proto -- operation and resource prototype (parameter list)  */

typedef struct proto {
    Operator p_rstr;		/* restrictions, or O_RESOURCE or O_FINAL */
    Parptr p_param;		/* first parameter (the return value) */
    char *p_def;		/* typedef name of parameter structure */
} Proto, *Proptr;



/*  Res -- information about a resource or global  */

typedef struct res {
    Operator r_opr;		/* O_RESOURCE or O_GLOBAL */
    Symptr r_sym;		/* symbol table entry */
    Sigptr r_sig;		/* signature */
    Symptr r_parm;		/* param block symtab node */
} Res, *Resptr;

#define NULLRES ((Resptr) 0)



/*  Op -- information about an operation
 *
 *  (Note that all parameter info is contained in the signature, o_usig.)
 *
 *  Op implementation field (o_impl) can mutate these ways:
 *	I_UNIMPL -> any but I_SEM   (on first usage)
 *	I_DCL -> I_CAP  (when capability used)
 *	I_CAP -> I_IN or I_PROC or I_EXTERNAL  (when implemented)
 *	I_IN -> I_SEM	(after all uses seen, if nothing prohibits sem impl)
 */

typedef enum e_seg {
    SG_BOGUS,
    SG_RESOURCE,/* declared at resource level */
    SG_PROC,	/* declared inside proc */
    SG_IMPORT,	/* imported from global */
    SG_GLOBAL	/* imported from other resource */
} Segment;

typedef enum e_impl {
    I_UNIMPL,	/* not yet implemented */
    I_EXTERNAL,	/* declared as external */
    I_PROC,	/* declared as proc */
    I_IN,	/* impl via in stmt(s) */
    I_SEM,	/* declared or optimized into a semaphore */
    I_CAP,	/* only capability used */
    I_DCL	/* impl not required -- just a declared op type */
} Impl;

typedef struct op {
    Symptr o_sym;		/* symbol table entry */
    Sigptr o_asig;		/* declared signature (including array etc.) */
    Sigptr o_usig;		/* underlying signature */
    Segment o_seg;		/* where resident */
    Impl o_impl;		/* how implemented (see above) */
    struct class *o_class;	/* class to which this op belongs */
    struct op *o_classmate;	/* next member of the class */
    Bool o_exported;		/* exported? */
    Bool o_dclsema;		/* declared as a semaphore? */
    Bool o_nosema;		/* true if something prevents semaphore impl */
    Bool o_sepctx;		/* true if op requires separate context */
    Bool o_madecap;		/* true if capability constructed */
    Bool o_incap;		/* true if capability used in input stmt */
} Op, *Opptr;

#define NULLOP ((Opptr) 0)



/*  Class -- operation class information
 *
 *  optab entry of each member in a class points back to this class header.
 *  c_members is useful for combining or for considering certain optimizations.
 */

typedef struct class {
    int c_num;			/* class number (0 indicates global) */
    int c_members;		/* member count */
    Opptr c_first;		/* first member */
    Segment c_seg;		/* SG_GLOBAL, SG_RESOURCE, or SG_PROC */
    Bool c_geninit;		/* has initialization been generated yet? */
} Class, *Classptr;

#define NULLCLASS ((Classptr) 0)



/*  Recdata -- data for implementing record types  */

typedef struct recdata {
    int k_size;			/* calculated size in bytes */
    char *k_tdef;		/* name of generated typedef */
    char *k_init;		/* name of initializer */
} Recdata, *Recptr;

#define NULLREC ((Recptr) 0)
