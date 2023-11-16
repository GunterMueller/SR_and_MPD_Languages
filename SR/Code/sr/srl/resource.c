/*  resource.c -- resource file handling  */

#include <sys/types.h>
#include <sys/stat.h>
#include "globals.h"
#include "funcs.h"
#include "../config.h"



/*  add resource name to list  */

void
resource (name)
char *name;
{
    resp res;
    buffer mesg, line;
    char path[MAX_PATH];
    FILE *f;
    int n;
    time_t srctime, objtime, spectime;

    for (res = res_list; res != NULL; res = res->next) {
	if (strcmp (name, res->name) == 0) {
	    sprintf (mesg, "resource duplicated (%s)", name);
	    srl_error (mesg);
	}
    }

    /* make an entry for it and link it in */
    res = new (struct res_st);
    res->name = salloc (name);
    res->params = 0;
    res->srcpath = NULL;
    res->specpath = NULL;

    /* find spec file */
    f = pathopen (name, SPEC_SUF, path);
    if (f == NULL) {
	sprintf (mesg, "can't find spec file for %s", name);
	srl_error (mesg);
	return;
    }
    res->specpath = salloc (path);		/* save path where spec found */
    strcpy (path + strlen (path) - strlen (SPEC_SUF), ".o");
    res->objpath = salloc (path);		/* .o must be in same place */

    /* read spec info */
    if (fgets (line, sizeof (line), f) != NULL
    &&  sscanf (line, "#%2d %s", &n, path) == 2) {
	res->params = n;
	res->srcpath = salloc (path);
	fgets (line, sizeof (line), f);		/* second line */
	fgets (line, sizeof (line), f);		/* third line */
	res->rtype = line[0];			/* save 'g' or 'r' */
    } else {
	sprintf (mesg, "incomplete spec file for %s", name);
	srl_error (mesg);
    }
    fclose (f);

    /* spec was good.  link resource into list. */
    res->next = res_list;
    res_list = res;

    /* now check file times */
    srctime = modtime (res->srcpath);
    objtime = modtime (res->objpath);
    spectime = modtime (res->specpath);

    if (objtime == 0) {
	sprintf (mesg, "can't status object file %s", res->objpath);
	srl_error (mesg);
	return;
    }

    if (srctime == 0 || spectime == 0)		/* ok if can't find source */
	return;

    if (srctime > objtime) {
	sprintf (mesg,
	    "source file for resource %s is newer than its object file",
	    res->name);
	srl_warn (mesg);
    } else if (srctime > spectime) {
	sprintf (mesg,
	    "source file for resource %s is newer than its spec file",
	    res->name);
	srl_warn (mesg);
    }
}
