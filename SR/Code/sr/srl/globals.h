/*  globals.h -- global declaration and definitions  */


#define new(s)	((s *) alloc (sizeof (s)))


typedef char buffer [100];

typedef struct res_st *resp;
typedef struct obj_st *objp;



/*  resource pattern record  */

struct res_st {
    resp next;		/* next resource pattern */
    char *name;		/* pattern name */
    char *srcpath;	/* source file path */
    char *objpath;	/* source file path */
    char *specpath;	/* spec file path */
    int params;		/* number of resource parameters */
    int patnum;		/* RTS resource pattern number */
    char rtype;		/* resource type: 'g' or 'r' */
};



/*  object file record  */

struct obj_st {
    objp next;		/* link to next object file */
    char *object;	/* object file name */
};



extern char version[];		/* SR version number */

extern char *Interfaces;	/* name of Interfaces directory */
extern int exper;		/* link with "experimental" library? */
extern int dbx;			/* link with -g flag for dbx? */

extern int errors;		/* cumulative error count */
extern char *exe_file;		/* name of executable file */
extern char *lib_file;		/* name of runtime library file, if specified */

extern int do_time_check;	/* make sure compiled objects are current */

extern resp res_list;		/* linked list of resources we want */
extern objp obj_list;		/* linked list of other object files */



/* runtime limits and parameters */

extern int max_co_stmts;	/* limit on active "co" statements */
extern int max_classes;		/* limit on "in" operation classes */
extern int max_loops;		/* limit on loops between context switches */
extern int max_operations;	/* limit on active operations */
extern int max_processes;	/* limit on number of processes */
extern int max_rmt_reqs;	/* limit on pending remote requests */
extern int max_resources;	/* limit on active resources */
extern int max_semaphores;	/* limit on number of semaphores */

extern int stack_size;		/* size of a process stack */

extern int async_flag;		/* asynchronous output? */
