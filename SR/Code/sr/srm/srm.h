#define TRUE  1
#define FALSE 0

#define MF_BANNER   "#\tMakefile created by SRM\n"

typedef struct importListSt {
    int    resNum;
    struct importListSt *next;
} importList;

struct symtabSt {	/* symbol table entry */
    char  *compName;		/* resource/global name */
    char  *bodySource;		/* file containing component body */
    char  *specSource;		/* file containing component spec */
    char  global;		/* true if component is global */
    char  inClosure;		/* used for computing transitive closure */
    int   timesImported;	/* number of times component is imported/used */
    importList	*specImports;	/* list of components imported/used by spec */
    importList	*bodyImports;	/* list of components imported/used by body */
};
