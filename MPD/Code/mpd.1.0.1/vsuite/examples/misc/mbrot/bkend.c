/*  backend(path) - fork and exec path as back end filter, with no arguments  */

backend(path)
char *path;
{
    char *argv[2];
    argv[0] = path;
    argv[1] = 0;
    filter(1,path,argv);
    return 0;
}



/*  filter(io,name,argv) - filter input or output through another program
 *
 *  If io is 0, program <name> is executed with argument vector <argv> as an
 *  input filter.  The filter inherits the standard input file of the calling
 *  program;  the filter's output becomes the caller's standard input via a
 *  pipe.
 *
 *  If io is 1, an output filter is created.  The caller's standard output is
 *  diverted into the filter's standard input, with the filter inheriting the
 *  old standard output.
 *
 *  <name> and <argv> are as for execv(3).  <name> is the file to be executed;
 *  <argv> is the argument vector.  Note that argv[0] (usually the program name)
 *  must be supplied, and that a NULL must terminate the list.
 *
 *  Filter returns the pid of its child (filter) process, or 0 if errors
 *  prevented its creation.  Errors occurring in the child, such as an inability
 *  to execute the named file, cause the process to exit with a status of 1
 *  after writing a diagnostic on standard error output.
 *
 *  Filter may be called repeatedly to build up an arbitrarily long pipeline
 *  from the outside in.
 *
 *  4-Mar-86  gmt
 */

#include <stdio.h>

filter(io,name,argv)
int io;
char *name, *argv[];
{
    int pid, p[2];

    io = io & 1;
    if ((pipe(p) < 0) || ((pid = fork()) < 0))
	return (0);
    if (!pid)  {		/* if we are the child, exec the filter */
	dup2(p[!io],!io);
	close(p[0]);
	close(p[1]);
	execv(name,argv);
	fprintf(stderr,"can't exec %s\n",name);
	exit(1);
    }
    dup2(p[io],io);		/* if parent, just connect to the pipe */
    close(p[0]);
    close(p[1]);
    return (pid);
}
