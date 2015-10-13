#ifndef lint
static char id[] = "$Id: amaxper_c.c,v 1.2 2013/02/28 21:24:55 lombard Exp $";
#endif

#include <stdio.h>
#include "math_fort.h"
#include "mathlib.h"

/*
 * instr_type:	0 - deconvolve
 *		1 - Benioff 100kg (Mb)
 *		2 - 17s-23s 10 pole butterworth filter (Ms)
 *		3 - Wood Anderson (ML)
 *
 *  pRaw_max: raw maximum
 *  pGround_max: ground motion maximum
 *  pPeriod_max: period of maximum
 */
void ComputeMaxAmpPeriod(int nsamp, float srate, int instr_type, float *data,
			 float *pRaw_max, float *pGround_max, 
			 float *pPeriod_max, int *pIdx )
{
    int npts, ind;
    float dt;
    float gd_max;

    npts= nsamp;
    dt= (float)1/srate;
    ind= instr_type;
    amaxper_(&npts,&dt,&ind,data,pRaw_max,&gd_max,pPeriod_max,pIdx);
    *pGround_max=gd_max;
    return;
}
