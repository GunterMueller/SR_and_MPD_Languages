/* fill.c */

#include <stdio.h>
#include "../config.h"
#include "srm.h"

int fillPos;		/* current position on line */
extern char widelines;	/* unlimited line-length flag */

/*
 * fprintf(fd,format) without exceeding MAX_COLUMNS chars on one line.
 */
fill0 (fd, format)
FILE *fd;
char *format;
{
    if (format[0] != '\n')	/* don't start new line for a newline */
	fillroom (fd, strlen (format));
    fprintf (fd, format);
}


/*
 * fprintf(fd, format, str1) without exceeding MAX_COLUMNS chars on one line.
 */
fill1 (fd, format, str1)
FILE *fd;
char *format, *str1;
{
    fillroom (fd, strlen (format) - 2 + strlen (str1));
    fprintf (fd, format, str1);
}


/*
 * fprintf(fd,format,str1,str2) without exceeding MAX_COLUMNS chars on one line.
 */
fill2 (fd, format, str1, str2)
FILE *fd;
char *format, *str1, *str2;
{
    fillroom (fd, strlen (format) - 4 + strlen (str1) + strlen (str2));
    fprintf (fd, format, str1, str2);
}


/*
 * make room for the addition of n characters, updating position counter.
 * starting a new line if MAX_COLUMNS would be exceeded.
 */
fillroom (fd, n)
FILE *fd;
int n;
{
    if (!widelines && fillPos + n >= MAX_COLUMNS && fillPos) {
	fprintf (fd, "\\\n");
	fillPos = n;
    } else
	fillPos += n;
}
