#include <stdio.h>
#include "../config.h"
#include "../gen.h"
#include "../paths.h"
#include "../util.h"
#include "srm.h"

struct	symtabSt symtab[MAX_RES_DEF + 1];
int	numResources;

char	*compiler = NULL;		/* sr compiler to use */
char	*executable = "a.out";		/* name of the executable */
char	*makefileName = "Makefile";	/* name of the makefile */
char	*interfaceDir = NULL;		/* name of the interfaces dir */
char	*mainResName = NULL;		/* name of the main resource */
char	*compileOptions = "";		/* options to the sr compiler */
char	*linkerOptions = "";		/* options for srl */
char	*runtimeArgs = "";		/* arguments for the executable */
char	verbose = FALSE;		/* turns on/off srm verbose mode */
char	widelines = FALSE;		/* output wide lines? */
char	*zapFiles = NULL;		/* files deleted by make cleanx */
char	**sourceFiles;			/* sr source files processes by srm */
char	**otherSrc = NULL;		/* other source/binary files named */
char	**otherBin = NULL;		/* same, in .o form, NULL if no depend*/

/* main program */

main (argc, argv)
int	argc;
char	**argv;
{
    int		opt;
    extern	char *optarg;
    extern	int  optind;
    char	tempString[MAX_PATH];
    char	*getarg ();
    char	*env, *option;

    /* set exit message prefix */
    xprefix = argv[0];

    /* process options in environment SRMOPTS */
    if (env = getenv ("SRMOPTS"))
	while (option = strchr (env, '-')) {
	    switch (* (++option)) {
		case 'f':
		    makefileName = getarg (option);
		    break;
		case 'I':
		    interfaceDir = getarg (option);
		    break;
		case 'o':
		    executable = getarg (option);
		    break;
		case 'c':
		    compiler = getarg (option);
		    break;
		case 'm':
		    mainResName = getarg (option);
		    break;
		case 'v':
		    verbose = TRUE;
		    break;
		case 'w':
		    widelines = TRUE;
		    break;
		case 'C':
		    compileOptions = getarg (option);
		    break;
		case 'L':
		    linkerOptions = getarg (option);
		    break;
		case 'R':
		    runtimeArgs = getarg (option);
		    break;
		case 'Z':
		    zapFiles = getarg (option);
		    break;
		default:
		    usageMessage ();	/* does not return */
	    }
	    env = option;
	}

    /* process command line options */
    /* the "+" getopt arg works around gratuitous GNU getopt incompatibility */
    while ((opt = getopt (argc, argv, "+f:I:o:c:m:vwC:L:R:Z:")) != EOF)
	switch (opt) {
	    case 'f':
		makefileName = optarg;
		break;
	    case 'I':
		interfaceDir = optarg;
		break;
	    case 'o':
		executable = optarg;
		break;
	    case 'c':
		compiler = optarg;
		break;
	    case 'm':
		mainResName = optarg;
		break;
	    case 'v':
		verbose = TRUE;
		break;
	    case 'w':
		widelines = TRUE;
		break;
	    case 'C':
		compileOptions = optarg;
		break;
	    case 'L':
		linkerOptions = optarg;
		break;
	    case 'R':
		runtimeArgs = optarg;
		break;
	    case 'Z':
		zapFiles = optarg;
		break;
	    case '?':
	    default:
		usageMessage ();
		/* NOTREACHED */
	}

    if (verbose)
	trcexec = 1;

    if (!compiler) {		/* compiler name unspecified - use default */
	sprintf (tempString, "%s/sr", SRCMD);
	compiler = salloc (tempString);
    }

    setpath (interfaceDir ? interfaceDir : ".", 0); /* init for path searches */
    buildSymtab (argc, argv, optind);	/* construct the symbol table */
    if (!checkTable ())			/* check symbol table for errors */
	exit (1);
    if (verbose)
	printStats ();
    computeTransitiveClosure ();	/* compute complete dependency lists */
    writeMakefile (argv);		/* write out makefile */

    exit (0);
    /*NOTREACHED*/
}


usageMessage ()
{
    fprintf (stderr, 
"usage: srm [-{fIoc} path] [-m res] [-{CLRZ} args] [-{vw}] file.sr... [f.o...]\n");
    exit (1);
}


/* picks out a word from the given string and advances the string pointer to
 * the end of the word.  The word is extracted and returned.  Note the scanning
 * does not start at the start of the given string but one character after.
 */

char	*getarg (envStr)
char	*envStr;
{
    int		i, j;
    char	*retval;

    for (i = 1; * (envStr + i) == ' '; i++)
	;	/* skip over spaces */
    for (j = i; * (envStr + j) != ' ' && * (envStr + j) != '\0'; j++)
	;	/* got to end of word -- get word size */

    retval = alloc (j - i + 1);
    strncpy (retval, envStr + i, j - i);
    retval[j-i] = '\0';
    envStr += j;	/* set envStr to point to after the word */
    return retval;
}
