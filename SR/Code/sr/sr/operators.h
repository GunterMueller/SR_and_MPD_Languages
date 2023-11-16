/*  operators.h -- list of operators used to distinguish tree nodes
 *
 *  usage of macro fields:
 *
 *	name	operator name  (turns into C definiton "O_name")
 *	left	usage of left child:  CH_{NIL,NODE,SYM,VAR,NAME,INT,REAL,STR}
 *
 *  The includer of this file must #define the OPR macro as desired.
 */



/*  name,	leftchild  */



/* general purpose nodes */

OPR (BOGUS,	CH_NIL)		/* code 0 illegal */

/* organizational nodes */
OPR (LIST,	CH_NODE)	/* item list, descends only to right */
OPR (SEQ,	CH_NODE)	/* stmt list, must traverse for correct order */

/* identifiers */
OPR (ID,	CH_NAME)	/* identifier, unresolved */
OPR (SYM,	CH_SYM)		/* identifier, resolved to symtab entry */
OPR (TYPENAME,	CH_NODE)	/* l: id, must be type; r: nz if new(optype) */
OPR (SUBS,	CH_NODE)	/* l: id; r: bounds */
OPR (BOUNDS,	CH_NODE)	/* l: lowerbound r: upperbound */
OPR (VERBATIM,	CH_NAME)	/* l: verbatim C code to output; r: previous */

/* literals & constants */
OPR (ILIT,	CH_INT)		/* integer */
OPR (RLIT,	CH_REAL)	/* real */
OPR (BLIT,	CH_INT)		/* boolean , 0=false, 1=true */
OPR (CLIT,	CH_INT)		/* character */
OPR (SLIT,	CH_STR)		/* string  (r: string number) */
OPR (NLIT,	CH_INT)		/* nxxx, 0=null, -1=noop */
OPR (FLIT,	CH_INT)		/* file, 0=stdin, 1=stdout, 2=stderr */



/* statements */

/* top level declarations */
OPR (COMPONENT,	CH_NODE)	/* l: id; r: GLOBAL or RESOURCE */
OPR (GLOBAL,	CH_NODE)	/* l: spec; r: body */
OPR (RESOURCE,	CH_NODE)	/* l: spec; r: body */
OPR (SPEC,	CH_NODE)	/* l: decls; r: params */
OPR (BODY,	CH_NIL)		/* r: stmts */
OPR (SEPARATE,	CH_NIL)		/* l,r: 0 */

/* import and extend */
OPR (EXTEND,	CH_NODE)	/* l: id; r: imported stmts */
OPR (IMPORT,	CH_NODE)	/* l: id; r: imported stmts */

/* op declarations */
OPR (OP,	CH_NODE)	/* l: idlist; r: ID or prototype */
OPR (EXTERNAL,	CH_NODE)	/* l: idlist; r: ID or prototype */
OPR (OPTYPE,	CH_NODE)	/* l: ID; r: PROTO */
OPR (PROTO,	CH_NODE)	/* l: params; r: restrictions */
OPR (SEM,	CH_NODE)	/* l: OP; r: initialval */

/* op implementations */
OPR (PROC,	CH_NODE)	/* l: paramlist; r: BLOCK */
OPR (PROCESS,	CH_NODE)	/* l: SEQ(OP,PROC); r: quant list */
OPR (FINAL,	CH_NODE)	/* l: BLOCK */

/* other declarations */
OPR (TYPE,	CH_NODE)	/* l: ID; r: type */
OPR (PARDCL,	CH_NODE)	/* l: idlist; r: PARATT */
OPR (VARDCL,	CH_NODE)	/* l: idlist; r: VARATT */
OPR (CONDCL,	CH_NODE)	/* l: idlist; r: VARATT */
OPR (FLDDCL,	CH_NODE)	/* l: idlist; r: VARATT */
OPR (PARATT,	CH_NODE)	/* l: type; r: paramkind */
OPR (VARATT,	CH_NODE)	/* l: type; r: initialval */

/* statement organization */
OPR (BLOCK,	CH_NODE)	/* l: [id]; r: stmts */
OPR (GUARD,	CH_NODE)
OPR (QUANT,	CH_NODE)	/* l: QSTEP; r: QTEST */
OPR (QSTEP,	CH_NODE)	/* l: initial; r: step */
OPR (QTEST,	CH_NODE)	/* l: loop test; r: suchthat test */

/* control statements */
OPR (IF,	CH_NODE)
OPR (DO,	CH_NODE)
OPR (FA,	CH_NODE)

OPR (SKIP,	CH_NIL)
OPR (STOP,	CH_NODE)
OPR (EXIT,	CH_NIL)
OPR (NEXT,	CH_NIL)

OPR (RETURN,	CH_NIL)
OPR (REPLY,	CH_NIL)

/* invocation operators */
OPR (LIBCALL,	CH_NODE)	/* l: id; r: parlist */    /* usable as expr */
OPR (CALL,	CH_NODE)	/* l: op; r : parlist */   /* usable as expr */
OPR (SEND,	CH_NODE)	/* l: op; r : parlist */
OPR (FORWARD,	CH_NODE)	/* l: op; r : parlist */
OPR (RECEIVE,	CH_NODE)	/* l: INOP; r: parlist */ 

/* create operators */
OPR (CREVM,	CH_NODE)	/* l: CREVOKE; r: ON node */   /* usable expr */
OPR (CREATE,	CH_NODE)	/* l: CREVOKE; r: ON node */   /* usable expr */
OPR (CREVOKE,	CH_NODE)	/* l: id | VM; r: parlist */
OPR (DESTROY,	CH_NODE)

/* input statements */
OPR (IN,	CH_NODE)	/* l: LIST; r: elseblock */
OPR (ARM,	CH_NODE)	/* l: SCHED; r: block */
OPR (SCHED,	CH_NODE)	/* l: SYNC; r: scheduling expr */
OPR (SYNC,	CH_NODE)	/* l: SELECT; r: boolean expr */
OPR (SELECT,	CH_NODE)	/* l: QUANT; r: INPARM */
OPR (INPARM,	CH_NODE)	/* l: INOP; r: paramlist */
OPR (INOP,	CH_NODE)	/* l: null; r: id */

/* co statements  */
OPR (CO,	CH_NODE)	/* l: list */
OPR (COINV,	CH_NODE)	/* l: COSEL; r: postprocessing */ 
OPR (COSEL,	CH_NODE)	/* l: QUANT; r: stmt */



/* types */

OPR (CAP,	CH_NODE)
OPR (PTR,	CH_NODE)
OPR (BOOL,	CH_NIL)
OPR (CHAR,	CH_NIL)
OPR (INT,	CH_NIL)
OPR (ENUM,	CH_NODE)
OPR (REAL,	CH_NIL)
OPR (FFILE,	CH_NIL)		/* don't use FILE, conflicts w/ stdio */
OPR (STRING,	CH_NODE)
OPR (REC,	CH_NODE)
OPR (ARRAY,	CH_NODE)
OPR (VM,	CH_NIL)
OPR (ANY,	CH_NIL)
OPR (VOID,	CH_NIL)		/* implicit return type */



/* expressions */

/* pseudo-functions that take a type as an argument */
OPR (NEW,	CH_NODE)	/* new(type) */
OPR (NEWOP,	CH_NODE)	/* new(op) */
OPR (LOW,	CH_NODE)
OPR (HIGH,	CH_NODE)
OPR (CAST,	CH_NODE)	/* l: type; r: value */

/* unary operators */
OPR (NOT,	CH_NODE)
OPR (POS,	CH_NODE)
OPR (NEG,	CH_NODE)
OPR (ADDR,	CH_NODE)
OPR (QMARK,	CH_NODE)
OPR (PREINC,	CH_NODE)
OPR (PREDEC,	CH_NODE)
OPR (POSTINC,	CH_NODE)
OPR (POSTDEC,	CH_NODE)
OPR (PTRDREF,	CH_NODE)

/* binary arithmetic operators */
OPR (EXPON,	CH_NODE)
OPR (MUL,	CH_NODE)
OPR (DIV,	CH_NODE)
OPR (MOD,	CH_NODE)
OPR (REMDR,	CH_NODE)
OPR (ADD,	CH_NODE)
OPR (SUB,	CH_NODE)
OPR (CONCAT,	CH_NODE)
OPR (EQ,	CH_NODE)
OPR (NE,	CH_NODE)
OPR (GE,	CH_NODE)
OPR (LE,	CH_NODE)
OPR (GT,	CH_NODE)
OPR (LT,	CH_NODE)
OPR (AND,	CH_NODE)
OPR (OR,	CH_NODE)
OPR (XOR,	CH_NODE)
OPR (RSHIFT,	CH_NODE)
OPR (LSHIFT,	CH_NODE)

/* subscripting operators */
OPR (FIELD,	CH_NODE)	/* l: expr;  r: id */
OPR (INDEX,	CH_NODE)	/* l: array; r: subscript */
OPR (SLICE,	CH_NODE)	/* l: array; r: list of bounds */
OPR (RANGE,	CH_NODE)	/* l: lowerbound r: upperbound */

/* construction operators */
OPR (RECCONS,	CH_NODE)	/* l: id;    r: list */
OPR (CLONE,	CH_NODE)	/* l: count; r: obj */
OPR (ARCONS,	CH_NODE)	/* l: list   (array constructor) */

/* assignment operators */
OPR (ASSIGN,	CH_NODE)
OPR (SWAP,	CH_NODE)
OPR (AUG_ADD,	CH_NODE)
OPR (AUG_SUB,	CH_NODE)
OPR (AUG_MUL,	CH_NODE)
OPR (AUG_EXPON,	CH_NODE)
OPR (AUG_DIV,	CH_NODE)
OPR (AUG_REMDR,	CH_NODE)
OPR (AUG_AND,	CH_NODE)
OPR (AUG_OR,	CH_NODE)
OPR (AUG_CONCAT, CH_NODE)
OPR (AUG_RSHIFT, CH_NODE)
OPR (AUG_LSHIFT, CH_NODE)



/* nodes used to indicate presence of syntactic tokens */

OPR (ASTER,	CH_NIL)		/*   bound value of `*' */

/* restrictions */
OPR (RCALL,	CH_NIL)
OPR (RSEND,	CH_NIL)
OPR (RNONE,	CH_NIL)

/* argtypes */
OPR (VAL,	CH_NIL)
OPR (VAR,	CH_NIL)
OPR (RES,	CH_NIL)
OPR (REF,	CH_NIL)
