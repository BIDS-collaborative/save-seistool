#ifndef lint
static char id[] = "$Id: coinst_c.c,v 1.2 2013/02/28 21:24:55 lombard Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include "math_fort.h"
#include "mathlib.h"

struct {
    int iunit;
    float ds;
    float gain;
    int npoles;
    int nzeros;
    COMPLEX poles[60];
} instr_;

/* map the common block */
void InitInstrResponse(int iunit, float ds, float gain, int npoles, 
		       int nzeros, COMPLEX *poles)
{
    int i;

    instr_.iunit= iunit;
    instr_.ds=    ds;
    instr_.gain=  gain;
    instr_.npoles= npoles;
    instr_.nzeros= nzeros;
    for(i=0;i<60;i++) {
	instr_.poles[i].real = poles[i].real;
	instr_.poles[i].imag = poles[i].imag;
    }
    return;
}

/*
 * instr_type:	0 - deconvolve
 *		1 - Benioff 100kg (Mb)
 *		2 - 17s-23s 10 pole butterworth filter (Ms)
 *		3 - Wood Anderson (ML)
 */
void ConvertInstr(int nsamp, float srate, int instr_type, float *data )
{
    int npts, ind, ind1, n2, i, errc;
    float dt, dum;
    float *cw, *c1;

    ind1=0;
    npts= nsamp;
    ind= instr_type;
    n2= ipow_(&npts);
    dum= 0.0;
    dt= (float)1/srate;
    cw= (float *)malloc(sizeof(double)*(n2+10));
    c1= (float *)malloc(sizeof(float)*(n2+10));

    for(i=0;i<nsamp;i++) {
	c1[i]=data[i];
    }

/*    coinst_(&npts,&dt,&ind,c1,cw,&dum,&dum); */
/*    printf("inst %d  decon %d\n",ind,ind1); */
    coinst_(&npts,&dt,&ind,&ind1,c1,cw,&dum,&dum,&errc);

    for(i=0;i<nsamp;i++) {
	data[i]=c1[i];
    }
    free(c1);
    free(cw);
    return;
}

/*
 * 1- bandpass
 * 2- bandpass 0 phase
 * 3- lowpass
 * 4- lowpass 0 phase
 * 5- highpass
 * 6- highpass 0 phase
 */
void FilterTrace(int nsamp, float srate, int which_fil, int dec_type,
		 float *data, float fl, float fh )
{
    int npts, ind, ind1, n2, i,errc;
    float dt, low_f, hi_f;
    float *cw, *c1;

    npts= nsamp;
    ind= which_fil;
    ind1= dec_type;
    low_f= fl;
    hi_f= fh;
    n2= ipow_(&npts);
    dt= (float)1/srate;
    cw= (float *)malloc(sizeof(double)*(n2+10));
    c1= (float *)malloc(sizeof(float)*(n2+10));

    for(i=0;i<nsamp;i++) {
	c1[i]=data[i];
    }

/*    printf("inst %d decon %d\n",ind,ind1); */
    coinst_(&npts,&dt,&ind1,&ind,c1,cw,&low_f,&hi_f,&errc);

    for(i=0;i<nsamp;i++) {
	data[i]=c1[i];
    }
    free(c1);
    free(cw);
    return;
}
