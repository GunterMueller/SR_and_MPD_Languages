/*
 *  mexpect.c -- synthesize expected test output for comparison.
 *
 *  usage:  progname nthreads
 *
 *  generate the output expected from multiSR_test.  The output
 *  from this program must be sorted and compared to the output
 *  of multiSR_test.
 *
 *  For simplicity, we print the output here in the order it
 *  occurs in mtest.c.
 */

#include "../srmulti.h"
#include "mtest.h"

main (argc, argv)
int argc;
char *argv[];
{
    int i, n;

    if (argc != 2) {
	fprintf (stderr, "usage: %s nthreads\n", argv[0]);
	exit (1);
    }
    n = atoi (argv[1]);
    if (n < 3)
	n = 3;
    printf ("main here\n");
    printf ("main creating children\n");
    for (i = 0; i < n; i++) {
	printf ("child %d here\n", i);
	printf ("child %d stack test finished\n", i);
	printf ("child %d done printing\n", i);
    }
    printf ("cptr MALLOCed OK\n");
    printf ("MALLOCed byte set to %2x\n", MALLOC_BYTE);
    printf ("sum1 OK after incrementing\n");
    printf ("MALLOCed byte read as %02x\n", MALLOC_BYTE);
#ifdef SHARED_FILE_OBJS
    printf ("File opened OK\n");
#endif
    printf ("sum2 OK after incrementing\n");
#ifdef SHARED_FILE_OBJS
    printf ("File written OK\n");
    printf ("File closed OK\n");
    printf ("File unlinked OK\n");
#endif
    EXIT (0);
}
