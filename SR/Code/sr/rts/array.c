/*  array.c -- runtime support of arrays  */

#include "rts.h"
#include <stdarg.h>


static void slices ();



/*
 *  sr_init_array (locn, addr, elemsize, initvalue, ndim, lb1, ub1, ...)
 *	-- initialize an array.
 *
 *  If addr is 0, first allocate it.  If addr is -1, obtain from sr_new().
 *
 *  If elemsize is zero or negative, it is the (negative) maxlength of
 *  a string, for which an initializer will be created internally.
 *
 *  If ndim is negative, it is followed by two *pointers* to lb/ub arrays,
 *  instead of a list of lb and ub values.
 */
Array *
sr_init_array (
    char *locn, Array *addr, int elemsize, Ptr initvalue, int ndim, ...)
{
    va_list ap;
    int alcsize, i, n;
    Dim *d;
    Ptr p, q;
    int *lb, *ub, la[MAX_DIMENS], ua[MAX_DIMENS], size[MAX_DIMENS+1];
    String s;			/* for string initialization */

    sr_check_stk (CUR_STACK);

    /*
     *  Calculate the size of each dimension.
     */
    va_start (ap, ndim);
    if (ndim < 0) {
	ndim = -ndim;
	lb = va_arg (ap, int *);
	ub = va_arg (ap, int *);
    } else {
	lb = la;
	ub = ua;
	for (i = 0; i < ndim; i++) {
	    lb[i] = va_arg (ap, int);
	    ub[i] = va_arg (ap, int);
	    if (ub[i] - lb[i] + 1 < 0)
		sr_runerr (locn, E_ABND, lb[i], ub[i]);
	}
    }
    va_end (ap);
    DEBUG5 (D_ARRAY, "sr_init_array (%06lX, %ld, %06lX, %ld, ?, %ld, ...)",
	addr, elemsize, initvalue, ndim, ub[0]);

    /*
     *  Create a string initializer, if needed.
     */
    if (elemsize <= 0) {
	s.size = -elemsize + STRING_OVH;
	s.length = -1;
	elemsize = SRALIGN (s.size);
	initvalue = (Ptr) &s;
    }

    /*
     *  Calculate the total size, and allocate.
     */
    size[ndim] = elemsize;
    for (i = ndim - 1; i >= 0; i--) 
	size[i] = (ub[i] - lb[i] + 1) * size[i+1];
    alcsize = SRALIGN (size[0] + sizeof (Array) + sizeof (Dim) * ndim);
    if (addr == NULL)
	addr = (Array *) sr_alc (alcsize, 1);
    else if (addr == (Array *) -1)
	addr = (Array *) sr_new (locn, alcsize);

    /*
     *  Initialize array headers.
     */
    addr->offset = sizeof (Array) + ndim * sizeof (Dim);
    addr->size = addr->offset + size[0];
    addr->ndim = ndim;

    /* 
     * Initialize dimension headers (in reverse order)
     */
    for (i = ndim - 1, d = (Dim *)(addr + 1); i >= 0; i--, d++)  {
	d->lb = lb[i];
	d->ub = ub[i];
	d->stride = size[i+1];
    }

    /*
     *  Initialize data
     */
    if (initvalue != NULL && size[0] > 0) {
	p = ADATA (addr);		/* destination */
	q = p + size[0];		/* end */
	memcpy (p, initvalue, elemsize); /* init first element */
	initvalue = p;			/* now copy from front of array */
	p += elemsize;			/* bump store pointer */
	n = elemsize;			/* bytes available to copy */
	while (p + n < q) {
	    memcpy (p, initvalue, n);
	    p += n;
	    n *= 2;
	}
	memcpy (p, initvalue, q - p);
    }

    return addr;
}



/*
 *  Return the number of elements in an array.
 */
int
sr_acount (a)
Array *a;
{
    Dim *d = (Dim *) (a + 1);
    int i = a->ndim;
    int n = 1;
    while (i-- > 0) {
	n *= d->ub - d->lb + 1;
	d++;
    }
    return n;
}



/*
 *  Copy one array to another, returning the pointer to the destination array.
 *  Data is copied but the destination header is left alone.
 */
Array *
sr_acopy (locn, dest, src)
char *locn;
Array *dest, *src;
{
    Dim *ld, *rd;
    int i, ndim;

    ndim = dest->ndim;
    ld = (Dim *) (dest + 1) + ndim - 1;
    rd = (Dim *) (src + 1) + ndim - 1;
    for (i = 0; i < ndim; i++, ld--, rd--)
	if (ld->ub - ld->lb != rd->ub - rd->lb)
	    sr_runerr (locn, E_ASIZ, ld, rd);

    memcpy ((Ptr) dest + dest->offset, (Ptr) src + src->offset,
	dest->size - dest->offset);
    return dest;
}



/*
 *  Swap two arrays and return the address of the left side (value of R side).
 */
Array *
sr_aswap (locn, lside, rside)
char *locn;
Array *lside, *rside;
{
    Dim *ld, *rd;
    Ptr l, r;
    char c;
    int i, n, ndim;

    ndim = lside->ndim;
    ld = (Dim *) (lside + 1) + ndim - 1;
    rd = (Dim *) (rside + 1) + ndim - 1;
    for (i = 0; i < ndim; i++, ld--, rd--)
	if (ld->ub - ld->lb != rd->ub - rd->lb)
	    sr_runerr (locn, E_ASIZ, ld, rd);

    l = ADATA (lside);			/* addrs of l and r data */
    r = ADATA (rside);
    n = lside->size - lside->offset;	/* number of bytes to swap */

    while (--n >= 0) {			/* swap contents */
	c = *l;
	*l++ = *r;
	*r++ = c;
    }

    return lside;			/* return left side address */
}



/*
 *  Implement a clone operation by making duplicates beyond
 *  the first and incrementing the store address.
 */
Ptr
sr_clone (locn, addr, len, n)
char *locn;
Ptr addr;
int len, n;
{
    Ptr new = addr;

    if (n < 0)
	sr_runerr (locn, E_AREP, n);
    if (n == 0)
	return new;

    while (--n > 0)
	memcpy (new += len, addr, len);
    return new + len;
}



/*
 *  sr_slice (locn, a1, a2, elemsize, nbounds, lb1, ub1, ...)
 *
 *  Extract or store into an array slice, handling multidimensional slicing.
 *  If a1 is zero, allocate and return a slice of a2.
 *  If a1 is nonzero, scatter a1 into a slice of a2.
 *
 *  a1 is always the contiguous array; a2 is always the sliced array.
 *  nbounds is the number of (lb,ub) index pairs that follow.
 *  A ub value of SINGLEINDEX indicates a singly-indexed dimension of a2 only.
 *
 *  Build an array of "slcinfo" structs and then call slices() to do the
 *  actual work; it recurses to handle multiple dimensions.
 */

struct slcinfo {
    int n;		/* number of elements in this dimension */
    int d1;		/* address incr in contigouous array (0 in last dim) */
    int d2;		/* address incr in array being sliced */
};

Ptr
sr_slice (char *locn, Array *a1, Array *a2, int elemsize, int nbounds, ...)
{
    va_list ap;
    int adim, lb[MAX_DIMENS], ub[MAX_DIMENS];	/* array dims*/
    int nelem, telem, offset, scatter;
    int i, j, k, l, u, z, o;
    struct slcinfo s[MAX_DIMENS+1];

    sr_check_stk (CUR_STACK);

    /* get fixed arguments */
    va_start (ap, nbounds);
    if (elemsize == 0)
	elemsize = STRIDE (a2, 0);

    /* get index arguments */
    telem = 1;				/* total element count */
    adim = 0;				/* dimensionality of a1 */
    offset = 0;				/* offset into a2 */

    for (i = 0; i < nbounds; i++) {	/* for each dimension of a2 */
	j = a2->ndim - i - 1;
	l = va_arg (ap, int);		/* get index bounds */
	u = va_arg (ap, int);
	if (l == ASTERISK)		/* translate '*' into actual value */
	    l = LB (a2, j);
	if (u == ASTERISK)
	    u = UB (a2, j);

	o = l - LB (a2, j);		/* get a zero-based index */
	z = STRIDE (a2, j);		/* element size of this dimension */
	offset += z * o;		/* adjust offset to first element */

	if (u == SINGLEINDEX) {

	    if (o < 0 || l > UB (a2, j))
		sr_runerr (locn, E_ASUB, l, & ADIM (a2, j));

	} else {

	    /* this dimension is truly sliced */
	    nelem = u - l + 1;		/* number of slice elements */
	    telem *= nelem;		/* calc total elements */
	    if (o < 0 || nelem < 0 || u > UB (a2, j))
		sr_runerr (locn, E_ASLC, l, u, & ADIM (a2, j));

	    if (a1 != NULL) {
		k = a1->ndim - adim - 1;
		if (UB (a1, k) - LB (a1, k) + 1 != nelem)
		    sr_runerr (locn, E_ACHG, l, u, & ADIM (a1, k));
	    }
	    lb[adim] = 1;		/* save bounds for a1 */
	    ub[adim] = u - l + 1;
	    s[adim].n = nelem;
	    s[adim].d2 = z;		/* size of element in a2 */
	    adim++;			/* count the dimensions */
	}
    }
    va_end (ap);

    /* if loading into new array, need to allocate it */
    scatter = (a1 != NULL);
    if (!scatter)
	a1 = sr_init_array (locn, (Array*) 0, elemsize, (Ptr) 0, -adim, lb, ub);

    for (i = 0; i < adim; i++)		/* calculate step sizes for a1 */
	s[i].d1 = STRIDE (a1, adim - i - 1);

    if (telem > 0)			/* if no zero dimensions, copy elems */
	slices (adim, ADATA (a1), ADATA (a2) + offset, s, scatter);

    return (Ptr) a1;			/* always return contiguous array */
}



/*
 *  slices (lv, p1, p2, s, scatter) -- copy slice data
 *
 *  lv is the number of nesting levels
 *  p1, p2 are data pointers
 *  s is array of slice info (see above)
 *  scatter indicates direction of copy (nonzero for p1->p2)
 *
 *  Copy s->n slice elements, recursing as needed for deeper levels.
 */
static void
slices (lv, p1, p2, s, scatter)
int lv;
Ptr p1, p2;
struct slcinfo *s;
int scatter;
{
    int n, d1, d2;

    n = s->n;
    d1 = s->d1;
    d2 = s->d2;
    while (--n >= 0) {
	if (lv > 1)
	    slices (lv - 1, p1, p2, s + 1, scatter);
	else {
	    if (scatter) 
		memcpy (p2, p1, d1);
	    else
		memcpy (p1, p2, d1);
	}
	p1 += d1;
	p2 += d2;
    }
}



/*
 *  Copy an array of strings, elementwise if necessary, into a slice
 *  (or the whole thing) of another string array.  The maxlengths of
 *  the strings in the two arrays may differ.
 */

Array *
sr_strarr (locn, dest, lb, ub, src)
char *locn;
Array *dest;
int lb, ub;
Array *src;
{
    int ndst, nsrc, i, nelem, maxl;
    String *pdst, *psrc;

    if (lb == ASTERISK)
	lb = LB (dest, 0);
    if (ub == ASTERISK)
	ub = UB (dest, 0);
    nelem = ub - lb + 1;

    if (nelem < 0 || lb < LB (dest, 0) || ub > UB (dest, 0))
	sr_runerr (locn, E_ASLC, lb, ub, dest);
    if (UB (src, 0) - LB (src, 0) + 1 != nelem)
	sr_runerr (locn, E_ACHG, lb, ub, src);

    if (nelem == 0)
	return dest;			/* empty arrays */

    psrc = (String *) ADATA (src);	/* pointer to source element */
    pdst = (String *) ADATA (dest);	/* pointer to dest element */
    nsrc = SRALIGN (psrc->size);		/* distance between source elements */
    ndst = SRALIGN (pdst->size);		/* distance between dest elements */
    pdst = (String *) ((Ptr) pdst + (lb - LB (dest, 0)) * ndst); 
					/* pointer to first destination strg */

    if (psrc->size == pdst->size) {	/* if maxlengths are the same */

	/* block copy */
	memcpy ((Ptr) pdst, (Ptr) psrc, nelem * ndst);

    } else {

	/* elementwise assignment */
	maxl = MAXLENGTH (pdst);	/* maxlength of dest element */
	for (i = 0; i < nelem; i++) {
	    if (psrc->length > maxl)
		sr_runerr (locn, E_SELM, LB (src, 0) + i, psrc->length, maxl);
	    memcpy (DATA (pdst), DATA (psrc), (pdst->length = psrc->length));
	    pdst = (String *) ((Ptr) pdst + ndst);
	    psrc = (String *) ((Ptr) psrc + nsrc);
	}
    }
    return dest;
}



/*
 *  Allocate and initialize a string as a copy of a char array.
 */
Ptr
sr_astring (a)
Array *a;
{
    String *s;
    int n;

    n = UB (a, 0) - LB (a, 0) + 1;
    s = (String *) sr_alc (n + STRING_OVH, 1);
    s->size = n + STRING_OVH;
    s->length = n;
    memcpy (DATA (s), ADATA (a), n);
    return (Ptr) s;
}



/*
 *  Allocate and extract a string slice.
 */
Ptr
sr_sslice (locn, s, index1, index2)
char *locn;
String *s;
int index1, index2;
{
    int nchars, nbytes;
    String *r;

    if (index1 == ASTERISK)
	index1 = 1;
    if (index2 == ASTERISK)
	index2 = s->length;
    nchars = index2 - index1 + 1;
    if (nchars < 0 || index1 < 1 || index2 > s->length)
	sr_runerr (locn, E_SSLC, index1, index2, s, s);
    nbytes = nchars + STRING_OVH;
    r = (String *) sr_alc (nbytes, 1);
    r->size = nbytes;
    r->length = nchars;
    memcpy (DATA (r), DATA (s) + index1 - 1, nchars);
    return (Ptr) r;
}



/*
 *  Change a string slice.
 */
String *
sr_chgstr (locn, s, index1, index2, v)
char *locn;
String *s;
int index1, index2;
String *v;
{
    int nchars;

    if (index1 == ASTERISK)
	index1 = 1;
    if (index2 == ASTERISK)
	index2 = s->length;
    nchars = index2 - index1 + 1;
    if (nchars < 0 || index1 < 1 || index2 > s->length)
	sr_runerr (locn, E_SSLC, index1, index2, s, s);
    if (nchars != v->length)
	sr_runerr (locn, E_SCHG, v, index1, index2);
    memcpy (DATA (s) + index1 - 1, DATA (v), nchars);
    return v;
}
