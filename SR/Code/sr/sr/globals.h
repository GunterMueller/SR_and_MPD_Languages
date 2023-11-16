/*  globals.h -- definitions of global variables  */

#ifndef GLOBAL
#define GLOBAL extern
#define INIT(v)
#endif



/*  command line options  */

GLOBAL Bool option_d;		/* any debugging selected? */
GLOBAL char dbflags[256];	/* debugging flags */

GLOBAL Bool option_b;		/* generate body */
GLOBAL Bool option_s;		/* generate specs */
GLOBAL Bool option_c;		/* compile only, no link */
GLOBAL Bool option_e;		/* experimental srl & runtime system */
GLOBAL Bool option_q;		/* don't print file names as compiled */
GLOBAL Bool option_g;		/* compile with -g for dbx, don't rm *.[ch] */
GLOBAL Bool option_v;		/* verbose mode */
GLOBAL Bool option_w;		/* don't warn about obsolete specs */
GLOBAL Bool option_C;		/* don't call cc(1) */
GLOBAL Bool option_F;		/* don't fold constants (may break some code) */
GLOBAL Bool option_P;		/* pessimize code (don't apply any SR opts) */
GLOBAL Bool option_M;		/* output make(1) dependencies */
GLOBAL Bool option_O;		/* call cc(1) with -O */
GLOBAL Bool option_T;		/* ``Turbo'' turns off time-slicing in loops */



/*  files  */

GLOBAL char *ifdir;		/* name of the Interfaces directory */

GLOBAL Bool allow_echo;		/* allow echoing to .spec file? */

GLOBAL char *srcname[MAX_SRC];	/* .sr file name (in [0]), .spec file names */
GLOBAL int srclocn;		/* current line number & file index */

GLOBAL int newlines;		/* count of newlines consumed by yylex() */

GLOBAL char cwd[MAX_PATH];	/* current working directory */



/*  resource information  */

GLOBAL char *resname;			/* current resource name */
GLOBAL Resptr curres;			/* current resource entry */
GLOBAL Proptr curproto;			/* current operation's prototype */

GLOBAL int indepth;			/* depth of input stmt nesting */
GLOBAL int inret[MAX_NESTING];		/* labels for returning from input */
GLOBAL int intmp[MAX_NESTING];		/* temps to free returning from in */
GLOBAL char *invptr[MAX_NESTING];	/* invocation block pointer name */

GLOBAL char *rlist[MAX_RES_DEF+1];	/* resource list */
GLOBAL char **rlp INIT (rlist);		/* resource list pointer */

GLOBAL char *ilist[MAX_RES_DEF+1];	/* import list */
GLOBAL char **ilp INIT (ilist);		/* import list pointer */

GLOBAL int nextlabel;			/* next code label to generate */



/*  errors  */

GLOBAL int nwarnings;		/* number of warning errors in current file */
GLOBAL int nfatals;		/* number of fatal errors in current file */
GLOBAL int tfatals;		/* total of fatal errors everywhere */



/*  signatures and types  */

GLOBAL int tflags[(int) TCOUNT];	/* flags for type attributes */
GLOBAL int tsize[(int) TCOUNT];		/* size, iff fixed for type */

GLOBAL Sigptr void_sig;
GLOBAL Sigptr any_sig;
GLOBAL Sigptr nlit_sig;

GLOBAL Sigptr bool_sig;
GLOBAL Sigptr char_sig;
GLOBAL Sigptr int_sig;
GLOBAL Sigptr real_sig;
GLOBAL Sigptr file_sig;

GLOBAL Sigptr array_sig;	/* array of chars of unknown size */
GLOBAL Sigptr string_sig;	/* string of unknown size */
GLOBAL Sigptr ptr_sig;		/* ptr any */

GLOBAL Sigptr mode_sig;		/* enum (READ, WRITE, READWRITE) */
GLOBAL Sigptr seek_sig;		/* enum (ABSOLUTE, RELATIVE, EXTEND) */

GLOBAL Sigptr vcap_sig;



/*  defined in node.c  */

extern Child leftchild [];	/* child variant of each node type */
