#ifndef lint
static char id[] = "$Id: spectrum_c.c,v 1.2 2013/02/28 21:24:54 lombard Exp $";
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "math_fort.h"
#include "mathlib.h"

/*
 * returns the spectrum in y (with nt2h-1 points)
 * for each y[i], tau= float(n2)/(float(i)*srate)
 *
 * soph: 0- nothing 1- detrend,taper(5.0%),demean
 */
void CalcSpectrum(float srate, float *data, int ix1, int ix2, 
		  float **y, float **a, float **b, int *n2_p, 
		  int *nt2h, int soph )
{
    int npts, n2, i, lsoph;
    float dt;
    float *c1, *y1;
    float *a1, *b1;

    npts= ix2 - ix1 + 1;
    *n2_p= n2= ipow_(&npts);
    dt= (float)1/srate;
    lsoph=soph;
    *nt2h = (n2 + 2) / 2;
    *y= y1= (float *)malloc(sizeof(float) * (*nt2h+10));
    *a= a1= (float *)malloc(sizeof(float) * (*nt2h+10));
    *b= b1= (float *)malloc(sizeof(float) * (*nt2h+10));
    c1= (float *)malloc(sizeof(float) * (n2+10));
    for(i=0;i< npts;i++) {
	c1[i]=data[i+ix1];
    }
    spectrum_(c1, y1, a1, b1, &npts, &dt, &lsoph);
    free(c1);
    return;
}
