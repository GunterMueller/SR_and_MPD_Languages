/*  gen.h -- general definitions for the SR system  */



/*  define macros allowing use of function prototypes  */
/*  define macros for turning identifiers into strings  */

#if defined(__STDC__) || defined(__sgi) || defined(_AIX)

#define PARAMS(p) p
#define NOARGS void
#define PASTE(a,b) a##b
#define LITERAL(x) #x

#else

#define PARAMS(p) ()
#define NOARGS
#define IDENT(s) s
#define PASTE(a,b) IDENT(a)b
#define LITERAL(x) "x"

#endif



/*  definitions of standard library functions
 *
 *  We declare these functions ourselves because include files are
 *  nonexistent or inconsistently located.
 *
 *  Don't use protoypes here; increases conflicts with system include files.
 */

char *getenv(), *getcwd();
char *strcat(), *strncat(), *strchr(), *strrchr(), *strcpy(), *strncpy();
void exit();

/* int functions are not explicitly defined -- use implicit definitions */
/* This avoids conflicts on Solaris (at least) where strlen is a size_t fn */
/* int  strcmp(), strncmp(), strlen(); */

double fmod();	/* not in Sequent <math.h> */

#if defined(__STDC__) || defined(__sgi) || defined(_AIX) || defined(__alpha)
void *malloc(), *realloc();
void *memset(), *memcpy();
#else
char *malloc(), *realloc();
char *memset(), *memcpy();
#endif


/*  handle missing functions and symbols under certain systems */

#if defined(hpux) || defined(__DGUX__)
#define psignal(sig,s) fprintf(stderr,"%s: signal %d\n",s,sig)
#endif

#if defined(sgi) || defined(linux) || defined(_AIX)
#define vfork fork
#endif

#if defined(sequent) || defined(NeXT)
char *getwd();
#define getcwd(s,n) getwd(s)
#endif

#if defined(sysV68)
#include <limits.h>
struct timezone { int tz_minuteswest; int tz_dsttime; };
#define psignal(sig,s) fprintf(stderr,"%s: signal %d\n",s,sig)
#define vfork		fork
#define SIGCHLD		SIGCLD
#endif
