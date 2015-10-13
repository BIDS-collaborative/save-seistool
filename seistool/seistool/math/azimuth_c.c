#ifndef lint
static char id[] = "$Id: azimuth_c.c,v 1.2 2013/02/28 21:24:55 lombard Exp $";
#endif

#include "math_fort.h"
#include "mathlib.h"

/*
	program azm

	print*, 'recv: berkeley'
	print*, 'input	slat slon'
	read*, slat, slon
	rlat= 37.877
	rlon= -122.235
	print*, 'slat=',slat,'slon',slon
	print*, 'rlat=',rlat,'rlon',rlon
	call azimth(slat,slon,rlat,rlon,delta,azim,bazim)
	print*, 'delta=',delta,'azim=',azim,'bazim=',bazim
	stop
	end
*/
void azimuth(float slat, float slon, float rlat, float rlon, float *pDelta, 
	float *pAzim, float *pBazim)
{
    /* just to be safe-- */
    float sla, slo, rla, rlo;
    float del, azim, bazim;
    sla=slat;
    slo=slon;
    rla=rlat;
    rlo=rlon;
    azimth_(&sla,&slo,&rla,&rlo,&del,&azim,&bazim);
    *pDelta= del;
    *pAzim= azim;
    *pBazim= bazim;
}
