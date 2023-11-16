/*  gen.c -- generate initialization file and link script  */

#include "globals.h"
#include "funcs.h"
#include "../config.h"
#include "../paths.h"
#include "../srmulti.h"
#include <ctype.h>

static char **addwords ();

static char *exe_tail;



void
gen_config () {
    FILE *fp;
    resp res;
    int res_cnt;		/* number of resources patterns */
    buffer temp;
    char *argv[MAX_RES_DEF+10];
    char **argp;
    char *base;

    if (exe_tail = strrchr (exe_file, '/'))	/* strip path */
	exe_tail++;
    else
	exe_tail = exe_file;

    sprintf (temp, "%s/_%s.c", Interfaces, exe_tail);
    fp = mustopen (temp, "w");

    if (exper)
	base = SRSRC;
    else
	base = SRLIB;

    fprintf (fp, "/* srl info for %s */\n\n", exe_tail);

    fprintf (fp, "#include \"%s/srmulti.h\"\n", base);
    fprintf (fp, "#include \"%s/sr.h\"\n\n", base);

    fprintf (fp, "static char SR_version[] = \n\t\"%s\";\n\n", version);

    writelimits (fp);

    fprintf (fp, "char sr_exec_path[] = \"");
    if (exper)
	fprintf (fp, "%s/rts/%s", SRSRC, RUNTIME_EXEC);
    else
	fprintf (fp, "%s/%s", SRLIB, RUNTIME_EXEC);
    fprintf (fp, "\";\n\n");

    for (res = res_list; res; res = res->next)
	fprintf (fp, "int N_%s = %d;  extern void R_%s(), F_%s();\n",
	    res->name, res->patnum, res->name, res->name);

    res_cnt = 0;
    fprintf (fp, "\nRpat sr_rpatt[] = {\n");
    for (res = res_list; res; res = res->next) {
	res_cnt += 1;
	fprintf (fp, "    { \"%s\", R_%s, F_%s },\n",
	    res->name, res->name, res->name);
    }
    fprintf (fp, "};\n");

    fprintf (fp, "int sr_num_rpats = %d;\n", res_cnt);

    fclose (fp);

    /* compile (in the Interfaces directory) the configuration file */
    sprintf (temp, "_%s.c", exe_tail);
    argp = argv;
    *argp++ = CCPATH;
    if (dbx)
	*argp++ = "-g";

#ifdef MULTI_CC_OPT		/* if need special cc option for MultiSR */
    *argp++ = MULTI_CC_OPT;
#endif

    *argp++ = "-c";
    *argp++ = temp;
    *argp++ = 0;
    if (spawn (argv, Interfaces) != 0)
	mexit ("can't compile VM init file");
}



/*
 *  Link the configuration with resources and rts.  DOES NOT RETURN.
 */
void
gen_exe () {
    resp res;
    objp obj;
    buffer rtsobj, figfile;
    char *argv[MAX_RES_DEF+10];
    char **argp;
    static char libm[] = LIBM;
    static char libr[] = LIBR;
    static char libc[] = LIBC;

    if (lib_file)
	strcpy (rtsobj, lib_file);
    else if (exper)
	sprintf (rtsobj, "%s/rts/%s", SRSRC, RUNTIME_OBJ);
    else
	sprintf (rtsobj, "%s/%s", SRLIB, RUNTIME_OBJ);
    
    sprintf (figfile, "%s/_%s.o", Interfaces, exe_tail);
    argp = argv;
    *argp++ = CCPATH;
    if (dbx)
	*argp++ = "-g";

    if (!dbx)
	*argp++ = "-s";		/* strip symbols from output if not debugging */

#ifdef MULTI_CC_OPT		/* if need special cc option for MultiSR */
    *argp++ = MULTI_CC_OPT;
#endif

    *argp++ = "-o";
    *argp++ = exe_file;
    *argp++ = figfile;
    for (res = res_list; res; res = res->next)
	*argp++ = res->objpath;
    *argp++ = rtsobj;
    for (obj = obj_list; obj; obj = obj->next)
	*argp++ = obj->object;
    argp = addwords (argp, libm);
    argp = addwords (argp, libr);
    argp = addwords (argp, libc);
    *argp++ = 0;
    doexec (argv);
    exit (0);
}



/*  addwords (argp, s) -- add words of s to an arg list, destroying s  */

static char **
addwords (argp, s)
char **argp;
char *s;
{
    for (;;) {
	while (isspace (*s))		/* skip leading whitespace */
	    s++;
	if (*s == '\0')			/* quit at end of string */
	    break;
	*argp++ = s;			/* add pointer to word */
	while (*s != '\0' && !isspace (*s))
	    s++;			/* find end of word */
	if (*s != '\0')
	    *s++ = '\0';		/* terminate word */
    }
    return argp;
}
