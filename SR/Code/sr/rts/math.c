/*  math.c -- runtime support of arithmetic and math builtins  */

#include <stdarg.h>
#include "rts.h"



/*  sr_rtor returns the exponent x**y, where x and y are Real.
 *  Error if (x=0 and y<=0) or (x<0 and y not int).
 */

Real
sr_rtor (locn, x, y)
char *locn;
Real x, y;
{
    if ((x == 0) && (y <= 0))
	sr_loc_abort (locn, "0.0**y with y<=0");

    else if ((x < 0) && (ceil (y) != y))
	sr_loc_abort (locn, "x**y with x<0 and y not an integer");

    return (Real) pow (x, y);
}



/*  returns the exponent i**j, where i and j are integers.  j must
 *  be positive.  Error if (x=0 and y<=0).
 */

Real
sr_rtoi (locn, x, j)
char *locn;
Real x;
int j;
{
    Real term = x;
    Real result = 1;
    int bits;

    if ((x == 0) && (j <= 0))
	sr_loc_abort (locn, "0.0**i with i<=0");

    for (bits = (j < 0 ? -j : j); bits; bits >>= 1, term *= term)
	if (bits & 1)
	    result *= term;

    return j >= 0 ? result : 1.0 / result;
}


/*  returns the exponent i**j, where i and j are integers.
 *  Error if (x=0 and y<=0) or if j is negative.
 */

int
sr_itoi (locn, i, j)
char *locn;
int i, j;
{
    int term = i;
    int result = 1;

    if (j < 0)
	sr_loc_abort (locn, "i**j with j<0");
    else if ((i == 0) && (j <= 0))
	sr_loc_abort (locn, "0**i with i<=0");

    for (; j; j>>=1, term *= term)
	if (j & 1)
	    result *= term;

    return result;
}



/*
 *  return the nearest integer to x (as a Real), and if it is exactly
 *  between two Reals then return the even ``neighbor.''
 */
Real
sr_round (locn, x)
char *locn;
Real x;
{
    Real xx, y;

    xx = x + 0.5;
    y = floor (xx);
    /* if halfway, and y ``odd'', then choose the even one */
    if  ((xx == y) && (sr_rmod (locn, y, 2.0) == 1.0))
	y -= 1.0;

    return y;
}



/*
 *   x mod y ==  x - y * floor (x / y)
 *  (from Knuth vol1 p38).
 *  thus,
 *  a) if y>0,  0 <= x mod y  < y
 *  b) if y<0,  0 >= x mod y  > y
 *  c) if y == 0 then error
 *  d) note that x mod y is always of the same sign as y
 */

Real
sr_rmod (locn, x, y)
char *locn;
Real x, y;
{
    if (y == 0.0)
	sr_loc_abort (locn, "x mod y with y=0.0");
    return x - y * ((Real) floor (x / y));
}


/*
 * see the documentation for sr_rmod for sr_imod.
 */

int
sr_imod (locn, x, y)
char *locn;
int x, y;
{
    if (y == 0)
	sr_loc_abort (locn, "i mod j with j=0");

    if (x >= 0) {
	if (y > 0)
	    return  x % y;
	else {
	    int res;
	    res = x % (-y);
	    if (res == 0)
	        return 0;
	    else
		return res + y;
	}
    } else {
	if (y > 0) {
	    int res;
	    res =  (-x) % y;
	    if (res == 0)
		return 0;
	    else
		return -res + y;
	} else
	    return - ((-x) % (-y));
    }
}



/*
 *  sr_imax (nargs, v, ...) -- return maximum of n integer values.
 */
int
sr_imax (int n, ...)
{
    va_list ap;
    int r, v;

    va_start (ap, n);
    if (n <= 0)
	sr_malf ("no args to sr_imax");

    r = va_arg (ap, int);	/* pick off the first one */
    n--;

    while (n--)  {
	v = va_arg (ap, int);
	if (v > r)
	    r = v;

    }
    va_end (ap);
    return r;
}



/*
 *  sr_imin (nargs, v, ...) -- return minimum of n integer values.
 */
int
sr_imin (int n, ...)
{
    va_list ap;
    int r, v;

    va_start (ap, n);
    if (n <= 0)
	sr_malf ("no args to sr_imin");

    r = va_arg (ap, int);	/* pick off the first one */
    n--;

    while (n--)  {
	v = va_arg (ap, int);
	if (v < r)
	    r = v;
    }
    va_end (ap);
    return r;
}


/*
 *  sr_rmax (nargs, v, ...) -- return maximum of n real values.
 */
Real
sr_rmax (int n, ...)
{
    va_list ap;
    Real r, v;

    va_start (ap, n);
    if (n <= 0)
	sr_malf ("no args to sr_rmax");

    r = va_arg (ap, Real);	/* pick off the first one */
    n--;


    while (n--)  {
	v = va_arg (ap, Real);
	if (v > r)
	    r = v;
    }
    va_end (ap);
    return r;
}



/*
 *  sr_rmin (nargs, v, ...) -- return minimum of n real values.
 */
Real
sr_rmin (int n, ...)
{
    va_list ap;
    Real r, v;

    va_start (ap, n);
    if (n <= 0)
	sr_malf ("no args to sr_rmin");

    r = va_arg (ap, Real);	/* pick off the first one */
    n--;

    while (n--)  {
	v = va_arg (ap, Real);
	if (v < r)
	    r = v;
    }
    va_end (ap);
    return r;
}



/*
 *  Random number generator based on Knuth.
 *  See Seminumerical Algorithms, 2nd edn, section 3.6.
 *  Uses the subtractive series X[n] = (X[n-55] - X[n-24]) mod M, n >= 55.
 */
#define RANDMAX 1000000000
#define RAND_L 24
#define RAND_K 55
#define RAND_D 21

static int X[RAND_K];			/* history array */
static int cur = 0;			/* Current index in array.   */
static int curL = RAND_K - RAND_L;	/* always (cur - RAND_L) mod RAND_K */
static Mutex random_mutex;		/* protection for above */



/*
 *  sr_init_random () -- initialize random number generation.
 */

void
sr_init_random ()
{
    INIT_LOCK (random_mutex, "random_mutex");
    sr_seed (0.0);
}



/*
 *  sr_seed (x) -- seed the random number generator based on the value of x.
 *
 *  If x is zero, an irreproducible value is used.
 */
void
sr_seed (x)
Real x;
{
    /* these are also protected by random mutex */
    static int num_seed_call;
    static int seed_pid;
    static int seed_time;
    int i, j, k, seed, index;
    time_t time ();

    LOCK (random_mutex, "sr_seed");

    num_seed_call++;
    if (num_seed_call == 1) {
	seed_pid = getpid ();
	seed_time = time ((time_t *) 0);
    }
    if (x == 0.0)			/* irreproducible */
	seed = (997 * seed_pid + seed_time + 23347 * num_seed_call);
    else if (x > -1 && x < 1)
	seed = 1 / x;
    else
	seed = x;
    if (seed < 0)
	seed = -seed;
    seed = seed % RANDMAX;

    /*
     * According to Knuth, "This computes a Fibonacci-like sequence;
     * multiplication of indices by 21 [the constant RAND_D] spreads the
     * values around so as to alleviate initial nonrandomness problems..."
     */
    X[0] = j = seed;
    k = 1;
    for (i = 1; i < RAND_K; i++) {
	index = (RAND_D * i) % RAND_K;
	X[index] = k;
	k = j - k;
	if (k < 0)
	    k += RANDMAX;
	j = X[index];
    }
    cur = 0;
    curL = RAND_K - RAND_L;

    UNLOCK (random_mutex, "sr_seed");

    /*
     * `warm up' the generator
     */
    for (i = 0; i < 5 * RAND_K; i += 1)
	sr_random (0.0, 1.0);
}



/*  sr_random (x, y) returns a random number r such that x <= r < y */

Real
sr_random (x, y)
double x, y;
{
    int iresult;

    LOCK (random_mutex, "sr_random");

    iresult = X[cur] - X[curL];
    if (iresult < 0)
	iresult += RANDMAX;
    X[cur] = iresult;

    cur += 1;   if (cur == RAND_K)  cur = 0;
    curL += 1;  if (curL == RAND_K) curL = 0;

    UNLOCK (random_mutex, "sr_random");

    return x + (y - x) * iresult * (1.0 / RANDMAX);
}
