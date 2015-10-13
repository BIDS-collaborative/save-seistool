/*	$Id: spctrm.h,v 1.2 2013/02/28 21:24:57 lombard Exp $	*/

/*************************************
 * 
 *   define the spectrum structure
 *
/*************************************/

#ifndef SPCTRM_H
#define SPCTRM_H

typedef struct {
    float *power;
    float *phase;
    float *amp;
    int   n_pnts;
    int   n2_pnts;
} hold_spct;

#endif
