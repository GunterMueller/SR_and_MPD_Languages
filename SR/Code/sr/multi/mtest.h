/*  mtest.h -- definitions for testing MultiSR port  */



#define NUM_LOOPS	10000
#define MALLOC_SIZE	100		/* be real safe */
#define MALLOC_BYTE	0xde

#define SUM_INC	2	/* incr for testing; avoid 1 in case it's atomic */



/*  define some runtime macros and functions for use here  */

#define DEBUG(mask,fmt,a,b,c) 0

#define sr_abort(message) { \
    fprintf (stderr, "ABORT: %s\n", message); \
    EXIT (1); \
}

#define sr_malf(message) { \
    fprintf (stderr, "Malfunction: %s\n", message); \
    EXIT (1); \
}



/* define some things that individual ports of MultiSR use */
int sr_irix_exitcode;
