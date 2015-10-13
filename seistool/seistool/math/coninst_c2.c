#ifndef lint
static char id[] = "$Id: coninst_c2.c,v 1.1 2001/12/21 18:39:07 lombard Exp $";
#endif

#include <stdio.h>

typedef struct {
    float x, y;
} COMPLEX;

struct {
    int iunit;
    float ds;
    float gain;
    int npoles;
    int nzeros;
    COMPLEX poles[60];
} instr_;


/* map the common block */
void InitInstrResponse(iunit, ds, gain, npoles, nzeros, poles)
    int iunit; float ds, gain; int npoles, nzeros; COMPLEX *poles;
{
    int i;

    instr_.iunit= iunit;
    instr_.ds=    ds;
    instr_.gain=  gain;
    instr_.npoles= npoles;
    instr_.nzeros= nzeros;
    for(i=0;i<60;i++) {
	instr_.poles[i].x= poles[i].x;
	instr_.poles[i].y= poles[i].y;
    }
    return;
}

/*
 * instr_type:	0 - deconvolve
 *		1 - Benioff 100kg (Mb)
 *		2 - 17s-23s 10 pole butterworth filter (Ms)
 *		3 - Wood Anderson (ML)
 */
void
ConvertInstr( nsamp, srate, instr_type, data )
    int nsamp; float srate; int instr_type; float *data;
{
    int npts, ind, n2, i;
    float dt, dum;
    float *cw, *c1;

    npts= nsamp;
    ind= instr_type;
    n2= ipow_(&npts);
    dum= 0.0;
    dt= (float)1/srate;
    cw= (float *)malloc(sizeof(double)*n2);
    c1= (float *)malloc(sizeof(float)*n2);

    for(i=0;i<nsamp;i++) {
	c1[i]=data[i];
    }

    coinst_(&npts,&dt,&ind,c1,cw,&dum,&dum);

    for(i=0;i<nsamp;i++) {
	data[i]=c1[i];
    }
    free(c1);
    free(cw);
    return;
}

/*
 *  0- bandpass
 *  1- lowpass
 *  2- highpass
 */
void
FilterTrace( nsamp, srate, which_fil, data, fl, fh )
    int nsamp; float srate; int which_fil; float *data, fl, fh;
{
    int npts, ind, n2, i;
    float dt, low_f, hi_f;
    float *cw, *c1;

    npts= nsamp;
    ind= which_fil+10;
    low_f= fl;
    hi_f= fh;
    n2= ipow_(&npts);
    dt= (float)1/srate;
    cw= (float *)malloc(sizeof(double)*n2);
    c1= (float *)malloc(sizeof(float)*n2);

    for(i=0;i<nsamp;i++) {
	c1[i]=data[i];
    }
    coinst_(&npts,&dt,&ind,c1,cw,&low_f,&hi_f);

    for(i=0;i<nsamp;i++) {
	data[i]=c1[i];
    }
    free(c1);
    free(cw);
    return;
}
