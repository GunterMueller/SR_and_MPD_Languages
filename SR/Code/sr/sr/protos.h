/*  protos.h -- declarations of C functions used in the compiler  */

/* main.c */
extern	int	main		PARAMS ((int argc, char *argv[]));
extern	void	cc		PARAMS ((char *dir, char *fname));

/* alloc.c */
extern	void	initmem		PARAMS ((NOARGS));
extern	char*	ralloc		PARAMS ((int nbytes));
extern	char*	rsalloc		PARAMS ((char *s));

/* attest.c */
extern	Sigptr	attest		PARAMS ((Nodeptr e));	
extern	Nodeptr	subop		PARAMS ((Nodeptr l));

/* dynamic.c */
extern	void	initdynamic	PARAMS (());
extern	char*	alctemp		PARAMS ((Type t));
extern	void	retemp		PARAMS ((NOARGS));
extern	void	freetemp	PARAMS ((Type t, char *name));
extern	void	dcltemps	PARAMS ((NOARGS));
extern	char*	alctrans	PARAMS ((NOARGS));
extern	int	ntrans		PARAMS ((NOARGS));
extern	void	freetrans	PARAMS ((int count, int ch));
extern	void	notepst		PARAMS ((Symptr s));
extern	int	npst		PARAMS ((NOARGS));
extern	void	freepst		PARAMS ((int count, int forget));

/* errors.c */
extern	void	checkid		PARAMS ((Nodeptr e1, char *kwd, Nodeptr e2));
extern	int	yyerror		PARAMS ((char *s));
extern	void	handler		PARAMS ((int sgnl));
extern	void	assfail		PARAMS ((char *file, int locn));
extern	void	kboom		PARAMS ((char *s1, char *s2, char *f, int lno));
extern	void	err		PARAMS ((int line, char *text, char *param));
extern	void	errflush	PARAMS ((NOARGS));
extern	void	traceback	PARAMS ((Nodeptr e));
extern	void	tracedump	PARAMS ((NOARGS));

/* fold.c */
extern	Nodeptr	fold		PARAMS ((Nodeptr e));

/* gdecl.c */
extern	void	genvar		PARAMS ((Nodeptr e));

/* gexpr.c */
extern	void	gexpr		PARAMS ((Nodeptr e));
extern	void	gfexpr		PARAMS ((Nodeptr e));
extern	void	gaddr		PARAMS ((Nodeptr e));

/* ginput.c */
extern	void	ginput		PARAMS ((Nodeptr e));
extern	void	greceive	PARAMS ((Nodeptr e));

/* gmisc.c */
extern	Nodeptr	once		PARAMS ((Nodeptr e, int c));
extern	void	goctal		PARAMS ((int n));
extern	char*	csig		PARAMS ((Sigptr g));
extern	Bool	adouble		PARAMS ((Sigptr g));
extern	void	opdecl		PARAMS ((Opptr o));
extern	char*	sname		PARAMS ((Symptr s, int fullflag));
extern	void	fixtypes	PARAMS ((Nodeptr e));
extern	void	fixvalue	PARAMS ((Nodeptr e));
extern	void	gstride		PARAMS ((Nodeptr e, Sigptr g));
extern	void	garray		PARAMS ((Nodeptr e, Sigptr g));

/* gloop.c */
extern	void	qbegin		PARAMS ((Nodeptr l));
extern	void	qend		PARAMS ((Nodeptr l));
extern	void	bgnloop	PARAMS ((NOARGS));
extern	void	goloop		PARAMS ((Nodeptr e));
extern	void	nextloop	PARAMS ((NOARGS));
extern	void	endloop		PARAMS ((NOARGS));

/* gparam.c */
extern	char*	pbdef		PARAMS ((Symptr e));
extern	char*	fixedpb		PARAMS ((Proptr p));
extern	void	gparblk		PARAMS ((char *blk, Proptr p, Nodeptr actuals,
					Nodeptr qlist, Operator o));
extern	void	gparback	PARAMS ((Proptr p, Nodeptr a, char *pbvar));

/* grammar.c (from grammar.y) */
extern	Nodeptr	parse		PARAMS ((NOARGS));
extern	int	yyparse		PARAMS ((NOARGS));

/* gstmt.c */
extern	void	gstmt		PARAMS ((Nodeptr e));

/* import.c */
extern	void	initimp		PARAMS ((char *rname));
extern	void	import		PARAMS ((Nodeptr e));
extern	Nodeptr	readspec	PARAMS ((Nodeptr id, int flags));

/* input.c */
extern	int	gettok		PARAMS ((NOARGS));
extern	void	updtloc		PARAMS ((NOARGS));
extern	void	start_echo	PARAMS ((Nodeptr e));
extern	void	stop_echo	PARAMS ((Nodeptr e));
extern	void	abort_echo	PARAMS ((NOARGS));
extern	void *	setinput	PARAMS ((FILE *spec));
extern	void	resetinput	PARAMS ((void *old));

/* list.c */
extern	List	list		PARAMS ((int n));
extern	char*	lfirst		PARAMS ((List l));
extern	char*	lpush		PARAMS ((List l));
extern	char*	lpop		PARAMS ((List l));
extern	char*	lput		PARAMS ((List l));
extern	char*	ldel		PARAMS ((List l, char *e));

/* names.c */
extern	void	initnames	PARAMS ((NOARGS));
extern	char*	unique		PARAMS ((char *name));
extern	char*	genref		PARAMS ((char *name));

/* node.c */
extern	Nodeptr	newnode		PARAMS ((Operator opr));
extern	Nodeptr	bnode		PARAMS ((Operator opr, Nodeptr l, Nodeptr r));
extern	Nodeptr	unode		PARAMS ((Operator opr, Nodeptr r));
extern	Nodeptr	idnode		PARAMS ((char *name));
extern	Nodeptr	intnode		PARAMS ((int n));
extern	Nodeptr	realnode	PARAMS ((Real v));
extern	Nodeptr	vbmnode		PARAMS ((char *text, Sigptr g));
extern	Nodeptr	refnode		PARAMS ((char *tmpname, Sigptr g));
extern	Nodeptr	parmnode	PARAMS ((char *cast, char *p, int n, Sigptr g));
extern	Nodeptr	blist		PARAMS ((Operator opr, Nodeptr l, Nodeptr r));
extern	Nodeptr	lcat		PARAMS ((Nodeptr list1, Nodeptr list2));
extern	Nodeptr	nreplace	PARAMS ((Nodeptr e, char *cast, char *tname));
extern	Nodeptr	indx		PARAMS ((Nodeptr l, Nodeptr r));
extern	Nodeptr	mkarray		PARAMS ((Nodeptr l, Nodeptr r));

/* op.c */
extern	void	initops		PARAMS ((NOARGS));
extern	Opptr	digop		PARAMS ((Sigptr g));
extern	Opptr	newop		PARAMS ((Symptr s, Segment seg));
extern	void	instmt		PARAMS ((Nodeptr e));
extern	void	globops		PARAMS ((NOARGS));
extern	void	opscan		PARAMS ((NOARGS));
extern	void	genclass	PARAMS ((NOARGS));
extern	void	genops		PARAMS ((NOARGS));
extern	void	makeop		PARAMS ((Opptr o));
extern	void	makesemop	PARAMS ((Opptr o, Nodeptr e));
extern	void	dumpops		PARAMS ((NOARGS));
extern	void	dumpclass	PARAMS ((NOARGS));
extern	void	wimpl		PARAMS ((char *globalname));
extern	void	rimpl		PARAMS ((Nodeptr e));

/* output.c */
extern	void	mkinter		PARAMS ((NOARGS));
extern	void	copen		PARAMS ((char *fname));
extern	void	cdivert		PARAMS ((int n));
extern	void	undivert	PARAMS ((int n));
extern	void	cprintf		PARAMS ((char *fmt, ...));
extern	void	cflush		PARAMS (());
extern	void	cclose		PARAMS ((NOARGS));	
extern	void	setstream	PARAMS ((int n));
extern	void	wescape		PARAMS ((FILE *fp, char *s, int n, int c));

/* params.c */
extern	void	initprotos	PARAMS ((NOARGS));
extern	Proptr	prototype	PARAMS ((Nodeptr e));
extern	void	protoname	PARAMS ((Proptr p, Symptr s));
extern	void	genprotos	PARAMS ((NOARGS));
extern	Proptr	eproto		PARAMS ((Nodeptr e));
extern	void	dumpprotos	PARAMS ((NOARGS));

/* pregen.c */
extern	void	pregen		PARAMS ((Nodeptr e));

/* presig.c */
extern	Sigptr	presig		PARAMS ((Nodeptr e));

/* print.c */
extern	void	ptree		PARAMS ((Nodeptr e));
extern	void	pstab		PARAMS ((Symptr s));
extern	void	psig		PARAMS ((Sigptr g));
extern	void	psym		PARAMS ((Symptr s));
extern	char*	kindtos		PARAMS ((Kind k));
extern	char*	oprtos		PARAMS ((Operator opr));
extern	char*	symtos		PARAMS ((Symptr s));
extern	char*	typetos		PARAMS ((Type t));
extern	char*	varitos		PARAMS ((Variety v));

/* resource.c */
extern	void	resource	PARAMS ((Nodeptr e));

/* signat.c */
extern	void	initsig		PARAMS ((NOARGS));
extern	Sigptr	newsig		PARAMS ((Type t, Sigptr usig));
extern	Sigptr	symsig		PARAMS ((Symptr s));
extern	Bool	easylval	PARAMS ((Nodeptr e));
extern	Bool	lvalue		PARAMS ((Nodeptr e));
extern	Bool	compat		PARAMS ((Sigptr g1, Sigptr g2, Compat c));
extern	int	ndim		PARAMS ((Sigptr g));
extern	Sigptr	usig		PARAMS ((Sigptr g));

/* symtab.c */
extern	void	initsym		PARAMS ((NOARGS));
extern	void	identify	PARAMS ((Nodeptr e));
extern	void	dumpsyms	PARAMS ((int wantpredefs));
extern	Varptr	newvar	PARAMS ((Symptr s, Variety v, Operator dcl, Sigptr g));

/* tokens.c (from tokens.l) */
extern	int	yylex		PARAMS ((NOARGS));
extern	char *	yyref		PARAMS ((NOARGS));
extern	void *	lexfrom		PARAMS ((FILE *fp));
extern	void	lexrevert	PARAMS ((void *old));
