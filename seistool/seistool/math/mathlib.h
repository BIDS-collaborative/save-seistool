/* Header for seistool math library, containing function prototypes *
 * for C functions. Pete Lombard, BSL, 2013/01/29                   */

#ifndef MATHLIB_H
#define MATHLIB_H

#include "mathlib.h"
#include "complex.h"

void ComputeMaxAmpPeriod(int nsamp, float srate, int instr_type, float *data,
			 float *pRaw_max, float *pGround_max, 
			 float *pPeriod_max, int *pIdx );
void azimuth(float slat, float slon, float rlat, float rlon, float *pDelta, 
	     float *pAzim, float *pBazim);
void InitInstrResponse(int iunit, float ds, float gain, int npoles, 
		       int nzeros, COMPLEX *poles);
void ConvertInstr(int nsamp, float srate, int instr_type, float *data);
void FilterTrace(int nsamp, float srate, int which_fil, int dec_type,
		 float *data, float fl, float fh );
void CalcSpectrum(float srate, float *data, int ix1, int ix2, 
		  float **y, float **a, float **b, int *n2_p, 
		  int *nt2h, int soph );
void GetTrvlTimes(float zs, float delta, char *phlist, int *n, 
		  float **p_tt, char **p_phcd);
void CalcXcorr(float srate, float *data1, float *data2, float **y, 
	       int *n2_p, int *npts, int soph );

#endif
