#include <stdio.h>
#include "../config.h"
#include "srm.h"

extern	struct	symtabSt symtab[MAX_RES_DEF + 1];
extern	int	numResources;

printStats ()
{
    int		i;
    importList	*imports;

    for (i = 0; i < numResources; i++) {

	if (symtab[i].global)
	    printf ("global");
	else
	    printf ("resource");
	printf (" %s is imported by %d other(s)\n",
	    symtab[i].compName, symtab[i].timesImported);

	if (symtab[i].specSource) {
	    printf ("  its spec (%s) imports", symtab[i].specSource);
	    if (symtab[i].specImports) {
		imports = symtab[i].specImports;
		while (imports != NULL) {
		    printf (" %s", symtab[imports->resNum].compName);
		    imports = imports->next;
		}
	    } else {
		printf (" nothing");
	    }
	} else {
	    printf ("  it was found via SR_PATH");
	}
	printf ("\n");

	if (symtab[i].bodySource) {
	    printf ("  its body (%s) imports", symtab[i].bodySource);
	    if (symtab[i].bodyImports) {
		imports = symtab[i].bodyImports;
		while (imports != NULL) {
		    printf (" %s", symtab[imports->resNum].compName);
		    imports = imports->next;
		}
	    } else
		printf (" nothing");
	    printf ("\n");
	}
	else
	    printf ("  it has no body\n");
    }
    /* some more info. is put out by writeMakefile */
}
