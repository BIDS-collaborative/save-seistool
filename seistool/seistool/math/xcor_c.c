#ifndef lint
static char id[] = "$Id: xcor_c.c,v 1.2 2013/02/28 21:24:54 lombard Exp $";
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "math_fort.h"
#include "mathlib.h"

/*
 * returns the xcorrelation of b with respect to a
 * or the spectral ratio of a/b.
 * buffers the input and output cleanly
 *
 */
void CalcXcorr(float srate, float *data1, float *data2, float **y, 
	       int *n2_p, int *npts, int soph )
{
    int n2, i, lsoph,err;
    float dt;
    float *c1, *c2, *y1;
    float *a1, *b1;

    n2= *n2_p;
 
    dt= (float)1/srate;
    lsoph=soph;
    *y= y1= (float *)malloc(sizeof(float) * (n2+10)*2);
    c1= (float *)malloc(sizeof(float) * (n2+10)*2);
    c2= (float *)malloc(sizeof(float) * (n2+10)*2);
    for(i=0;i<(*npts)*2;i++) {
	c1[i]=data1[i];
	c2[i]=data2[i];
    }
    xcor_(npts,&dt,c1, c2, y1, &err);
    free(c1);
    free(c2);
    return;
}
