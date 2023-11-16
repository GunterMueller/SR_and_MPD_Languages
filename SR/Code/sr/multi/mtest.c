/*  mtest.c -- test the MultiSR porting primitives.
 *
 *  Test:
 *	1) SHARED_FILE_OBJS works if claimed (WARNING: untested)
 *	2) beat on locks some
 *	3) shared memory allocation
 *	4) thread ID
 *
 *  usage: progname nthreads
 *
 *  On a Sequent, be certain to compile with -Y, or it won't work.
 *  On an SGI Iris, be certain to compile with -lmpc, or it won't work.
 *  On Solaris 2.x, be sure to compile with -lthreads.
 */



#include "../srmulti.h"
#include "../config.h"		/* for STACK_SIZE */
#include "mtest.h"
#include <stdio.h>
#include <errno.h>

#ifndef MULTI_SR
	ERROR -- cannot test something that does not exist
#else


char *mktemp ();

void sr_missing_children ();
static void child (), use_stack ();
int sr_num_job_servers;

#include "../srmulti.c"

#define FILES_TO_TEST 10	/* test this many shared files */



multi_mutex_t test_lock;
int count1 = 0, count2 = 0, count3 = 0, count4 = 0, sum1 = 0, sum2 = 0;

char *cptr, *filename;
FILE *fp;



/*  We will allocate more memory and locks than a reasonable SR program
 *  with 10 or so CPUs would have to, just to insure that the limits
 *  or other system constraints are high enough.
 */
int alloc_sizes[] = {25600, 8192, 60, 40, 0};   /* 0 terminated */

#define NUM_ALLOCS 40		/* of each alloc_sizes[i] */
#define NUM_LOCKS 500		/* number of locks to allocate */

multi_mutex_t test_locks[NUM_LOCKS];



/*  main program  */

main (argc, argv)
int argc;
char *argv[];
{
    setbuf (stdout, NULL);	/* unbuffered, in case of crash */
    alarm (100);	/* prevent infinite loop; should run in <10 seconds */

    if (argc != 2) {
	fprintf (stderr, "usage: %s threads\n", argv[0]);
	exit (1);
    }

    sr_num_job_servers = atoi (argv[1]);
    if (sr_num_job_servers < 3)
	sr_num_job_servers = 3;

    printf ("main here\n");
    sr_init_multiSR ();
    multi_alloc_lock (test_lock);
    multi_reset_lock (test_lock);

    printf ("main creating children\n");
    sr_create_jobservers (child, sr_num_job_servers);

    /* the children are never supposed to return */
    printf ("OOPS: main returned!\n");
    exit (1);
    /*NOTREACHED*/
}


static void
child (arg)
void *arg;
{
    int id, i, j;
    static char template[] = "/tmp/mtestXXXXXX";
    FILE *dev_null;

    sr_jobserver_first (arg);

    id = MY_JS_ID;

    if ((dev_null = fopen ("/dev/null", "w")) == NULL)
	sr_abort ("Could not open /dev/null for writing");

    multi_lock (test_lock);
    printf ("child %d here\n", id);
    count1++;
    /* thread 0 will allocate data, later thread 1 will write,
     * and thread 2 will read.
     */

    use_stack ();
    printf ("child %d stack test finished\n", MY_JS_ID);

    if (id == 0) {

	int total = 0;

	/* allocate, initialize, grab, and free NUM_LOCKS locks */
	for (i = 0; i < NUM_LOCKS; i++)
	    multi_alloc_lock (test_locks[i]);
	for (i = 0; i < NUM_LOCKS; i++)
	    multi_reset_lock (test_locks[i]);
	for (i = 0; i < NUM_LOCKS; i++)
	    multi_lock (test_locks[i]);
	for (i = 0; i < NUM_LOCKS; i++)
	    multi_free_lock (test_locks[i]);

	/* Make sure we can allocate enough memory and locks. */
	for (i = 0; alloc_sizes[i]; i++)
	    for (j = 0; j < NUM_ALLOCS; j++) {
		if ((cptr = MALLOC (alloc_sizes[i])) == (char *) NULL) {
		    fprintf (stderr,
			"MALLOC(%d) failed, iteration %d, %d allocd\n",
			alloc_sizes[i], i, total);
		    EXIT (1);
		}
		total += alloc_sizes[i];
	    }

	cptr = MALLOC (MALLOC_SIZE);
	if (cptr == NULL) {
	    fprintf (stderr, "OOPS: MALLOC(%d) returned NULL\n", MALLOC_SIZE);
	    exit (1);
	} else
	    printf ("cptr MALLOCed OK\n");
    }
    multi_unlock (test_lock);

    /* wait until all are done printing.  We will put characters
     * out to /dev/null so the threads package will hopefully
     * give other threads a chance to run.  */

    while (count1 < sr_num_job_servers)
	putc ('X', dev_null);

    /* beat on a lock hard, see if it really gives mutex */
    for (i = 0; i < NUM_LOOPS; i++) {
	multi_lock (test_lock);
	sum1 += SUM_INC;
	multi_unlock (test_lock);
    }

    /* let others know I'm done incrementing; if I'm the
     * last one here check results */
    multi_lock (test_lock);

    printf ("child %d done printing\n", id);

    /* thread 1 sets cptr here */
    if (id == 1) {
	*cptr = MALLOC_BYTE;
	printf ("MALLOCed byte set to %02x\n", MALLOC_BYTE);
    }

    if (count2 == (sr_num_job_servers - 1)) {
	/* I'm the last one here */
	int target = sr_num_job_servers * NUM_LOOPS * SUM_INC;
	if (sum1 != target) {
	    fflush (stdout);
	    fprintf (stderr, "OOPS: sum1 %d, not %d\n", sum1, target);
	    EXIT (1);
	} else
	    printf ("sum1 OK after incrementing\n");
    }
    count2++;
    multi_unlock (test_lock);

    /* spin until all are done incrementing */

    while (count2 < sr_num_job_servers)
	putc ('X', dev_null);

    /* Beat hard on the lock, incrementing by one. */
    for (i = 0; i < NUM_LOOPS; i++) {
	multi_lock (test_lock);
	sum2++;
	multi_unlock (test_lock);
    }

    multi_lock (test_lock);
    /* thread 2 reads byte here */
    if (id == 2) {
	unsigned char c;
	c = (unsigned char) *cptr;
	if (c != MALLOC_BYTE) {
	    fflush (stdout);
	    fprintf (stderr, "OOPS: shared byte read %02x, not %02x\n",
		c, MALLOC_BYTE);
	    EXIT (1);
	}
	printf ("MALLOCed byte read as %02x\n", c);

#ifdef SHARED_FILE_OBJS
	/* open a shared file for later use by another thread. */
	filename = mktemp (template);
	fp = fopen (filename, "w");
	if (fp == (FILE *) NULL) {
	    fflush (stdout);
	    perror ("file opening problem");
	    EXIT (1);
	}
	printf ("File opened OK\n");
#endif /* SHARED_FILE_OBJS */

    }

    if (count3 == (sr_num_job_servers - 1)) {
	/* I'm the last one here */
	int target = sr_num_job_servers * NUM_LOOPS;
	if (sum2 != target) {
	    fflush (stdout);
	    fprintf (stderr, "OOPS: sum2 %d, not %d\n", sum2, target);
	    fprintf (stderr, "multi_lock does not protect increment\n");
	    EXIT (1);
	} else
	    printf ("sum2 OK after incrementing\n");
    }

    count3++;
    multi_unlock (test_lock);

    /* spin until thread 2 has opened files and read byte */
    while (count3 < sr_num_job_servers)
	putc ('X', dev_null);

    /* have thread 1 write to the file objects */
    multi_lock (test_lock);

#ifdef SHARED_FILE_OBJS
    if (id == 1) {
	int rtn;
	rtn = fprintf (fp, "Howdy, howdy\n");
	if (rtn == EOF) {
	    perror ("file write problem");
	    EXIT (1);
	}
	printf ("File written OK\n");

	if (fclose (fp) == EOF) {
	    perror ("file close problem");
	    EXIT (1);
	}
	printf ("File closed OK\n");

	/* this isn't necessarily a threads test, but it must be removed
	 * so junk doesn't accumulate in the temp directory. */
	if (unlink (filename) == -1) {
	    perror ("file unlink problem");
	    fprintf (stderr, "Oops: child %d couldn't unlink file %s\n",
		id, filename);
	    EXIT (1);
	}
	printf ("File unlinked OK\n");
    }
#endif /* SHARED_FILE_OBJS */

    count4++;
    multi_unlock (test_lock);

    /* spin until thread 1 has written files */
    while (count4 < sr_num_job_servers)
	putc ('X', dev_null);

    EXIT (0);
    /*NOTREACHED*/
}


void
sr_missing_children (requested, granted)
int requested, granted;
{
    multi_lock (test_lock);
    fflush (stdout);
    fprintf (stderr, "OOPS: we asked for %d threads, but we got %d\n",
	requested, granted);
    exit (1);
    /*NOTREACHED*/
}


/*  use_stack consumes a large amount of stack space by declaring a
 *  huge automatic array, and it verifies that it can touch it all.  */
static void
use_stack ()
{
    unsigned char stack_bytes[STACK_SIZE/2];
    int i;

    for (i = 0; i < STACK_SIZE / 2; i++)
	stack_bytes[i] = i & 0xff;
}

#endif	/* MULTI_SR */
