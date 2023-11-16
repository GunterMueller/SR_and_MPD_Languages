#include <stdio.h>
#include <sys/wait.h>
#include "../config.h"
#include "../gen.h"
#include "../util.h"
#include "mpdm.h"

extern	struct	symtabSt symtab[MAX_RES_DEF + 1];
extern	int	numResources;
extern	char	*compiler;
extern	char	**otherBin;
extern	char	**sourceFiles;
extern	char	**otherSrc;

/* processes the mpdm command line after the options.  The mpd compiler is
 * invoked with the -M option and .mpd files listed on the command line to get
 * import information.	The information obtained is recorded in the symbol
 * table.
 */
buildSymtab (argc, argv, optind)
int	argc;
char	**argv;
int	optind;
{
    char inputline[MAX_PATH];
    char compType[10];		/* component type: "global" or "resource" */
    char compName[MAX_PATH];	/* component name */
    char bodyOrSpec[5];		/* keyword "body" or "spec" */
    char compOrFileName[MAX_PATH]; /* name of component or file name */
    char sourceOrImport[8];	/* keyword "source" or "import" */
    char processingSpec;	/* flag. TRUE if spec is being processed */
    char **nargv;		/* command line for mpd compiler */
    char c, *s, buf[MAX_PATH];
    int  i, j, n;
    int  numMPDfiles;		/* number of mpd files on mpdm command line */
    int  numOtherfiles;		/* number of non-mpd files on command line */
    FILE *f;
    importList	*import;
    int status;			/* used to determine exit status of compiler */

    struct {			/* cache last component seen */
	char  *compName;	   /* component name */
	int   symtabEntry;	   /* position of component in symbol table */
    } cache;

    numResources = 0;
    cache.compName = "";

    /* optind points to the first mpd file name.  Count the  number of mpd files
     * listed on the command line.  Form command line for mpd compiler.
     */
    for (i = optind, numMPDfiles = 0;
    	i < argc && strtail (argv[i], ".mpd");
	i++, numMPDfiles++)
	    ;
    if (numMPDfiles == 0) {
	fprintf (stderr, "No mpd files specified\n");
	usageMessage ();
	/* NOTREACHED */
    }

    /* form argument list for mpd compiler */
    nargv = (char **) alloc ((numMPDfiles + 4) * sizeof (char *));
    nargv[0] = compiler;
    nargv[1] = "-w";
    nargv[2] = "-M";
    for (i = 0; i < numMPDfiles; i++)
	nargv[3+i] = argv[optind+i];
    nargv[3+i] = NULL;

    optind = optind + numMPDfiles; /*  optind points to rest of command line */
    sourceFiles = nargv + 3;	  /* list of source files (starts at third
				   * argument to compiler; ends with null)
				   */

    if (optind < argc) {
	/* process remaining command line arguments.  All arguments assumed to
	 * be files or libraries to be linked into mpd executable.
	 */
	numOtherfiles = argc - optind;
	otherBin = (char **) alloc ((numOtherfiles + 1) * sizeof (char *));
	otherSrc = (char **) alloc ((numOtherfiles + 1) * sizeof (char *));
	for (i = 0; i < numOtherfiles; i++) {
	    s = argv[optind + i];
	    otherSrc[i] = s;		/* file name as given */
	    otherBin[i] = salloc (s);	/* may get altered or zapped*/

	    if (strncmp (s, "-l", 2) == 0) {
		otherBin[i] = NULL;
	    } else {
		n = strlen (s);
		if (n < 3 || s[n-2] != '.') {
		    /* not of the form xxxxx.x */
		    fprintf (stderr, "mpdm: invalid filename: %s\n", s);
		    exit (1);
		}
		c = s[n-1];
		switch (c) {
		    case 'a':
		    case 'o':
			if (f = fopen (s, "r")) {
			    /* file is in this directory; create dependency */
			    fclose (f);
			} else if (f = pathopen (s, "", buf)) {
			    /* file is found on MPD_PATH; don't create */
			    fclose (f);
			    otherBin[i] = NULL;
			}
			/* else */
			    /* assume it will be built in this directory */
			break;
		    default:
			/* .c or .p or .s or something: depend on .o variant */
			otherBin[i][n-1] = 'o';
			break;
		}
	    }
	}
	otherSrc[i] = otherBin[i] = NULL;
    }

    if (filter (0, compiler, nargv) == 0)	/* run compiler into stdin */
	mexit ("can't fork compiler");

    while (fgets (inputline, MAX_PATH, stdin)) {
	sscanf (inputline, "%s %s %s %s %s",
	    compType, compName, bodyOrSpec, sourceOrImport, compOrFileName);
	if (strcmp (cache.compName, compName) != 0) {
	    /* component name not in cache -- must search symtab */
	    symtab[numResources].compName = compName;
	    for (i = 0; strcmp (symtab[i].compName, compName) != 0; i++)
		;
	    if (i == numResources) {
		/* haven't seen this component name before */
		symtab[numResources].compName = salloc (compName);
		numResources++;
	    }
	    /* set resource type now -- may not have been set on earlier ref */
	    if (strcmp (compType, "resource") == 0)
		symtab[i].global = FALSE;
	    else
		symtab[i].global = TRUE;
	    cache.compName = symtab[i].compName;
	    cache.symtabEntry = i;
	}
	else	/* cache has entry for resource */
	    i = cache.symtabEntry;
	if (strcmp (bodyOrSpec, "spec") == 0)
	    processingSpec = TRUE;
	else
	    processingSpec = FALSE;
	if (strcmp (sourceOrImport, "source") == 0)
	    if (processingSpec)
		if (symtab[i].specSource) {
		    if (strcmp (symtab[i].specSource, compOrFileName) != 0)
			fprintf (stderr,
		"mpdm: spec for resource %s defined in files %s and %s.\n",
			    compName, compOrFileName, symtab[i].specSource);
		    else
			fprintf (stderr,
		"mpdm: multiple definitions for resource %s in file %s.\n",
			    compName, compOrFileName);
		    exit (1);
		}
		else
		    symtab[i].specSource = salloc (compOrFileName);
	    else
		if (symtab[i].bodySource) {
		    fprintf (stderr,
		"mpdm: body for resource %s defined in files %s and %s.\n",
			    compName, compOrFileName, symtab[i].bodySource);
		    exit (1);
		}
	    else
		symtab[i].bodySource = salloc (compOrFileName);
	else {
	    /* processing an import clause.  See if symtab has entry
	     * for component being imported.
	     */
	    symtab[numResources].compName =  compOrFileName;
	    for (j = 0; strcmp (symtab[j].compName, compOrFileName) != 0; j++)
		;
	    if (j == numResources) {
		/* add imported component to symtab */
		symtab[numResources].compName = salloc (compOrFileName);
		numResources++;
	    }
	    symtab[j].timesImported++;

	    /* add the imported component to the list of components
	     * imported by the component being processed
	     */
	    if (processingSpec) {
		/* component imported by spec */
		import = symtab[i].specImports;
		while (import != NULL && import->resNum != j)
		    import = import->next;
		if (import == NULL) {
		    import = (importList *) alloc (sizeof (importList));
		    import->resNum = j;
		    import->next = symtab[i].specImports;
		    symtab[i].specImports = import;
		}
	    }
	    else {
		/* component imported by body */
		import = symtab[i].bodyImports;
		while (import != NULL && import->resNum != j)
		    import = import->next;
		if (import == NULL) {
		    import = (importList *) alloc (sizeof (importList));
		    import->resNum = j;
		    import->next = symtab[i].bodyImports;
		    symtab[i].bodyImports = import;
		}
	    }
	}
    }

    /* check compiler's exit status */
    if (wait (&status) == -1)
	pexit ("mpdm");
    if (status != 0)
	mexit ("exiting due to compilation errors");
    return;
}
