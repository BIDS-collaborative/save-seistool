/* Function prototypes for fortran codes called from C *
 * Only includes functions actually called from C.     */

#ifndef MATH_FORT_H
#define MATH_FORT_H

void amaxper_(int *npts, float *dt, int *ind, float *data, float *pRaw_max,
	      float *gd_max, float *pPeriod_max, int *pIdx);
void azimth_(float *slat, float *slon, float *rlat, float *rlon, float *delta,
	     float *azim, float *bazim);
void coinst_(int *npts, float *dt, int *ind, int *ind1, float *c1, float *cw,
	     float *fl, float *fh, int *errc);
int ipow_(int *n);
void spectrum_(float *c1, float *y, float *aa, float *bb, int *npts, float *dt,
	       int *lsoph);
void ttimes_(float *zs, float *delta, char *plist, int *n, float *tt, 
	     float *dtdd, float *dtdh, float *dddp, char *phcd, int n_plist,
	     int n_phcd);
void xcor_(int *npts, float *dt, float *c1, float *c2, float *y1, int *err);


#endif
