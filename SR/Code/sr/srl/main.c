/*  main.c -- main program  */


#include <ctype.h>
#include <stdio.h>
#include "globals.h"
#include "funcs.h"
#include "../config.h"
#include "../paths.h"


char version[] = VERSION;	/* SR version number */

int exper = 0;			/* experimental RTS flag */
int dbx = 0;			/* link with debugging lib for dbx */
int errors = 0;			/* number of errors */
int do_time_check = 1;		/* make sure compiled objects are current */

char *Interfaces = INTER_DIR;	/* name of Interfaces directory */
char *exe_file;			/* name of executable file */
char *lib_file = 0;		/* name of object library file */
    
resp res_list = NULL;		/* list of all resource patterns */
				/* (main resource heads the list) */

objp obj_list = NULL;		/* list of other object files */
objp obj_tail = NULL;		/* tail of the list */



/*  main program  */

main (argc, argv)
int argc;
char *argv[];
{
    int i, c;
    resp res;
    objp obj;
    buffer mesg;
    char path[MAX_PATH];
    FILE *f;
    extern int optind;
    extern char *optarg;

    /* parse command line options. */
    /* the "+" getopt arg works around gratuitous GNU getopt incompatibility */
    while ((c = getopt (argc, argv, "+eglvwI:o:r:AC:L:N:O:P:Q:R:V:S:")) != EOF)
	switch (c)  {

	case 'I':
	    Interfaces = alloc (strlen (optarg) + strlen (INTER_DIR) + 2);
	    sprintf (Interfaces, "%s/%s", optarg, INTER_DIR);
	    break;

	case 'o':
	    exe_file = optarg;
	    break;

	case 'r':
	    lib_file = optarg;
	    break;
	    
	case 'e':  exper++;		break;
	case 'g':  dbx++;		break;
	case 'v':  trcexec++;		break;
	case 'w':  do_time_check = 0;	break;
	case 'A':  async_flag = 1;	break;

	case 'l':  showlimits ();	exit (0);

	case 'C':
	case 'L':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'V':
	case 'S':
	    setlimit (c, optarg);
	    break;
	
	default:  mexit (
"usage: srl [-eglvwA] [-I dir] [-o file] [-CLNOPQRVS size] component...");
    }

    if (!exe_file)
	exe_file = "a.out";

    xprefix = argv[0];	/* set exit message prefix */
    if (optind >= argc)
	mexit ("no resources specified");

    setpath (Interfaces, exper);

    /* select the ones we want */
    while (optind < argc) {
	optarg = argv[optind++];
	if (strtail (optarg, ".o") || strtail (optarg, ".a")
	|| strncmp (optarg, "-l", 2) == 0)  {
	    /* file name for loading */
	    obj = (objp) alloc (sizeof (struct obj_st));
	    obj->next = 0;
	    obj->object = optarg;
	    if (optarg[0] != '-') {	/* if file.o or file.a form */
		f = pathopen (optarg, "", path);
		obj->object = salloc (path);
		if (f == NULL) {
		    sprintf (mesg, "can't find %s", optarg);
		    srl_error (mesg);
		} else
		    fclose (f);
	    }
	    if (obj_tail)
		obj_tail = obj_tail->next = obj;
	    else 
		obj_list = obj_tail = obj;
	} else if (strchr (optarg, '.')) {
	    sprintf (mesg, "invalid resource name: %s", optarg);
	    mexit (mesg);
	} else {
	    resource (optarg);		/* add resource name to list */
	}
    }

    /* make sure everything is up to date and correctly specified */
    if (!res_list)
	mexit ("no resources named");
    if (res_list->params != 0) {
	sprintf (mesg, "main resource (%s) has parameters", res_list->name);
	srl_error (mesg);
    }
    if (res_list->rtype == 'g') {
	sprintf (mesg, "main resource (%s) is a global", res_list->name);
	srl_error (mesg);
    }

    /* number the resources (note that the *last* one begins the list) */
    for (i = 0, res = res_list; res; res = res->next)
	res->patnum = i++;

    /* get out early before doing costly bogus link */
    if (errors) 
	mexit ("linking suppressed");

    /* generate executable(s) */
    gen_config ();	/* gen a.out.o configuration file */
    gen_exe ();		/* Does Not Return! */
    /*NOTREACHED*/
}



/*  Print a warning message.  */

void
srl_warn (s)
char *s;
{
    fprintf (stderr, "srl warning:  %s\n", s);
}



/*  Print a non-fatal error message.  */

void
srl_error (s)
char *s;
{
    fprintf (stderr, "srl error:  %s\n", s);
    errors++;
}
