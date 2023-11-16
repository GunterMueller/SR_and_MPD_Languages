#include <stdio.h>
#include "../config.h"
#include "../gen.h"
#include "../util.h"
#include "srm.h"

extern	struct symtabSt symtab[MAX_RES_DEF + 1];
extern	int    numResources;

extern	char	*mainResName;

static void unrefd ();



/* scans the symbol table looking for errors */
/* also assigns a main resource if one was not given explicitly */

checkTable ()
{
    int returnval = 1;		/* checkTable returns 0 on error; 1 otherwise */
    int i;
    int mainRes = 0;		/* position of main resource in symTab */
    int num_unref = 0;		/* number of unreferenced resources */
    char buf[MAX_PATH];
    FILE *f;

    for (i = 0; i < numResources; i++) {
	if (symtab[i].bodySource == NULL && symtab[i].specSource == NULL) {
	    if (f = pathopen (symtab[i].compName, ".spec", buf)) {
		/* imported file found in SR_PATH. No problem. */
		fclose (f);
	    } else {
		fprintf (stderr, "srm: resource %s not found\n",
		    symtab[i].compName);
		returnval = 0;
	    }
	}
	else if (symtab[i].specSource == NULL) {
	    fprintf (stderr, "srm: spec for resource %s not found.\n",
		symtab[i].compName);
	    returnval = 0;
	}
	if (!symtab[i].timesImported) {
	    /* component has not been imported by any other component */
	    num_unref++;
	    if (mainResName) {
		/* main resource has been specified on the command line */
		if (strcmp (symtab[i].compName, mainResName) != 0) {
		    unrefd (i);
		}
	    }
	    else {
		/* Main resource was not specified by user.  If there is just
		 * one unreferenced resource it will be picked to be the main
		 * resource by the code in writeMakefile.  If there are more
		 * than one, give errors for all.
		 */
		if (num_unref == 1)
		    mainRes = i;		/* no error... yet */
		else {
		    if (num_unref == 2)
			unrefd (mainRes);	/* error for earlier one */
		    unrefd (i);			/* error for this one */
		}
	    }
	}
    }
    return returnval;

}



/* give error message for unreferenced resource */

static void
unrefd (i)
int i;
{
    fprintf (stderr,
	"srm: warning: resource %s not imported by any other resource\n",
	symtab[i].compName);
}
