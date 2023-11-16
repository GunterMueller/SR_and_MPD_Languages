#include <stdio.h>
#include "../config.h"
#include "../gen.h"
#include "../util.h"
#include "mpdm.h"

extern	struct symtabSt symtab[MAX_RES_DEF + 1];
extern	int    numResources;

extern	char	*compiler;
extern	char	*executable;
extern	char	*makefileName;
extern	char	*interfaceDir;
extern	char	*mainResName;
extern	char	*compileOptions;
extern	char	*linkerOptions;
extern	char	*runtimeArgs;
extern	char	verbose;
extern	char	widelines;
extern	char	*zapFiles;
extern	char	**otherBin;
extern	char	**otherSrc; 
extern	char	**sourceFiles;

extern	int	fillPos;

/* writes out the makefile */
writeMakefile (argv)
char	**argv;
{
    int		i, mainRes, timesImported;
    FILE	*makefile;
    importList	*import;
    char	buffer[80];

    if (strcmp (makefileName, "-") == 0)
	makefile = stdout;
    else {
	/* check if the makefile will overwrite a file not created by mpdm */
	if (makefile = fopen (makefileName, "r")) {
	    if (fgets(buffer,80,makefile)!=0 && strcmp(MF_BANNER,buffer)!=0) {
		fprintf (stderr,
"mpdm: \"%s\" exists and was not created by MPDM - will not overwrite\n",
		    makefileName);
		exit (1);
	    }
	    fclose (makefile);
	}
	if ((makefile = fopen (makefileName, "w")) == NULL) {
	    perror (makefileName);
	    exit (1);
	}
    }

    if (numResources == 0)		/* handle "mpdm emptyfile.mpd" case */
	mexit ("no resources found");

    if (!mainResName) {
	/* main resource not specified.  First resource that is imported by
	 * the fewest number of resources is assumed to be the main resource.
	 */
	mainRes = 0;
	timesImported = MAX_RES_DEF + 1;
	for (i = 0; i < numResources; i++)
	    if (!symtab[i].global && timesImported > symtab[i].timesImported) {
		mainRes = i;
		timesImported = symtab[i].timesImported;
	    }
    }
    else {
	/* main resource specified. Find position of this resource in symtab */
	symtab[numResources].compName = mainResName;
	for (i = 0; strcmp (symtab[i].compName, mainResName) != 0; i++)
	    ;
	if (i == numResources) {
	    fprintf (stderr, "mpdm: main resource %s not found\n", mainResName);
	    exit (1);
	}
	mainRes = i;
    }

    if (verbose)
	printf ("Main Resource: %s\n", symtab[mainRes].compName);

    /* Check if there is at least a single resource with a body
     * Otherwise we abort mpdm */
    for (i = 0; i < numResources; i++)
	if (symtab[i].bodySource)
	    break;			/* Ahh! a component with a body */
    if (i >= numResources)
	mexit ("no resource has a body");
     
    /* write out Makefile */
    fprintf (makefile, "%s\n\n", MF_BANNER);
    fprintf (makefile, "MPD = mpd\n");
    if (interfaceDir)
	fprintf (makefile, "MPDFLAGS = -I %s %s\n", interfaceDir,
	    compileOptions);
    else
	fprintf (makefile, "MPDFLAGS = %s\n", compileOptions);

    fprintf (makefile, "I = %s/MPDinter\n\n",
	(interfaceDir) ? interfaceDir : ".");
    fillPos = 0;

    /* prevent weird behavior, e.g.,
     * if a.mpd.r exists (taken to be a Ratfor file),
     * don't want to invoke f77 on it!
     */
    fprintf (makefile, ".SUFFIXES:\n\n");

    /* make link.  All .o files created by the compiler are linked with other
     * files and libraries specified on the mpdm command line.
     */
    fill1 (makefile, "link: %s\n\n", executable); 
    fillPos = 0; 

    fill1 (makefile, "%s:", executable);
    for (i = 0; i < numResources; i++)
	if (symtab[i].bodySource)
	    /* only components with bodies will have .o files */
	    fill1 (makefile, " $I/%s.o", symtab[i].compName);
    if (otherBin)
	for (i = 0; otherSrc[i]; i++)
	    if (otherBin[i])
		fill1 (makefile, " %s", otherBin[i]);
    fill0 (makefile, "\n");
    
    fillPos = TAB_WIDTH;
    fill2 (makefile, "\tmpdl %s -o %s", linkerOptions, executable);
    if (interfaceDir)
	fill1 (makefile, " -I %s", interfaceDir);
    for (i = 0; i < numResources; i++)
	if (i != mainRes && (symtab[i].bodySource || !symtab[i].specSource))
	    fill1 (makefile, " %s", symtab[i].compName);
    fill1 (makefile, " %s", symtab[mainRes].compName);
    if (otherBin)
	for (i = 0; otherSrc[i]; i++)
	    if (otherBin[i])
		fill1 (makefile, " %s", otherBin[i]);
	    else
		fill1 (makefile, " %s", otherSrc[i]);
    fill0 (makefile, "\n\n");
    fillPos = 0;


    /* make compile */
    fill0 (makefile, "compile:");
    for (i = 0; i < numResources; i++)
	if (symtab[i].bodySource)
	    /* only components with bodies will have .o files */
	    fill1 (makefile, " $I/%s.o", symtab[i].compName);
    if (otherBin)
	for (i = 0; otherSrc[i]; i++)
	    if (otherBin[i])
		fill1 (makefile, " %s", otherBin[i]);
    fill0 (makefile, "\n\n");
    fillPos = 0;

    /* make run */
    fill0 (makefile, "run: link\n");
    fillPos = TAB_WIDTH;
    fill2 (makefile, "\t%s %s\n\n", executable, runtimeArgs);
    fillPos = 0;


    /* write out dependencies for .o and .spec files created for each mpd
     * component.
     */
    for (i = 0; i < numResources; i++) {
	if (symtab[i].bodySource) {
	    /* write out dependencies for resource body only if it exists */
	    fill1 (makefile, "$I/%s.o:", symtab[i].compName);
	    import = symtab[i].bodyImports;
	    while (import != NULL) {
		/* depend on spec unless found via MPD_PATH */
		if (symtab[import->resNum].specSource)
		    fill1 (makefile, " $I/%s.spec",
			symtab[import->resNum].compName);
		/* if a global is used and has a body, also depend on that */
		if (symtab[import->resNum].global &&
		    symtab[import->resNum].bodySource && import->resNum != i)
		    fill1(makefile," $I/%s.o",symtab[import->resNum].compName);
		import = import->next;
	    }
	    fill1 (makefile, " %s\n", symtab[i].bodySource);
	    fillPos = TAB_WIDTH;
	    fill1 (makefile, "\t$(MPD) $(MPDFLAGS) -b %s\n\n",
		symtab[i].bodySource);
	    fillPos = 0;
	}
	if (symtab[i].specSource) {
	    /* write out dependencies for resource spec only if local */
	    fill2 (makefile, "$I/%s.spec: %s\n", symtab[i].compName,
		symtab[i].specSource);
	    fillPos = TAB_WIDTH;
	    fill1 (makefile, "\t$(MPD) $(MPDFLAGS) -s %s\n\n",
		symtab[i].specSource);
	    fillPos = 0;
	}
    }

    /* make clean */
    fill0 (makefile, "clean:\n");
    fillPos = TAB_WIDTH;
    fill0 (makefile, "\trm -rf $I ");
    if (otherBin)
	for (i = 0; otherSrc[i]; i++)
	    /* Remove only .o files synthesized from other files such as *.c,
	     * not .o files passed explicitly.  */
	    if (otherBin[i] != NULL && strcmp (otherBin[i], otherSrc[i]) != 0)
		fill1 (makefile, " %s", otherBin[i]);
    fill0 (makefile, "\n\n");
    fillPos = 0;


    /* make cleanx */
    fill0 (makefile, "cleanx: clean\n");
    fillPos = TAB_WIDTH;
    if (zapFiles)
	fill2 (makefile, "\trm -f core %s %s\n\n", executable, zapFiles);
    else
	fill1 (makefile, "\trm -f core %s\n\n", executable);
    fillPos = 0;

    /* make ls */
    fill0 (makefile, "ls:\n");
    fillPos = TAB_WIDTH;
    fill0 (makefile, "\t@echo");
    while (*sourceFiles)
	fill1 (makefile, " %s", *sourceFiles++);
    /* list all the *.[^o] files in the otherFiles */
    if (otherSrc)
	for (i = 0; otherSrc[i]; i++)
	    if (otherBin[i] != NULL && strcmp (otherBin[i], otherSrc[i]) != 0)
		fill1 (makefile, " %s", otherSrc[i]);
    fill0 (makefile, "\n\n");
    fillPos = 0;

    /* make make */
    fill0 (makefile, "make:\n");
    fillPos = TAB_WIDTH;
    fill0 (makefile, "\tmpdm");
    for (i=1; argv[i]; i++)
	fill1 (makefile, " %s", argv[i]);
    fill0 (makefile, "\n\n");
}
