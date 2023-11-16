#include <stdio.h>

printreal(r1, r2, r3)
double r1, r2, r3;
{
	printf("%2.5f %2.5f %2.5f\n", r1, r2, r3);
	fflush(stdout);
	return 'A';
}

int_times_real(i1, r1, r2, i2)
double r1, r2; int i1, i2;
{
	printf("%2.5f %2.5f %2.5f\n", r1*i1, r2*i2, r1*(r2/i2)*i1);
	fflush(stdout);
	return 0;
}

double prod(r1, i1, i2, r2)
double r1, r2; int i1, i2;
{
	return r1*(r2/i2)*i1;
}
