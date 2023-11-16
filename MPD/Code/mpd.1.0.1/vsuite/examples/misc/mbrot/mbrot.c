/*  mbrot.c -- actual Mandelbrot set calculations  */


/*  calc(x,y,d,s,n,w,h) - calculate one cell
 *
 *  	x, y	coordinates of upper left corner of cell
 *	d	delta in Mandelbrot space per one pixel
 *	s	output buffer
 *	n	number of iterations
 *	w	width of cell, in pixels
 *	h	height of cell in pixels
 */

calc(x,y,d,s,n,w,h)
double x, y, d;
char *s;
int n, w, h;
{
    int i, j;
    double xx;
    for (j = 0; j < h; j++) {
	xx = x;
	for (i = 0; i < w; i++) {
	    *s++ = ptval(xx,y,n);
	    xx += d;
	}
	y -= d;
    }
    return 0;
}



/*  ptval(x,y,n) - try up to <n> iterations and return the number needed  */

ptval(x,y,n)
float x, y;
int n;
{
    register float zx, zy, xx, yy, rx, ry;
    register int i;

    rx = x;
    ry = y;
    zx = zy = xx = yy = 0.0;
    for (i = n+1;  --i;  )  {
	zy = (zx + zx) * zy + ry;
	zx = xx - yy + rx;
	xx = zx * zx;
	yy = zy * zy;
	if ((xx + yy) > 4.0)
	    break;
    }
    return n - i;
}
