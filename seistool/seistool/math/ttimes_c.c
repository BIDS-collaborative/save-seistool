#ifndef lint
static char id[] = "$Id: ttimes_c.c,v 1.2 2013/02/28 21:24:54 lombard Exp $";
#endif

#include <stdlib.h>
#include <strings.h>
#include "math_fort.h"
#include "mathlib.h"

/*
 *     zs - source depth (km)
 *     delta - distance in degrees
 *     plist - phase request
 *
 *    
 *     n - number of phases
 *     tt - array of travel times
 *     dtdd - array of d time /d distance values
 *     dtdh - array of d time /d depth values
 *     dddp - array of d distane /d ray parameter values
 *    phcd - array of phase names
 */
#define NPHASE		200
#define MAXPHNAMELEN	8
 
void GetTrvlTimes(float zs, float delta, char *phlist, int *n, 
		  float **p_tt, char **p_phcd)
{
    float dtdd[NPHASE],dtdh[NPHASE],dddp[NPHASE];
    char *phcd, plist[8], *t;
    float  *tt, ddelta, zzs;
    int n_plist=8;
    int n_phcd= MAXPHNAMELEN;

    /* make sure we are blank padded */
    strncpy(plist,phlist,8);
    if (plist[7]=='\0') {
	t=plist+strlen(plist);
	while(t<(plist+8))
	    *t++=' ';
    }

    *p_tt= tt= (float *)malloc(sizeof(float)*NPHASE);
    *p_phcd= phcd= (char *)malloc(MAXPHNAMELEN*NPHASE);
    bzero(tt,sizeof(float)*NPHASE);

    zzs= zs;
    ddelta=delta;   /* I throw up my hands-- don't ask me */

    ttimes_(&zzs,&ddelta,plist,n,tt,dtdd,dtdh,dddp,phcd,n_plist,n_phcd);
}

