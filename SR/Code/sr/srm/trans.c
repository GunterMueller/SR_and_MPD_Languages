#include <stdio.h>
#include "../config.h"
#include "../gen.h"
#include "../util.h"
#include "srm.h"

extern	struct symtabSt symtab[MAX_RES_DEF + 1];
extern	int    numResources;

/* the .o file for each resource depends on the file containing the resource
 * body, the resource's spec., specs. of components imported by
 * the resource, and the transitive closure of the specs. imported by the
 * imported components.  This function computes this transitive
 * closure.
 */
computeTransitiveClosure ()
{
    int i, j;
    char noChange;
    importList *newImport, *currImport, *newImportList;

    /* add resource's spec to the list of specs its body depends on */
    for (i = 0; i < numResources; i++) {
	newImport = (importList *) alloc (sizeof (importList));
	newImport->resNum = i;	/* add res. to its own dependency list */
	newImport->next = symtab[i].bodyImports;
	symtab[i].bodyImports = newImport;
    }

    /* compute transitive closure */
    do {
	noChange = TRUE;
	for (i = 0; i < numResources; i++) {
	    for (j = 0; j < numResources; j++)
		symtab[j].inClosure = FALSE;

	    /* add components in list of imports to the closure */
	    currImport = symtab[i].bodyImports;
	    while (currImport != NULL) {
		symtab[currImport->resNum].inClosure = TRUE;
		currImport = currImport->next;
	    }

	    /* for each component in the closure, add the components its spec.
	     * imports to the closure (if it isn't already in the closure).
	     */
	    currImport = symtab[i].bodyImports;
	    while (currImport != NULL) {
		newImportList = symtab[currImport->resNum].specImports;
		while (newImportList != NULL) {
		    if (!symtab[newImportList->resNum].inClosure) {
			newImport = (importList *) alloc (sizeof (importList));
			newImport->resNum = newImportList->resNum;
			newImport->next = symtab[i].bodyImports;
			symtab[i].bodyImports = newImport;
			symtab[newImportList->resNum].inClosure = TRUE;
			noChange = FALSE;
		    }
		    newImportList = newImportList->next;
		}
		currImport = currImport->next;
	    }
	}
    }
    while (!noChange);

}
